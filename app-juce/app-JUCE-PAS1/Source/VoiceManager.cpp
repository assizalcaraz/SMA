#include "VoiceManager.h"

//==============================================================================
VoiceManager::VoiceManager()
{
    maxVoices = DEFAULT_MAX_VOICES;
    
    // RT-SAFE: Inicializar array de tiempos de trigger y pan
    for (int i = 0; i < MAX_VOICES_LIMIT; i++)
    {
        voiceTriggerTime[i] = -1;
        voiceGainL[i] = 1.0f;
        voiceGainR[i] = 1.0f;
    }
}

//==============================================================================
void VoiceManager::prepare(double sampleRate, int maxVoicesToSet)
{
    currentSampleRate = sampleRate;
    
    // RT-SAFE: Pre-allocar todas las voces hasta MAX_VOICES_LIMIT (una sola vez)
    while (voices.size() < MAX_VOICES_LIMIT)
    {
        auto* voice = new ModalVoice();
        voice->prepare(sampleRate);
        voices.add(voice);
    }
    
    // Establecer límite activo (no modifica el array, solo el contador)
    maxVoices = juce::jlimit(MIN_VOICES_LIMIT, MAX_VOICES_LIMIT, maxVoicesToSet);
    
    // Preparar todas las voces pre-allocadas
    for (auto* voice : voices)
    {
        voice->prepare(sampleRate);
    }
    
    // Resetear tiempos de trigger
    for (int i = 0; i < MAX_VOICES_LIMIT; i++)
    {
        voiceTriggerTime[i] = -1;
    }
    
    currentTime = 0;
}

//==============================================================================
void VoiceManager::setMaxVoices(int newMaxVoices)
{
    newMaxVoices = juce::jlimit(MIN_VOICES_LIMIT, MAX_VOICES_LIMIT, newMaxVoices);
    if (newMaxVoices == maxVoices)
        return;
    maxVoices = newMaxVoices;
}

void VoiceManager::setMaxVoicesPerQuadrant(int n)
{
    maxVoicesPerQuadrant = juce::jlimit(0, 8, n);
}

//==============================================================================
void VoiceManager::triggerVoice(float baseFreq, float amplitude, float damping, 
                                 float brightness, float metalness,
                                 ModalVoice::ExcitationWaveform waveform,
                                 float subOscMix,
                                 float gainL, float gainR,
                                 int quadrant)
{
    if (maxVoices == 0 || voices.size() == 0)
        return;
    
    ModalVoice* voiceToUse = findAvailableVoice(quadrant);
    if (voiceToUse == nullptr)
        voiceToUse = findVoiceToSteal(quadrant);
    
    if (voiceToUse != nullptr)
    {
        // RT-SAFE: Calcular índice directamente (voces están en array fijo)
        // En lugar de indexOf() que es costoso, calculamos el índice desde el puntero
        int voiceIndex = -1;
        int searchLimit = juce::jmin(maxVoices, voices.size());
        for (int i = 0; i < searchLimit; i++)
        {
            if (voices.getUnchecked(i) == voiceToUse)
            {
                voiceIndex = i;
                break;
            }
        }
        
        // Configurar y trigger la voz
        voiceToUse->setParameters(baseFreq, amplitude, damping, brightness, metalness, waveform, subOscMix);
        voiceToUse->trigger();
        
        if (voiceIndex >= 0)
        {
            updateVoiceTime(voiceIndex);
            voiceGainL[voiceIndex] = gainL;
            voiceGainR[voiceIndex] = gainR;
        }
    }
}

//==============================================================================
void VoiceManager::renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    // Limpiar el buffer primero
    buffer.clear(startSample, numSamples);
    
    // RT-SAFE: Verificar que hay voces disponibles
    if (maxVoices == 0 || voices.size() == 0)
        return;
    
    // Obtener write pointers una sola vez
    float* leftChannel = buffer.getWritePointer(0, startSample);
    float* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1, startSample) : nullptr;
    
    // RT-SAFE: Renderizar solo voces dentro del rango activo (maxVoices)
    int searchLimit = juce::jmin(maxVoices, voices.size());
    for (int i = 0; i < searchLimit; i++)
    {
        auto* voice = voices.getUnchecked(i);
        
        // Check rápido de actividad antes del loop
        if (!voice->isActive())
            continue;
        
        // Renderizar todos los samples de esta voz de una vez (pan constant-power)
        float gL = voiceGainL[i];
        float gR = voiceGainR[i];
        for (int sample = 0; sample < numSamples; sample++)
        {
            float sampleValue = voice->renderNextSample();
            leftChannel[sample] += sampleValue * gL;
            if (rightChannel != nullptr)
                rightChannel[sample] += sampleValue * gR;
        }
    }
    
    // Incrementar tiempo global
    currentTime += numSamples;
}

