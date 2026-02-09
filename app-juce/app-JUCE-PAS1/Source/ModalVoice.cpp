#include "ModalVoice.h"

//==============================================================================
ModalVoice::ModalVoice()
{
    reset();
}

//==============================================================================
void ModalVoice::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    reset();
    updateFilterCoefficients();
}

//==============================================================================
void ModalVoice::setParameters(float baseFreq, float amplitude, float damping, 
                                float brightness, float metalness)
{
    bool needsUpdate = false;
    
    if (std::abs(currentBaseFreq - baseFreq) > 0.1f)
    {
        currentBaseFreq = baseFreq;
        needsUpdate = true;
    }
    
    currentAmplitude = juce::jlimit(0.0f, 1.0f, amplitude);
    currentDamping = juce::jlimit(0.0f, 1.0f, damping);
    currentBrightness = juce::jlimit(0.0f, 1.0f, brightness);
    currentMetalness = juce::jlimit(0.0f, 1.0f, metalness);
    
    if (needsUpdate)
    {
        updateFilterCoefficients();
    }
    
    // Actualizar decay rate basado en damping
    // Damping 0 = decay muy lento, damping 1 = decay muy rápido
    float decayTime = juce::jmap(currentDamping, 0.05f, 2.0f, 0.0f, 1.0f); // 50ms a 2 segundos
    envelopeDecay = decayTime > 0.0f ? std::exp(-1.0f / (decayTime * (float)currentSampleRate)) : 0.0f;
}

//==============================================================================
void ModalVoice::trigger()
{
    generateExcitation();
    isExciting = true;
    excitationPosition = 0;
    envelope = 1.0f;
    residualAmplitude = currentAmplitude;
}

//==============================================================================
float ModalVoice::renderNextSample()
{
    // RT-SAFE: Check rápido de actividad (evitar procesamiento si está inactiva)
    if (envelope <= 0.0001f && !isExciting)
        return 0.0f;
    
    // Generar excitación si aún está activa - optimizado RT-safe
    float excitation = 0.0f;
    if (isExciting && excitationPosition < excitationLength)
    {
        // Calcular envolvente (optimizado: evitar división cuando sea posible)
        // Para longitudes típicas (64-128 samples), la división es aceptable
        float envFactor = 1.0f - ((float)excitationPosition / (float)excitationLength);
        excitation = excitationBuffer[excitationPosition] * envFactor;
        excitationPosition++;
        
        if (excitationPosition >= excitationLength)
        {
            isExciting = false;
        }
    }
    
    // RT-SAFE: Procesar modos (optimizado para 4 modos)
    float output = modes[0].process(excitation) + modes[1].process(excitation) + 
                   modes[2].process(excitation) + modes[3].process(excitation);
    
    // Aplicar envolvente de decaimiento global (optimizado: combinar multiplicaciones)
    float envAmp = envelope * currentAmplitude;
    output *= envAmp;
    envelope *= envelopeDecay;
    
    // Actualizar amplitud residual (para voice stealing) - solo si es significativo
    // RT-SAFE: Usar multiplicación en lugar de std::abs() cuando sea posible
    if (envelope > 0.001f)
    {
        float absOutput = output >= 0.0f ? output : -output;
        residualAmplitude = absOutput * envelope;
    }
    else
    {
        residualAmplitude = 0.0f;
    }
    
    return output;
}

//==============================================================================
bool ModalVoice::isActive() const
{
    // Considerar activa si la envolvente está por encima de un umbral muy bajo
    return (envelope > 0.0001f) || isExciting; // Usar lógico OR para claridad
}

//==============================================================================
float ModalVoice::getResidualAmplitude() const
{
    return residualAmplitude;
}

//==============================================================================
void ModalVoice::reset()
{
    for (int i = 0; i < NUM_MODES; i++)
    {
        modes[i].reset();
    }
    
    excitationPosition = 0;
    excitationLength = 0;
    isExciting = false;
    envelope = 0.0f;
    residualAmplitude = 0.0f;
}

//==============================================================================
void ModalVoice::updateFilterCoefficients()
{
    for (int i = 0; i < NUM_MODES; i++)
    {
        float freq = calculateModeFrequency(i);
        float gain = calculateModeGain(i);
        float q = calculateModeQ(i);
        
        modes[i].setCoefficients(freq, q, gain, currentSampleRate);
    }
}

//==============================================================================
void ModalVoice::generateExcitation()
{
    // Generar excitación con más contenido de alta frecuencia para timbre metálico
    // Duración variable: 3-8ms (más corta = más aguda, más metálica)
    float durationMs = 4.0f + random.nextFloat() * 4.0f; // 4-8ms variable
    excitationLength = (int)(durationMs * 0.001f * currentSampleRate);
    excitationLength = juce::jlimit(1, 128, excitationLength);
    
    // Generar ruido blanco con énfasis en altas frecuencias
    // Usar diferenciación para crear click más agudo (impulso diferenciado)
    const float scale = 2.0f;
    const float offset = -1.0f;
    float prevSample = 0.0f;
    
    for (int i = 0; i < excitationLength; i++)
    {
        // Generar ruido blanco
        float noise = random.nextFloat() * scale + offset; // -1 a 1
        
        // Aplicar diferenciación (high-pass implícito) para más contenido de alta frecuencia
        // Esto crea un click más agudo y metálico
        float diff = noise - prevSample;
        prevSample = noise;
        
        // Aplicar envolvente exponencial para suavizar el inicio
        float env = 1.0f - ((float)i / (float)excitationLength);
        env = env * env; // Envolvente cuadrática para más suavidad
        
        excitationBuffer[i] = diff * env * 1.5f; // Amplificar ligeramente para compensar diferenciación
    }
}

//==============================================================================
float ModalVoice::calculateModeFrequency(int modeIndex) const
{
    // Frecuencia base multiplicada por factor inarmónico
    float inharmonicFactor = INHARMONIC_FACTORS[modeIndex];
    
    // Aplicar metalness: 0 = armónico, 1 = muy inarmónico
    float spread = 1.0f + currentMetalness * (inharmonicFactor - 1.0f);
    
    return currentBaseFreq * spread;
}

//==============================================================================
float ModalVoice::calculateModeGain(int modeIndex) const
{
    // Ganancia base del modo
    float baseGain = MODE_GAINS[modeIndex];
    
    // Aplicar brightness: 0 = más graves, 1 = más agudos
    // Modos más altos (índices mayores) se potencian con brightness
    float brightnessFactor = 1.0f + currentBrightness * ((float)modeIndex / (float)(NUM_MODES - 1));
    
    return baseGain * brightnessFactor;
}

//==============================================================================
float ModalVoice::calculateModeQ(int modeIndex) const
{
    // Q más alto = resonancia más estrecha y decay más largo = más "ringing" metálico
    // Modos más altos típicamente tienen Q más alto (más "ringing")
    // Base Q aumentado para timbre metálico: 25-60 (en lugar de 10-25)
    float baseQ = 25.0f + (float)modeIndex * 11.67f; // Q de 25 a 60 para 4 modos
    
    // Damping afecta el Q: más damping = Q más bajo = decay más rápido
    // Pero también afectamos el decay directamente, así que Q puede ser más constante
    // Reducir Q hasta 40% con damping máximo (menos reducción que antes para mantener ringing)
    return baseQ * (1.0f - currentDamping * 0.4f);
}
