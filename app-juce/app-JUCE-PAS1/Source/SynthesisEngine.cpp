#include "SynthesisEngine.h"
#include <atomic>

// renderNextBlock(), processEventQueue(), and everything they call run on the audio thread
// and must be RT-safe: no locks, allocations, or logging.

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
    prevBrightness = brightness.load();
    prevDamping = damping.load();
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
    processEventQueue();
    // Actualizar cada N bloques para eficiencia (evitar actualizar en cada bloque)
    parameterUpdateCounter++;
    if (parameterUpdateCounter >= PARAMETER_UPDATE_INTERVAL)
    {
        parameterUpdateCounter = 0;
        
        // Leer parámetros globales (atomic, thread-safe)
        float currentMetalness = metalness.load();
        float currentBrightness = brightness.load();
        float currentDamping = damping.load();
        
        // Actualizar solo si cambió algún parámetro (optimización)
        bool needsUpdate = (std::abs(currentMetalness - prevMetalness) > 0.001f) ||
                          (std::abs(currentBrightness - prevBrightness) > 0.001f) ||
                          (std::abs(currentDamping - prevDamping) > 0.001f);
        
        if (needsUpdate)
        {
            // Actualizar parámetros globales en voces activas
            voiceManager.updateGlobalParameters(currentMetalness, currentBrightness, currentDamping);
            
            // Actualizar valores previos
            prevMetalness = currentMetalness;
            prevBrightness = currentBrightness;
            prevDamping = currentDamping;
        }
    }
    
    // Renderizar voces
    voiceManager.renderNextBlock(buffer, startSample, numSamples);
    
    // Renderizar plate y sumar al buffer (pre-limiter)
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
    
    // Aplicar procesamiento master (solo clipper); M3: contar bloques con clipping (una vez por bloque)
    bool currentLimiterEnabled = limiterEnabled.load();
    bool blockHadClip = false;
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        float* channelData = buffer.getWritePointer(channel, startSample);
        for (int sample = 0; sample < numSamples; sample++)
        {
            float sampleValue = channelData[sample];
            if (currentLimiterEnabled)
            {
                if (sampleValue > clipperThreshold || sampleValue < -clipperThreshold)
                    blockHadClip = true;
                sampleValue = applyClipper(sampleValue);
            }
            channelData[sample] = sampleValue;
            if (channel == 0)
                updateOutputLevel(sampleValue);
        }
    }
    if (blockHadClip)
        blocksClippedCount.fetch_add(1, std::memory_order_relaxed);
}

