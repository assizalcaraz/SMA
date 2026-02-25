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

    /** Obtiene una voz disponible y la configura, o roba una si es necesario.
     *  gainL, gainR: pan constant-power (1,1 = centro/mono).
     *  quadrant: 0..3 para priorizar presupuesto por cuadrante (R4); -1 = pool global. */
    void triggerVoice(float baseFreq, float amplitude, float damping, 
                      float brightness, float metalness,
                      ModalVoice::ExcitationWaveform waveform = ModalVoice::ExcitationWaveform::Noise,
                      float subOscMix = 0.0f,
                      float gainL = 1.0f, float gainR = 1.0f,
                      int quadrant = -1);

    /** Renderiza todas las voces activas en el buffer */
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /** Obtiene el número de voces activas */
    int getActiveVoiceCount() const;

    /** Obtiene el número máximo de voces */
    int getMaxVoices() const { return maxVoices; }

    /** Establece el número máximo de voces (requiere prepare) */
    void setMaxVoices(int newMaxVoices);

    /** Voces reservadas por cuadrante (0..3); resto compartidas. Default 2. */
    static constexpr int DEFAULT_MAX_VOICES_PER_QUADRANT = 2;
    void setMaxVoicesPerQuadrant(int n);
    int getMaxVoicesPerQuadrant() const { return maxVoicesPerQuadrant; }

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
    int maxVoicesPerQuadrant = DEFAULT_MAX_VOICES_PER_QUADRANT; // Reserva por cuadrante (R4)
    
    // RT-SAFE: Pre-allocado array fijo de voces (no allocations en runtime)
    juce::OwnedArray<ModalVoice> voices;
    
    // Contador de tiempo para voice stealing (voces más antiguas)
    // RT-SAFE: Array fijo pre-allocado
    int voiceTriggerTime[MAX_VOICES_LIMIT];
    int currentTime = 0;
    
    // Pan constant-power por voz (aplicado en renderNextBlock)
    float voiceGainL[MAX_VOICES_LIMIT];
    float voiceGainR[MAX_VOICES_LIMIT];
    
    //==============================================================================
    /** Encuentra una voz disponible; quadrant 0..3 prioriza reserva por cuadrante, -1 = todas */
    ModalVoice* findAvailableVoice(int quadrant = -1);

    /** Encuentra la mejor voz para robar; quadrant 0..3 prefiere robar de ese cuadrante */
    ModalVoice* findVoiceToSteal(int quadrant = -1);

    /** Actualiza el tiempo de trigger de una voz */
    void updateVoiceTime(int voiceIndex);
};
