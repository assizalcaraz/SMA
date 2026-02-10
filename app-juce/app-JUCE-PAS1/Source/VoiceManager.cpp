#include "VoiceManager.h"

//==============================================================================
VoiceManager::VoiceManager()
{
    maxVoices = DEFAULT_MAX_VOICES;
    
    // RT-SAFE: Inicializar array de tiempos de trigger
    for (int i = 0; i < MAX_VOICES_LIMIT; i++)
    {
        voiceTriggerTime[i] = -1;
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
    // RT-SAFE: Solo actualiza el límite activo, no modifica el array pre-allocado
    // Las voces ya están pre-allocadas en prepare(), solo cambiamos cuántas están "activas"
    newMaxVoices = juce::jlimit(MIN_VOICES_LIMIT, MAX_VOICES_LIMIT, newMaxVoices);
    
    if (newMaxVoices == maxVoices)
        return;
    
    maxVoices = newMaxVoices;
    // No hay allocations ni modificaciones del array aquí - todo está pre-allocado
}

//==============================================================================
void VoiceManager::triggerVoice(float baseFreq, float amplitude, float damping, 
                                 float brightness, float metalness,
                                 ModalVoice::ExcitationWaveform waveform,
                                 float subOscMix)
{
    // RT-SAFE: Verificar que hay voces disponibles
    if (maxVoices == 0 || voices.size() == 0)
        return;
    
    // RT-SAFE: Buscar solo dentro del rango activo (maxVoices)
    ModalVoice* voiceToUse = findAvailableVoice();
    
    // Si no hay voces disponibles, robar una
    if (voiceToUse == nullptr)
    {
        voiceToUse = findVoiceToSteal();
    }
    
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
        
        // Actualizar tiempo de trigger
        if (voiceIndex >= 0)
            updateVoiceTime(voiceIndex);
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
        
        // Renderizar todos los samples de esta voz de una vez
        for (int sample = 0; sample < numSamples; sample++)
        {
            float sampleValue = voice->renderNextSample();
            
            // Sumar directamente a los canales (más eficiente que addSample)
            leftChannel[sample] += sampleValue;
            if (rightChannel != nullptr)
                rightChannel[sample] += sampleValue;
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
    
    // Resetear tiempos de trigger
    for (int i = 0; i < MAX_VOICES_LIMIT; i++)
    {
        voiceTriggerTime[i] = -1;
    }
    
    currentTime = 0;
}

//==============================================================================
ModalVoice* VoiceManager::findAvailableVoice()
{
    // RT-SAFE: Buscar solo dentro del rango activo (maxVoices)
    // Verificar que hay voces disponibles
    if (maxVoices == 0 || voices.size() == 0)
        return nullptr;
    
    int searchLimit = juce::jmin(maxVoices, voices.size());
    for (int i = 0; i < searchLimit; i++)
    {
        auto* voice = voices.getUnchecked(i);
        if (!voice->isActive())
        {
            return voice;
        }
    }
    return nullptr;
}

//==============================================================================
ModalVoice* VoiceManager::findVoiceToSteal()
{
    if (maxVoices == 0 || voices.size() == 0)
        return nullptr;
    
    // RT-SAFE: Buscar solo dentro del rango activo (maxVoices)
    // Optimizado: buscar voz con menor amplitud residual o más antigua
    int searchLimit = juce::jmin(maxVoices, voices.size());
    ModalVoice* bestVoice = voices.getUnchecked(0);
    float minAmplitude = bestVoice->getResidualAmplitude();
    int bestIndex = 0;
    int oldestTime = voiceTriggerTime[0];
    
    for (int i = 1; i < searchLimit; i++)
    {
        auto* voice = voices.getUnchecked(i);
        float amplitude = voice->getResidualAmplitude();
        int triggerTime = voiceTriggerTime[i];
        
        // Priorizar menor amplitud residual
        if (amplitude < minAmplitude)
        {
            bestVoice = voice;
            minAmplitude = amplitude;
            bestIndex = i;
            oldestTime = triggerTime;
        }
        // Si hay empate en amplitud, elegir la más antigua
        else if (std::abs(amplitude - minAmplitude) < 0.0001f && triggerTime < oldestTime)
        {
            bestVoice = voice;
            minAmplitude = amplitude;
            bestIndex = i;
            oldestTime = triggerTime;
        }
    }
    
    // Resetear la voz robada
    bestVoice->reset();
    
    return bestVoice;
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
        {
            // Obtener parámetros actuales de la voz y actualizar solo los globales
            // Mantener waveform y subOscMix actuales (no se actualizan en updateGlobalParameters)
            voice->setParameters(
                voice->getCurrentBaseFreq(),
                voice->getCurrentAmplitude(),
                damping,
                brightness,
                metalness,
                ModalVoice::ExcitationWaveform::Noise, // Default, se mantiene el waveform de cada voz
                0.0f // Default subOscMix, se mantiene el valor actual
            );
        }
    }
}
