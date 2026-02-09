#pragma once

#include <JuceHeader.h>
#include "ModalVoice.h"

//==============================================================================
/**
    Gestor de polyphony para el sintetizador modal.
    
    Gestiona un pool de voces, asigna voces disponibles y implementa voice stealing
    cuando todas las voces están activas.
*/
class VoiceManager
{
public:
    //==============================================================================
    VoiceManager();
    ~VoiceManager() = default;

    //==============================================================================
    /** Prepara el gestor con el sample rate y número máximo de voces */
    void prepare(double sampleRate, int maxVoices);

    /** Obtiene una voz disponible y la configura, o roba una si es necesario */
    void triggerVoice(float baseFreq, float amplitude, float damping, 
                      float brightness, float metalness);

    /** Renderiza todas las voces activas en el buffer */
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /** Obtiene el número de voces activas */
    int getActiveVoiceCount() const;

    /** Obtiene el número máximo de voces */
    int getMaxVoices() const { return maxVoices; }

    /** Establece el número máximo de voces (requiere prepare) */
    void setMaxVoices(int newMaxVoices);

    /** Resetea todas las voces */
    void resetAll();

    /** Actualiza parámetros globales en todas las voces activas (RT-safe) */
    void updateGlobalParameters(float metalness, float brightness, float damping);

private:
    //==============================================================================
    static constexpr int DEFAULT_MAX_VOICES = 8;
    static constexpr int MAX_VOICES_LIMIT = 32; // Pre-allocar hasta este límite
    static constexpr int MIN_VOICES_LIMIT = 4;

    //==============================================================================
    double currentSampleRate = 44100.0;
    int maxVoices = DEFAULT_MAX_VOICES; // Límite activo (puede ser menor que MAX_VOICES_LIMIT)
    
    // RT-SAFE: Pre-allocado array fijo de voces (no allocations en runtime)
    juce::OwnedArray<ModalVoice> voices;
    
    // Contador de tiempo para voice stealing (voces más antiguas)
    // RT-SAFE: Array fijo pre-allocado
    int voiceTriggerTime[MAX_VOICES_LIMIT];
    int currentTime = 0;
    
    //==============================================================================
    /** Encuentra una voz disponible, o null si todas están activas */
    ModalVoice* findAvailableVoice();

    /** Encuentra la mejor voz para robar (menor amplitud residual o más antigua) */
    ModalVoice* findVoiceToSteal();

    /** Actualiza el tiempo de trigger de una voz */
    void updateVoiceTime(int voiceIndex);
};
