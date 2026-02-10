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
    /** Formas de onda disponibles para la excitación */
    enum class ExcitationWaveform
    {
        Noise,      // Ruido diferenciado (actual)
        Sine,       // Sinusoidal suave
        Square,     // Cuadrada agresiva
        Saw,        // Diente de sierra brillante
        Triangle,   // Triangular suave
        Click,      // Impulso delta percusivo
        Pulse       // Pulso estrecho muy agudo
    };

    //==============================================================================
    ModalVoice();
    ~ModalVoice() = default;

    //==============================================================================
    /** Prepara la voz con el sample rate */
    void prepare(double sampleRate);

    /** Configura los parámetros de la voz */
    void setParameters(float baseFreq, float amplitude, float damping, 
                       float brightness, float metalness,
                       ExcitationWaveform waveform = ExcitationWaveform::Noise,
                       float subOscMix = 0.0f);

    /** Trigger la voz (inicia la excitación) */
    void trigger();

    /** Renderiza el siguiente sample */
    float renderNextSample();

    /** Verifica si la voz está activa (aún resonando) */
    bool isActive() const;

    /** Obtiene la amplitud residual actual (para voice stealing) */
    float getResidualAmplitude() const;

    /** Obtiene la frecuencia base actual */
    float getCurrentBaseFreq() const { return currentBaseFreq; }

    /** Obtiene la amplitud actual */
    float getCurrentAmplitude() const { return currentAmplitude; }

    /** Resetea la voz completamente */
    void reset();

private:
    //==============================================================================
    static constexpr int NUM_MODES = 6; // Número de modos resonantes (aumentado para timbre metálico más rico)
    
    // Factores inarmónicos para cada modo (valores típicos para timbre metálico)
    static constexpr float INHARMONIC_FACTORS[NUM_MODES] = {
        1.0f,      // Modo 0: fundamental
        2.76f,     // Modo 1: segundo modo inarmónico
        5.40f,     // Modo 2: tercer modo inarmónico
        8.93f,     // Modo 3: cuarto modo inarmónico
        13.34f,    // Modo 4: quinto modo inarmónico
        18.65f     // Modo 5: sexto modo inarmónico
    };

    // Ganancias relativas por modo (para brightness)
    static constexpr float MODE_GAINS[NUM_MODES] = {
        1.0f,      // Modo 0: fundamental más fuerte
        0.8f,      // Modo 1: segundo modo
        0.9f,      // Modo 2: tercer modo (medio-alto, más brillante)
        0.7f,      // Modo 3: cuarto modo (alto)
        0.6f,      // Modo 4: quinto modo
        0.5f       // Modo 5: sexto modo
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
    struct SubOscillator
    {
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        ExcitationWaveform waveform = ExcitationWaveform::Square; // Por defecto square
        
        void setFrequency(float freq, double sampleRate)
        {
            phaseIncrement = 2.0f * juce::MathConstants<float>::pi * freq / (float)sampleRate;
        }
        
        void reset()
        {
            phase = 0.0f;
        }
        
        float renderNextSample()
        {
            float output = 0.0f;
            
            switch (waveform)
            {
                case ExcitationWaveform::Square:
                    output = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
                    break;
                case ExcitationWaveform::Sine:
                    output = std::sin(phase);
                    break;
                case ExcitationWaveform::Saw:
                    output = (phase / juce::MathConstants<float>::pi) - 1.0f;
                    break;
                case ExcitationWaveform::Triangle:
                    if (phase < juce::MathConstants<float>::pi)
                        output = (phase / juce::MathConstants<float>::pi) * 2.0f - 1.0f;
                    else
                        output = 1.0f - ((phase - juce::MathConstants<float>::pi) / juce::MathConstants<float>::pi) * 2.0f;
                    break;
                default:
                    output = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f; // Default to square
                    break;
            }
            
            phase += phaseIncrement;
            if (phase >= 2.0f * juce::MathConstants<float>::pi)
                phase -= 2.0f * juce::MathConstants<float>::pi;
            
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
    
    // Variación aleatoria sutil en frecuencias (para timbre más rico y menos repetitivo)
    float frequencyVariation[NUM_MODES];
    juce::Random freqRandom;
    
    // Filtros resonantes para cada modo
    ResonantFilter modes[NUM_MODES];
    
    // Filtro formant opcional para más carácter tímbrico (peaking filter)
    struct FormantFilter
    {
        float b0, b1, b2, a1, a2;
        float x1, x2, y1, y2;
        
        void reset()
        {
            x1 = x2 = y1 = y2 = 0.0f;
        }
        
        void setCoefficients(float freq, float q, float gain, double sampleRate)
        {
            // Peaking filter (EQ boost/cut) - implementación simplificada y RT-safe
            float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sampleRate;
            float cosw = std::cos(w);
            float sinw = std::sin(w);
            float A = std::sqrt(gain); // Linear gain
            float alpha = sinw / (2.0f * q);
            
            float b0_peak = 1.0f + alpha * A;
            float b1_peak = -2.0f * cosw;
            float b2_peak = 1.0f - alpha * A;
            float a0_peak = 1.0f + alpha / A;
            float a1_peak = -2.0f * cosw;
            float a2_peak = 1.0f - alpha / A;
            
            float norm = 1.0f / a0_peak;
            b0 = b0_peak * norm;
            b1 = b1_peak * norm;
            b2 = b2_peak * norm;
            a1 = a1_peak * norm;
            a2 = a2_peak * norm;
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
    
    FormantFilter formantFilter;
    bool formantEnabled = true; // Habilitado por defecto para timbre metálico
    float formantFreq = 3000.0f; // Frecuencia formant típica para metales (3kHz)
    float formantQ = 2.0f;
    float formantGain = 1.3f; // Boost ligero en formant
    
    // Excitación: buffer de ruido con envolvente
    juce::Random random;
    float excitationBuffer[128]; // Máximo 5ms a 48kHz (reducido para mejor rendimiento)
    int excitationLength = 0;
    int excitationPosition = 0;
    bool isExciting = false;
    ExcitationWaveform currentWaveform = ExcitationWaveform::Noise;
    
    // Envolvente de decaimiento global
    float envelope = 0.0f;
    float envelopeDecay = 0.0f;
    
    // Amplitud residual para voice stealing
    float residualAmplitude = 0.0f;
    
    // Sub-oscillator
    SubOscillator subOsc;
    float currentSubOscMix = 0.0f;
    
    //==============================================================================
    void updateFilterCoefficients();
    void generateExcitation();
    void generateNoiseExcitation();
    void generateSineExcitation();
    void generateSquareExcitation();
    void generateSawExcitation();
    void generateTriangleExcitation();
    void generateClickExcitation();
    void generatePulseExcitation();
    float renderSubOscillator();
    float calculateModeFrequency(int modeIndex) const;
    float calculateModeGain(int modeIndex) const;
    float calculateModeQ(int modeIndex) const;
};
