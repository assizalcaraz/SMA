#include "SynthesisEngine.h"

//==============================================================================
SynthesisEngine::SynthesisEngine()
{
    reset();
}

//==============================================================================
void SynthesisEngine::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    voiceManager.prepare(sampleRate, maxVoices.load());
    outputLevel = 0.0f;
    
    // Ajustar decay rate del nivel según sample rate
    // Decay de ~100ms
    outputLevelDecay = std::exp(-1.0f / (0.1f * (float)sampleRate));
    
    // DIAGNOSTIC: For stability testing, use:
    // - Buffer size: 1024 samples
    // - Sample rate: 48kHz  
    // - Polyphony: 8 voices
    // If overload persists, check for remaining RT-unsafe operations
}

//==============================================================================
void SynthesisEngine::renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    // RT-SAFE: Procesar eventos de la cola lock-free primero (máximo MAX_HITS_PER_BLOCK)
    processEventQueue();
    
    // Renderizar voces
    voiceManager.renderNextBlock(buffer, startSample, numSamples);
    
    // Aplicar procesamiento master
    float currentDrive = drive.load();
    bool currentLimiterEnabled = limiterEnabled.load();
    
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        float* channelData = buffer.getWritePointer(channel, startSample);
        
        for (int sample = 0; sample < numSamples; sample++)
        {
            float sampleValue = channelData[sample];
            
            // Aplicar saturación si drive > 0
            if (currentDrive > 0.0f)
            {
                sampleValue = applySaturation(sampleValue, currentDrive);
            }
            
            // Aplicar limiter si está habilitado
            if (currentLimiterEnabled)
            {
                sampleValue = applyLimiter(sampleValue);
            }
            
            channelData[sample] = sampleValue;
            
            // Actualizar nivel de salida (usar primer canal)
            if (channel == 0)
            {
                updateOutputLevel(sampleValue);
            }
        }
    }
    
    // RT-SAFE: Removido setMaxVoices() y prepare() desde audio thread
    // Estos deben llamarse desde UI thread usando setMaxVoices() que actualiza el atomic
    // VoiceManager debe pre-allocar todas las voces y solo activar/desactivar según maxVoices
}

//==============================================================================
void SynthesisEngine::setMaxVoices(int newMaxVoices)
{
    // RT-SAFE: Solo actualiza el atomic, no modifica voces desde aquí
    // VoiceManager debe pre-allocar todas las voces y solo activar/desactivar según este valor
    maxVoices.store(juce::jlimit(4, 12, newMaxVoices)); // Rango reducido a 4-12 para estabilidad RT
}

void SynthesisEngine::setMetalness(float newMetalness)
{
    metalness.store(juce::jlimit(0.0f, 1.0f, newMetalness));
}

void SynthesisEngine::setBrightness(float newBrightness)
{
    brightness.store(juce::jlimit(0.0f, 1.0f, newBrightness));
}

void SynthesisEngine::setDamping(float newDamping)
{
    damping.store(juce::jlimit(0.0f, 1.0f, newDamping));
}

void SynthesisEngine::setDrive(float newDrive)
{
    drive.store(juce::jlimit(0.0f, 1.0f, newDrive));
}

void SynthesisEngine::setReverbMix(float newReverbMix)
{
    reverbMix.store(juce::jlimit(0.0f, 1.0f, newReverbMix));
}

void SynthesisEngine::setLimiterEnabled(bool enabled)
{
    limiterEnabled.store(enabled);
}

//==============================================================================
int SynthesisEngine::getMaxVoices() const
{
    return maxVoices.load();
}

float SynthesisEngine::getMetalness() const
{
    return metalness.load();
}

float SynthesisEngine::getBrightness() const
{
    return brightness.load();
}

float SynthesisEngine::getDamping() const
{
    return damping.load();
}

float SynthesisEngine::getDrive() const
{
    return drive.load();
}

float SynthesisEngine::getReverbMix() const
{
    return reverbMix.load();
}

bool SynthesisEngine::isLimiterEnabled() const
{
    return limiterEnabled.load();
}

//==============================================================================
void SynthesisEngine::triggerTestVoice()
{
    // RT-SAFE: Escribir evento a cola lock-free en lugar de llamar directamente
    // El audio thread procesará el evento en el próximo renderNextBlock()
    HitEvent event;
    event.baseFreq = testFreq.load();
    event.amplitude = testAmplitude.load();
    event.damping = damping.load();
    event.brightness = brightness.load();
    event.metalness = metalness.load();
    
    int start1, size1, start2, size2;
    eventFifo.prepareToWrite(1, start1, size1, start2, size2);
    
    if (size1 > 0)
    {
        eventQueue[start1] = event;
        eventFifo.finishedWrite(size1);
    }
    // Si la cola está llena, el evento se descarta silenciosamente (protección contra overflow)
}

//==============================================================================
int SynthesisEngine::getActiveVoiceCount() const
{
    return voiceManager.getActiveVoiceCount();
}

//==============================================================================
float SynthesisEngine::getOutputLevel() const
{
    return outputLevel;
}

//==============================================================================
void SynthesisEngine::reset()
{
    voiceManager.resetAll();
    outputLevel = 0.0f;
}

//==============================================================================
float SynthesisEngine::applySaturation(float sample, float driveAmount)
{
    // Saturación suave usando tanh
    // driveAmount 0 = sin saturación, 1 = saturación máxima
    float saturationAmount = 1.0f + driveAmount * 2.0f; // 1.0 a 3.0
    return std::tanh(sample * saturationAmount) / saturationAmount;
}

//==============================================================================
float SynthesisEngine::applyLimiter(float sample)
{
    // RT-SAFE: Limiter simplificado (evitar std::abs() costoso)
    float absSample = sample >= 0.0f ? sample : -sample;
    
    if (absSample > limiterThreshold)
    {
        // Simplificar cálculo de limiter
        float ratio = limiterThreshold / absSample;
        return sample * ratio;
    }
    
    return sample;
}

//==============================================================================
void SynthesisEngine::processEventQueue()
{
    // RT-SAFE: Drenar eventos de la cola lock-free (máximo MAX_HITS_PER_BLOCK por bloque)
    int numToRead = juce::jmin(MAX_HITS_PER_BLOCK, eventFifo.getNumReady());
    
    if (numToRead == 0)
        return;
    
    int start1, size1, start2, size2;
    eventFifo.prepareToRead(numToRead, start1, size1, start2, size2);
    
    // Procesar primera sección
    for (int i = 0; i < size1; i++)
    {
        const HitEvent& event = eventQueue[start1 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude, 
                                   event.damping, event.brightness, event.metalness);
    }
    
    // Procesar segunda sección (si hay wraparound)
    for (int i = 0; i < size2; i++)
    {
        const HitEvent& event = eventQueue[start2 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude, 
                                   event.damping, event.brightness, event.metalness);
    }
    
    eventFifo.finishedRead(size1 + size2);
}

//==============================================================================
void SynthesisEngine::updateOutputLevel(float sample)
{
    // RMS aproximado con decay exponencial
    float squared = sample * sample;
    outputLevel = outputLevel * outputLevelDecay + squared * (1.0f - outputLevelDecay);
}
