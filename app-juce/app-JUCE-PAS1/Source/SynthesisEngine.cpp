#include "SynthesisEngine.h"

//==============================================================================
SynthesisEngine::SynthesisEngine()
    : plateBuffer(2, MAX_BLOCK_SIZE) // Pre-allocar buffer para 2 canales, tamaño máximo
{
    reset();
}

//==============================================================================
void SynthesisEngine::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    voiceManager.prepare(sampleRate, maxVoices.load());
    plateSynth.prepare(sampleRate);
    outputLevel = 0.0f;
    
    // Ajustar decay rate del nivel según sample rate
    // Decay de ~100ms
    outputLevelDecay = std::exp(-1.0f / (0.1f * (float)sampleRate));
    
    // Preparar buffer de plate (RT-safe: pre-allocar tamaño máximo esperado)
    // Nota: El tamaño real se ajustará dinámicamente si es necesario, pero esto
    // debería ser raro ya que prepare() se llama cuando cambia el buffer size
    plateBuffer.setSize(2, MAX_BLOCK_SIZE, false, false, true);
    
    // Inicializar valores previos de parámetros globales
    prevMetalness = metalness.load();
    parameterUpdateCounter = 0;
    
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
    
    // RT-SAFE: Actualizar parámetros globales en voces activas periódicamente
    // Actualizar cada N bloques para eficiencia (evitar actualizar en cada bloque)
    parameterUpdateCounter++;
    if (parameterUpdateCounter >= PARAMETER_UPDATE_INTERVAL)
    {
        parameterUpdateCounter = 0;
        
        // Leer parámetros globales (atomic, thread-safe)
        float currentMetalness = metalness.load();
        
        // Actualizar solo si cambió (optimización)
        if (std::abs(currentMetalness - prevMetalness) > 0.001f)
        {
            // Usar valores fijos para brightness y damping (0.5 = medio)
            voiceManager.updateGlobalParameters(currentMetalness, 0.5f, 0.5f);
            
            // Actualizar valor previo
            prevMetalness = currentMetalness;
        }
    }
    
    // Renderizar voces
    voiceManager.renderNextBlock(buffer, startSample, numSamples);
    
    // Renderizar plate y sumar al buffer (pre-limiter)
    // RT-SAFE: Usar buffer pre-allocado en prepare()
    int numChannels = buffer.getNumChannels();
    
    // Limpiar y renderizar plate en buffer temporal
    // Usar una vista del buffer pre-allocado para el tamaño necesario
    juce::AudioBuffer<float> plateView(plateBuffer.getArrayOfWritePointers(), 
                                        juce::jmin(numChannels, plateBuffer.getNumChannels()), 
                                        0, 
                                        juce::jmin(numSamples, plateBuffer.getNumSamples()));
    plateView.clear();
    plateSynth.renderNextBlock(plateView, 0, plateView.getNumSamples());
    
    // Sumar plate a buffer principal (pre-limiter) con control de volumen
    float currentPlateVolume = plateVolume.load(); // RT-safe: leer atomic
    int safeSamples = juce::jmin(numSamples, plateView.getNumSamples());
    int safeChannels = juce::jmin(numChannels, plateView.getNumChannels());
    for (int channel = 0; channel < safeChannels; channel++)
    {
        float* mainData = buffer.getWritePointer(channel, startSample);
        const float* plateData = plateView.getReadPointer(channel, 0);
        for (int sample = 0; sample < safeSamples; sample++)
        {
            mainData[sample] += plateData[sample] * currentPlateVolume;
        }
    }
    
    // Aplicar procesamiento master (solo limiter)
    bool currentLimiterEnabled = limiterEnabled.load();
    
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        float* channelData = buffer.getWritePointer(channel, startSample);
        
        for (int sample = 0; sample < numSamples; sample++)
        {
            float sampleValue = channelData[sample];
            
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
    // RT-SAFE: Actualizar atomic y también el VoiceManager
    int limitedVoices = juce::jlimit(4, 12, newMaxVoices);
    maxVoices.store(limitedVoices);
    
    // Actualizar VoiceManager (esto es seguro porque se llama desde UI thread)
    voiceManager.setMaxVoices(limitedVoices);
}

