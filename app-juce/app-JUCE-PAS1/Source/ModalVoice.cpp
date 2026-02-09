#include "ModalVoice.h"

//==============================================================================
ModalVoice::ModalVoice()
{
    // Inicializar variaciones de frecuencia (serán regeneradas en trigger)
    for (int i = 0; i < NUM_MODES; i++)
    {
        frequencyVariation[i] = 1.0f;
    }
    reset();
}

//==============================================================================
void ModalVoice::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    reset();
    updateFilterCoefficients();
    
    // Preparar filtro formant
    if (formantEnabled)
    {
        formantFilter.setCoefficients(formantFreq, formantQ, formantGain, sampleRate);
    }
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
    
    float prevBrightness = currentBrightness;
    float prevMetalness = currentMetalness;
    
    currentAmplitude = juce::jlimit(0.0f, 1.0f, amplitude);
    currentDamping = juce::jlimit(0.0f, 1.0f, damping);
    currentBrightness = juce::jlimit(0.0f, 1.0f, brightness);
    currentMetalness = juce::jlimit(0.0f, 1.0f, metalness);
    
    // Actualizar coeficientes si cambió frecuencia, brightness o metalness
    if (needsUpdate || std::abs(prevBrightness - currentBrightness) > 0.01f || 
        std::abs(prevMetalness - currentMetalness) > 0.01f)
    {
        updateFilterCoefficients();
    }
    
    // Actualizar decay rate basado en damping - respuesta MUCHO más dramática
    // Damping 0 = decay muy lento (largo), damping 1 = decay muy rápido (corto)
    // Mapeo invertido y más extremo para que sea muy notorio
    float dampingInverted = 1.0f - currentDamping; // Invertir: 0 = corto, 1 = largo
    float dampingCurve = dampingInverted * dampingInverted * dampingInverted; // Curva cúbica para más impacto
    float decayTime = juce::jmap(dampingCurve, 0.01f, 5.0f, 0.0f, 1.0f); // 10ms a 5 segundos (rango muy amplio)
    envelopeDecay = decayTime > 0.0f ? std::exp(-1.0f / (decayTime * (float)currentSampleRate)) : 0.0f;
}

//==============================================================================
void ModalVoice::trigger()
{
    // Regenerar variaciones aleatorias de frecuencia para timbre más rico
    // Variación sutil: ±2% para evitar que suene desafinado
    for (int i = 0; i < NUM_MODES; i++)
    {
        frequencyVariation[i] = 1.0f + (freqRandom.nextFloat() - 0.5f) * 0.04f; // ±2%
    }
    
    // Actualizar coeficientes con las nuevas variaciones
    updateFilterCoefficients();
    
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
    
    // Aplicar filtro formant opcional para más carácter tímbrico
    if (formantEnabled)
    {
        output = formantFilter.process(output);
    }
    
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
    
    formantFilter.reset();
    
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
    
    // Actualizar filtro formant: ajustar frecuencia según brightness para más expresividad
    // Brightness alto = formant más alto (más brillante)
    float formantFreqAdjusted = formantFreq * (1.0f + currentBrightness * 0.5f); // 3kHz a 4.5kHz
    float formantGainAdjusted = formantGain * (0.8f + currentBrightness * 0.4f); // 1.04 a 1.56
    formantFilter.setCoefficients(formantFreqAdjusted, formantQ, formantGainAdjusted, currentSampleRate);
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
    
    // Aplicar metalness mejorado: mapeo no lineal para mayor impacto
    // Usar curva exponencial para que metalness tenga más efecto en valores altos
    float metalnessCurve = currentMetalness * currentMetalness; // Curva cuadrática
    float spread = 1.0f + metalnessCurve * (inharmonicFactor - 1.0f);
    
    // Aplicar variación aleatoria sutil (regenerada en cada trigger)
    float freq = currentBaseFreq * spread * frequencyVariation[modeIndex];
    
    return freq;
}

//==============================================================================
float ModalVoice::calculateModeGain(int modeIndex) const
{
    // Ganancia base del modo
    float baseGain = MODE_GAINS[modeIndex];
    
    // Aplicar brightness MUCHO más dramático: 0 = más graves, 1 = más agudos
    float modePosition = (float)modeIndex / (float)(NUM_MODES - 1); // 0.0 a 1.0
    
    // Curva más agresiva para brightness
    float brightnessCurve = currentBrightness * currentBrightness; // Curva cuadrática
    
    if (brightnessCurve > 0.5f)
    {
        // Brightness alto: reducir graves MUCHO, potenciar agudos MUCHO
        float brightAmount = (brightnessCurve - 0.5f) * 2.0f; // 0.0 a 1.0
        float reductionFactor = 1.0f - (brightAmount * (1.0f - modePosition) * 0.9f); // Reducir graves hasta 90%
        float boostFactor = 1.0f + (brightAmount * modePosition * 3.0f); // Potenciar agudos hasta 300%
        return baseGain * reductionFactor * boostFactor;
    }
    else
    {
        // Brightness bajo: potenciar graves MUCHO, reducir agudos MUCHO
        float darkAmount = (0.5f - brightnessCurve) * 2.0f; // 0.0 a 1.0
        float boostFactor = 1.0f + (darkAmount * (1.0f - modePosition) * 2.0f); // Potenciar graves hasta 200%
        float reductionFactor = 1.0f - (darkAmount * modePosition * 0.8f); // Reducir agudos hasta 80%
        return baseGain * boostFactor * reductionFactor;
    }
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
