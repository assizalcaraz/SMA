#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Resonador modal con modos inarmónicos para timbre metálico "Coin Cascade"
    
    Cada voz implementa un resonador modal con múltiples modos (3-8) que se excitan
    con un impulso corto (noise burst) y decaen según el parámetro de damping.
*/
class ModalVoice
{
public:
    //==============================================================================
    ModalVoice();
    ~ModalVoice() = default;

    //==============================================================================
    /** Prepara la voz con el sample rate */
    void prepare(double sampleRate);

    /** Configura los parámetros de la voz */
    void setParameters(float baseFreq, float amplitude, float damping, 
                       float brightness, float metalness);

    /** Trigger la voz (inicia la excitación) */
    void trigger();

    /** Renderiza el siguiente sample */
    float renderNextSample();

    /** Verifica si la voz está activa (aún resonando) */
    bool isActive() const;

    /** Obtiene la amplitud residual actual (para voice stealing) */
    float getResidualAmplitude() const;

    /** Resetea la voz completamente */
    void reset();

private:
    //==============================================================================
    static constexpr int NUM_MODES = 2; // Número de modos resonantes (reducido a 2 para estabilidad RT)
    
    // Factores inarmónicos para cada modo (valores típicos para timbre metálico)
    static constexpr float INHARMONIC_FACTORS[NUM_MODES] = {
        1.0f,      // Modo fundamental
        2.76f      // Segundo modo inarmónico
    };

    // Ganancias relativas por modo (para brightness)
    static constexpr float MODE_GAINS[NUM_MODES] = {
        1.0f,      // Fundamental más fuerte
        0.7f       // Segundo modo
    };

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
    
    // Parámetros actuales
    float currentBaseFreq = 220.0f;
    float currentAmplitude = 0.0f;
    float currentDamping = 0.5f;
    float currentBrightness = 0.5f;
    float currentMetalness = 0.5f;
    
    // Filtros resonantes para cada modo
    ResonantFilter modes[NUM_MODES];
    
    // Excitación: buffer de ruido con envolvente
    juce::Random random;
    float excitationBuffer[128]; // Máximo 5ms a 48kHz (reducido para mejor rendimiento)
    int excitationLength = 0;
    int excitationPosition = 0;
    bool isExciting = false;
    
    // Envolvente de decaimiento global
    float envelope = 0.0f;
    float envelopeDecay = 0.0f;
    
    // Amplitud residual para voice stealing
    float residualAmplitude = 0.0f;
    
    //==============================================================================
    void updateFilterCoefficients();
    void generateExcitation();
    float calculateModeFrequency(int modeIndex) const;
    float calculateModeGain(int modeIndex) const;
    float calculateModeQ(int modeIndex) const;
};