void SynthesisEngine::setMetalness(float newMetalness)
{
    metalness.store(juce::jlimit(0.0f, 1.0f, newMetalness));
}

void SynthesisEngine::setWaveform(ModalVoice::ExcitationWaveform newWaveform)
{
    waveform.store(static_cast<int>(newWaveform));
}

void SynthesisEngine::setSubOscMix(float newSubOscMix)
{
    subOscMix.store(juce::jlimit(0.0f, 1.0f, newSubOscMix));
}

void SynthesisEngine::setLimiterEnabled(bool enabled)
{
    limiterEnabled.store(enabled);
}

void SynthesisEngine::setPlateVolume(float volume)
{
    plateVolume.store(juce::jlimit(0.0f, 1.0f, volume));
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

ModalVoice::ExcitationWaveform SynthesisEngine::getWaveform() const
{
    return static_cast<ModalVoice::ExcitationWaveform>(waveform.load());
}

float SynthesisEngine::getSubOscMix() const
{
    return subOscMix.load();
}

bool SynthesisEngine::isLimiterEnabled() const
{
    return limiterEnabled.load();
}

float SynthesisEngine::getPlateVolume() const
{
    return plateVolume.load();
}

//==============================================================================
void SynthesisEngine::triggerTestVoice()
{
    // RT-SAFE: Escribir evento a cola lock-free en lugar de llamar directamente
    // El audio thread procesará el evento en el próximo renderNextBlock()
    HitEvent event;
    event.baseFreq = testFreq.load();
    event.amplitude = testAmplitude.load();
    event.damping = 0.5f; // Valor fijo
    event.brightness = 0.5f; // Valor fijo
    event.metalness = metalness.load();
    event.waveform = static_cast<ModalVoice::ExcitationWaveform>(waveform.load());
    event.subOscMix = subOscMix.load();
    
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
void SynthesisEngine::triggerVoiceFromOSC(float baseFreq, float amplitude, 
                                         float damping, float brightness, float metalness,
                                         ModalVoice::ExcitationWaveform waveform,
                                         float subOscMix)
{
    // RT-SAFE: Escribir evento a cola lock-free (mismo path que triggerTestVoice)
    // El audio thread procesará el evento en el próximo renderNextBlock()
    HitEvent event;
    event.baseFreq = baseFreq;
    event.amplitude = amplitude;
    event.damping = damping;
    event.brightness = brightness;
    event.metalness = metalness;
    event.waveform = waveform;
    event.subOscMix = subOscMix;
    
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
    plateSynth.reset();
    outputLevel = 0.0f;
}

//==============================================================================
void SynthesisEngine::triggerPlateFromOSC(float freq, float amp, int mode)
{
    // RT-SAFE: Llamar a PlateSynth que usa atomic internamente
    plateSynth.triggerPlate(freq, amp, mode);
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
    // Limitar aún más bajo carga para evitar sobrecarga del audio thread
    int available = eventFifo.getNumReady();
    if (available == 0)
        return;
    
    // Reducir procesamiento si hay muchos eventos pendientes (protección contra sobrecarga)
    int numToRead = juce::jmin(MAX_HITS_PER_BLOCK, available);
    
    // Si hay demasiados eventos acumulados, procesar menos para evitar sobrecarga
    if (available > MAX_HITS_PER_BLOCK * 2)
    {
        numToRead = MAX_HITS_PER_BLOCK / 2; // Procesar la mitad para dar tiempo al audio thread
    }
    
    int start1, size1, start2, size2;
    eventFifo.prepareToRead(numToRead, start1, size1, start2, size2);
    
    // Procesar primera sección
    for (int i = 0; i < size1; i++)
    {
        const HitEvent& event = eventQueue[start1 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude, 
                                   event.damping, event.brightness, event.metalness,
                                   event.waveform, event.subOscMix);
    }
    
    // Procesar segunda sección (si hay wraparound)
    for (int i = 0; i < size2; i++)
    {
        const HitEvent& event = eventQueue[start2 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude, 
                                   event.damping, event.brightness, event.metalness,
                                   event.waveform, event.subOscMix);
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