//==============================================================================
int VoiceManager::getActiveVoiceCount() const
{
    // RT-SAFE: Contar solo dentro del rango activo
    if (maxVoices == 0 || voices.size() == 0)
        return 0;
    
    int count = 0;
    int searchLimit = juce::jmin(maxVoices, voices.size());
    for (int i = 0; i < searchLimit; i++)
    {
        if (voices.getUnchecked(i)->isActive())
            count++;
    }
    return count;
}

//==============================================================================
void VoiceManager::resetAll()
{
    // RT-SAFE: Resetear solo voces dentro del rango activo
    if (voices.size() > 0)
    {
        int resetLimit = juce::jmin(maxVoices, voices.size());
        for (int i = 0; i < resetLimit; i++)
        {
            voices.getUnchecked(i)->reset();
        }
    }
    
    // Resetear tiempos de trigger y pan
    for (int i = 0; i < MAX_VOICES_LIMIT; i++)
    {
        voiceTriggerTime[i] = -1;
        voiceGainL[i] = 1.0f;
        voiceGainR[i] = 1.0f;
    }
    
    currentTime = 0;
}

//==============================================================================
ModalVoice* VoiceManager::findAvailableVoice(int quadrant)
{
    if (maxVoices == 0 || voices.size() == 0)
        return nullptr;
    
    int searchLimit = juce::jmin(maxVoices, (int)voices.size());
    int reservedTotal = 4 * maxVoicesPerQuadrant;
    
    if (quadrant >= 0 && quadrant < 4 && reservedTotal <= maxVoices)
    {
        // Primero buscar en el presupuesto del cuadrante
        int quadStart = quadrant * maxVoicesPerQuadrant;
        int quadEnd = juce::jmin(quadStart + maxVoicesPerQuadrant, searchLimit);
        for (int i = quadStart; i < quadEnd; i++)
        {
            auto* voice = voices.getUnchecked(i);
            if (!voice->isActive())
                return voice;
        }
        // Luego en el pool compartido
        for (int i = reservedTotal; i < searchLimit; i++)
        {
            auto* voice = voices.getUnchecked(i);
            if (!voice->isActive())
                return voice;
        }
        return nullptr;
    }
    
    for (int i = 0; i < searchLimit; i++)
    {
        auto* voice = voices.getUnchecked(i);
        if (!voice->isActive())
            return voice;
    }
    return nullptr;
}

//==============================================================================
ModalVoice* VoiceManager::findVoiceToSteal(int quadrant)
{
    if (maxVoices == 0 || voices.size() == 0)
        return nullptr;
    
    int searchLimit = juce::jmin(maxVoices, (int)voices.size());
    int reservedTotal = 4 * maxVoicesPerQuadrant;
    
    auto findBestInRange = [this, searchLimit](int start, int end) -> ModalVoice* {
        ModalVoice* best = nullptr;
        float minAmp = 1e9f;
        int oldestTime = 2147483647;
        for (int i = start; i < end && i < searchLimit; i++)
        {
            auto* v = voices.getUnchecked(i);
            if (!v->isActive()) continue;
            float a = v->getResidualAmplitude();
            int t = voiceTriggerTime[i];
            if (best == nullptr || a < minAmp || (std::abs(a - minAmp) < 0.0001f && t < oldestTime))
            {
                best = v;
                minAmp = a;
                oldestTime = t;
            }
        }
        return best;
    };
    
    ModalVoice* toSteal = nullptr;
    if (quadrant >= 0 && quadrant < 4 && reservedTotal <= maxVoices)
    {
        int quadStart = quadrant * maxVoicesPerQuadrant;
        int quadEnd = juce::jmin(quadStart + maxVoicesPerQuadrant, searchLimit);
        toSteal = findBestInRange(quadStart, quadEnd);
        if (!toSteal)
            toSteal = findBestInRange(reservedTotal, searchLimit);
    }
    if (!toSteal)
        toSteal = findBestInRange(0, searchLimit);
    
    if (toSteal)
        toSteal->reset();
    return toSteal;
}

//==============================================================================
void VoiceManager::updateVoiceTime(int voiceIndex)
{
    // RT-SAFE: Actualizar tiempo usando array fijo pre-allocado
    if (voiceIndex >= 0 && voiceIndex < MAX_VOICES_LIMIT)
    {
        voiceTriggerTime[voiceIndex] = currentTime;
    }
}

//==============================================================================
void VoiceManager::updateGlobalParameters(float metalness, float brightness, float damping)
{
    // RT-SAFE: Actualizar parámetros globales en todas las voces activas
    // Mantener frecuencia base y amplitud actuales de cada voz
    if (maxVoices == 0 || voices.size() == 0)
        return;
    
    int searchLimit = juce::jmin(maxVoices, voices.size());
    for (int i = 0; i < searchLimit; i++)
    {
        auto* voice = voices.getUnchecked(i);
        
        // Solo actualizar voces activas
        if (voice->isActive())
            voice->setGlobalParametersOnly(metalness, brightness, damping);
    }
}
