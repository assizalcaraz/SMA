#pragma once

#include <JuceHeader.h>
#include "VoiceManager.h"
#include "PlateSynth.h"

//==============================================================================
/**
    Motor de síntesis principal que orquesta el VoiceManager y aplica
    procesamiento master (saturación, limiter).
*/
class SynthesisEngine
{
public:
    //==============================================================================
    /** Estructura de evento para cola lock-free */
    struct HitEvent
    {
        float baseFreq;
        float amplitude;
        float damping;
        float brightness;
        float metalness;
        ModalVoice::ExcitationWaveform waveform;
        float subOscMix;
    };

    //==============================================================================
    SynthesisEngine();
    ~SynthesisEngine() = default;

    //==============================================================================
    /** Prepara el motor con el sample rate */
    void prepare(double sampleRate);

    /** Renderiza el siguiente bloque de audio */
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    //==============================================================================
    /** Parámetros globales (thread-safe usando atomic) */
    void setMaxVoices(int maxVoices);
    void setMetalness(float metalness);
    void setWaveform(ModalVoice::ExcitationWaveform waveform);
    void setSubOscMix(float subOscMix);
    void setLimiterEnabled(bool enabled);
    void setPlateVolume(float volume);

    /** Obtiene parámetros actuales */
    int getMaxVoices() const;
    float getMetalness() const;
    ModalVoice::ExcitationWaveform getWaveform() const;
    float getSubOscMix() const;
    bool isLimiterEnabled() const;
    float getPlateVolume() const;

    //==============================================================================
    /** Trigger manual de una voz (para testing sin OSC) - RT-safe: escribe a cola */
    void triggerTestVoice();

    /** Trigger voz desde OSC - RT-safe: escribe a cola lock-free */
    void triggerVoiceFromOSC(float baseFreq, float amplitude, 
                             float damping, float brightness, float metalness,
                             ModalVoice::ExcitationWaveform waveform = ModalVoice::ExcitationWaveform::Noise,
                             float subOscMix = 0.0f);

    /** Trigger plate desde OSC - RT-safe: actualiza atomic */
    void triggerPlateFromOSC(float freq, float amp, int mode);

    /** Obtiene el número de voces activas */
    int getActiveVoiceCount() const;

    /** Obtiene el nivel RMS de salida (para UI) */
    float getOutputLevel() const;

    /** Resetea el motor completamente */
    void reset();

private:
    //==============================================================================
    static constexpr int MAX_HITS_PER_BLOCK = 16; // Límite de eventos procesados por bloque
    static constexpr int EVENT_QUEUE_SIZE = 64;  // Tamaño de la cola de eventos
    
    VoiceManager voiceManager;
    PlateSynth plateSynth;
    
    // Sample rate actual
    double currentSampleRate = 44100.0;
    
    // Parámetros globales (atomic para thread safety)
    std::atomic<int> maxVoices{8}; // Reducido a 8 por defecto para estabilidad RT
    std::atomic<float> metalness{0.5f};
    std::atomic<int> waveform{0}; // ExcitationWaveform como int (enum class no es directamente atomic)
    std::atomic<float> subOscMix{0.0f};
    std::atomic<bool> limiterEnabled{true};
    std::atomic<float> plateVolume{1.0f}; // Volumen del módulo Plate (0.0-1.0)
    
    // Parámetros de trigger manual
    std::atomic<float> testFreq{220.0f};
    std::atomic<float> testAmplitude{0.7f};
    
    // Cola lock-free para eventos (OSC y test trigger)
    juce::AbstractFifo eventFifo{EVENT_QUEUE_SIZE};
    HitEvent eventQueue[EVENT_QUEUE_SIZE];
    
    // Limiter simple
    float limiterThreshold = 0.95f;
    float limiterRatio = 10.0f;
    
    // Medición de nivel de salida
    float outputLevel = 0.0f;
    float outputLevelDecay = 0.999f; // Decay para RMS
    
    // Buffer temporal para plate (RT-safe: pre-allocado, tamaño máximo)
    static constexpr int MAX_BLOCK_SIZE = 2048; // Tamaño máximo de bloque esperado
    juce::AudioBuffer<float> plateBuffer;
    
    // Parámetros previos para detectar cambios (RT-safe: solo lectura desde audio thread)
    float prevMetalness = 0.5f;
    int parameterUpdateCounter = 0; // Contador para actualizar periódicamente
    static constexpr int PARAMETER_UPDATE_INTERVAL = 4; // Actualizar cada 4 bloques (~10ms a 44.1kHz/512)
    
    //==============================================================================
    /** Procesa eventos de la cola lock-free (llamado desde audio thread) */
    void processEventQueue();

    /** Aplica limiter suave */
    float applyLimiter(float sample);

    /** Actualiza el nivel de salida */
    void updateOutputLevel(float sample);
};