//==============================================================================
void SynthesisEngine::setMaxVoices(int newMaxVoices)
{
    int limitedVoices = juce::jlimit(4, 24, newMaxVoices);
    maxVoices.store(limitedVoices);
    
    // Actualizar VoiceManager (esto es seguro porque se llama desde UI thread)
    voiceManager.setMaxVoices(limitedVoices);
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

void SynthesisEngine::setWaveform(ModalVoice::ExcitationWaveform newWaveform)
{
    waveform.store(static_cast<int>(newWaveform));
}

void SynthesisEngine::setSubOscMix(float newSubOscMix)
{
    subOscMix.store(juce::jlimit(0.0f, 1.0f, newSubOscMix));
}

void SynthesisEngine::setPitchRange(float newPitchRange)
{
    pitchRange.store(juce::jlimit(0.0f, 1.0f, newPitchRange));
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

float SynthesisEngine::getBrightness() const
{
    return brightness.load();
}

float SynthesisEngine::getDamping() const
{
    return damping.load();
}

ModalVoice::ExcitationWaveform SynthesisEngine::getWaveform() const
{
    return static_cast<ModalVoice::ExcitationWaveform>(waveform.load());
}

float SynthesisEngine::getSubOscMix() const
{
    return subOscMix.load();
}

float SynthesisEngine::getPitchRange() const
{
    return pitchRange.load();
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
void SynthesisEngine::incrementHitsReceived()
{
    hitsReceived.fetch_add(1, std::memory_order_relaxed);
}

//==============================================================================
void SynthesisEngine::triggerVoiceFromOSC(float baseFreq, float amplitude, 
                                         float damping, float brightness, float metalness,
                                         ModalVoice::ExcitationWaveform waveform,
                                         float subOscMix)
{
    hitsReceived.fetch_add(1, std::memory_order_relaxed);
    
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
    else
    {
        // Cola llena - evento descartado
        hitsDiscarded.fetch_add(1, std::memory_order_relaxed);
    }
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
    hitsReceived.store(0, std::memory_order_relaxed);
    hitsTriggered.store(0, std::memory_order_relaxed);
    hitsDiscarded.store(0, std::memory_order_relaxed);
    fusedHitsEnqueued.store(0, std::memory_order_relaxed);
    fusedHitsDiscardedQueue.store(0, std::memory_order_relaxed);
    blocksClippedCount.store(0, std::memory_order_relaxed);
    fusedFifo.reset();
}

//==============================================================================
int SynthesisEngine::getHitsReceived() const
{
    return hitsReceived.load(std::memory_order_relaxed);
}

int SynthesisEngine::getHitsTriggered() const
{
    return hitsTriggered.load(std::memory_order_relaxed);
}

int SynthesisEngine::getHitsDiscarded() const
{
    return hitsDiscarded.load(std::memory_order_relaxed);
}

float SynthesisEngine::getHitCoverageRatio() const
{
    int received = hitsReceived.load(std::memory_order_relaxed);
    if (received == 0)
        return 1.0f; // Sin hits recibidos = ratio perfecto
    
    int triggered = hitsTriggered.load(std::memory_order_relaxed);
    return (float)triggered / (float)received;
}

//==============================================================================
bool SynthesisEngine::enqueueFusedSnapshot(const FusedHitSnapshot& snapshot)
{
    int start1, size1, start2, size2;
    fusedFifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0)
    {
        fusedQueue[start1] = snapshot;
        fusedFifo.finishedWrite(size1);
        fusedHitsEnqueued.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    fusedHitsDiscardedQueue.fetch_add(1, std::memory_order_relaxed);
    return false;
}

int SynthesisEngine::getFusedHitsEnqueued() const
{
    return fusedHitsEnqueued.load(std::memory_order_relaxed);
}

int SynthesisEngine::getFusedHitsDiscardedQueue() const
{
    return fusedHitsDiscardedQueue.load(std::memory_order_relaxed);
}

int SynthesisEngine::getBlocksClippedCount() const
{
    return blocksClippedCount.load(std::memory_order_relaxed);
}

//==============================================================================
void SynthesisEngine::triggerPlateFromOSC(float freq, float amp, int mode)
{
    plateSynth.triggerPlate(freq, amp, mode);
}

//==============================================================================
float SynthesisEngine::applyClipper(float sample)
{
    float absSample = sample >= 0.0f ? sample : -sample;
    if (absSample > clipperThreshold)
        return sample * (clipperThreshold / absSample);
    return sample;
}

//==============================================================================
void SynthesisEngine::processEventQueue()
{
    // 1) M2 fused: constant-power stereo pan (gL/gR from snapshot) applied in VoiceManager mix stage
    int fusedAvailable = fusedFifo.getNumReady();
    int fusedToRead = juce::jmin(MAX_HITS_PER_BLOCK, fusedAvailable);
    if (fusedToRead > 0)
    {
        int start1, size1, start2, size2;
        fusedFifo.prepareToRead(fusedToRead, start1, size1, start2, size2);
        for (int i = 0; i < size1; i++)
        {
            const FusedHitSnapshot& s = fusedQueue[start1 + i];
            auto wf = static_cast<ModalVoice::ExcitationWaveform>(juce::jlimit(0, 6, s.waveformAsInt));
            voiceManager.triggerVoice(s.baseFreq, s.amplitude, s.damping, s.brightness, s.metalness,
                                     wf, s.subOscMix, s.gainL, s.gainR, s.quadrant);
            hitsTriggered.fetch_add(1, std::memory_order_relaxed);
        }
        for (int i = 0; i < size2; i++)
        {
            const FusedHitSnapshot& s = fusedQueue[start2 + i];
            auto wf = static_cast<ModalVoice::ExcitationWaveform>(juce::jlimit(0, 6, s.waveformAsInt));
            voiceManager.triggerVoice(s.baseFreq, s.amplitude, s.damping, s.brightness, s.metalness,
                                     wf, s.subOscMix, s.gainL, s.gainR, s.quadrant);
            hitsTriggered.fetch_add(1, std::memory_order_relaxed);
        }
        fusedFifo.finishedRead(size1 + size2);
    }
    
    // 2) Drenar cola de hits crudos (mono: gainL/gainR = 1)
    int available = eventFifo.getNumReady();
    if (available == 0)
        return;
    
    int numToRead = juce::jmin(MAX_HITS_PER_BLOCK, available);
    if (available > MAX_HITS_PER_BLOCK * 2)
        numToRead = MAX_HITS_PER_BLOCK / 2;
    
    int start1, size1, start2, size2;
    eventFifo.prepareToRead(numToRead, start1, size1, start2, size2);
    
    for (int i = 0; i < size1; i++)
    {
        const HitEvent& event = eventQueue[start1 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude,
                                   event.damping, event.brightness, event.metalness,
                                   event.waveform, event.subOscMix, 1.0f, 1.0f);
        hitsTriggered.fetch_add(1, std::memory_order_relaxed);
    }
    for (int i = 0; i < size2; i++)
    {
        const HitEvent& event = eventQueue[start2 + i];
        voiceManager.triggerVoice(event.baseFreq, event.amplitude,
                                   event.damping, event.brightness, event.metalness,
                                   event.waveform, event.subOscMix, 1.0f, 1.0f);
        hitsTriggered.fetch_add(1, std::memory_order_relaxed);
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
