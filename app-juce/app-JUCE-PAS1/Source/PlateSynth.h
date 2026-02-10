#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Sintetizador de placa metálica con resonadores modales excitados por ruido.
    
    Implementa síntesis de vibración de placa mediante múltiples modos resonantes
    excitados continuamente por ruido blanco. El sistema incluye fail-safe que
    hace fade-out automático si no se reciben updates en 2 segundos.
*/
class PlateSynth
{
public:
    //==============================================================================
    PlateSynth();
    ~PlateSynth() = default;

    //==============================================================================
    /** Prepara el sintetizador con el sample rate */
    void prepare(double sampleRate);

    /** Actualiza parámetros de la placa (RT-safe: usa atomic) */
    void triggerPlate(float freq, float amp, int mode);

    /** Renderiza el siguiente bloque de audio (RT-safe) */
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    /** Resetea el sintetizador completamente */
    void reset();

private:
    //==============================================================================
    static constexpr int NUM_PLATE_MODES = 6; // Número de modos resonantes
    
    // Timeout para fail-safe (2 segundos)
    static constexpr int TIMEOUT_MS = 2000;
    
    //==============================================================================
    struct ResonantFilter
    {
        // Coeficientes del filtro biquad (bandpass resonante)
        float b0, b1, b2, a1, a2;
        
        // Estados del filtro (delay line)
        float x1, x2, y1, y2;
        
        void reset()
        {
            x1 = x2 = y1 = y2 = 0.0f;
        }
        
        void setCoefficients(float freq, float q, float gain, double sampleRate)
        {
            // Diseño de filtro bandpass resonante usando transformación bilineal
            float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sampleRate;
            float cosw = std::cos(w);
            float sinw = std::sin(w);
            float alpha = sinw / (2.0f * q);
            
            float b0_band = sinw / 2.0f;
            float b1_band = 0.0f;
            float b2_band = -sinw / 2.0f;
            float a0_band = 1.0f + alpha;
            float a1_band = -2.0f * cosw;
            float a2_band = 1.0f - alpha;
            
            // Normalizar y aplicar ganancia
            float norm = 1.0f / a0_band;
            b0 = b0_band * norm * gain;
            b1 = b1_band * norm * gain;
            b2 = b2_band * norm * gain;
            a1 = a1_band * norm;
            a2 = a2_band * norm;
        }
        
        float process(float input)
        {
            float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            return output;
        }
    };

    //==============================================================================
    double currentSampleRate = 44100.0;
    
    // Parámetros actuales (atomic para thread safety)
    std::atomic<float> currentFreq{220.0f};
    std::atomic<float> currentAmp{0.0f};
    std::atomic<int> currentMode{0};
    
    // Timeout fail-safe (atomic para thread safety)
    std::atomic<juce::int64> lastUpdateTime{0};
    
    // Filtros resonantes para cada modo
    ResonantFilter modes[NUM_PLATE_MODES];
    
    // Excitación: ruido blanco
    juce::Random noiseGen;
    
    // Envolvente de fade-out para fail-safe
    float fadeOutGain = 1.0f;
    float fadeOutDecay = 0.999f; // Ajustar según sample rate
    
    // Parámetros previos para detectar cambios (RT-safe: solo lectura desde audio thread)
    float prevFreq = 220.0f;
    float prevAmp = 0.0f;
    int prevMode = 0;
    int parameterUpdateCounter = 0;
    static constexpr int PARAMETER_UPDATE_INTERVAL = 4; // Actualizar cada 4 bloques
    
    //==============================================================================
    /** Calcula factores inarmónicos según modo de placa */
    float getInharmonicFactor(int modeIndex, int plateMode) const;
    
    /** Calcula ganancia relativa del modo según modo de placa */
    float getModeGain(int modeIndex, int plateMode) const;
    
    /** Calcula Q del resonador según modo */
    float getModeQ(int modeIndex) const;
    
    /** Actualiza coeficientes de filtros cuando cambian parámetros */
    void updateFilterCoefficients();
    
    /** Verifica timeout y actualiza fade-out gain */
    void updateFailSafe();
};
