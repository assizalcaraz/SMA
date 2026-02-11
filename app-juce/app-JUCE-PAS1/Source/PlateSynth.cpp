#include "PlateSynth.h"

//==============================================================================
PlateSynth::PlateSynth()
{
    reset();
}

//==============================================================================
void PlateSynth::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    
    // Ajustar decay rate del fade-out según sample rate
    // Decay de ~100ms para fade-out suave
    fadeOutDecay = std::exp(-1.0f / (0.1f * (float)sampleRate));
    
    // Inicializar filtros
    for (int i = 0; i < NUM_PLATE_MODES; i++)
    {
        modes[i].reset();
    }
    
    // Inicializar valores previos
    prevFreq = currentFreq.load();
    prevAmp = currentAmp.load();
    prevMode = currentMode.load();
    parameterUpdateCounter = 0;
    
    // Actualizar coeficientes iniciales
    updateFilterCoefficients();
}

//==============================================================================
void PlateSynth::triggerPlate(float freq, float amp, int mode)
{
    // RT-SAFE: Actualizar atomic (llamado desde OSC thread)
    currentFreq.store(juce::jlimit(20.0f, 2000.0f, freq));
    currentAmp.store(juce::jlimit(0.0f, 1.0f, amp));
    currentMode.store(juce::jlimit(0, 7, mode));
    
    // Actualizar timestamp para fail-safe
    lastUpdateTime.store(juce::Time::currentTimeMillis());
}

//==============================================================================
void PlateSynth::renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    // RT-SAFE: Verificar timeout y actualizar fade-out
    updateFailSafe();
    
    // RT-SAFE: Actualizar parámetros periódicamente (cada N bloques)
    parameterUpdateCounter++;
    if (parameterUpdateCounter >= PARAMETER_UPDATE_INTERVAL)
    {
        parameterUpdateCounter = 0;
        
        // Leer parámetros actuales (atomic, thread-safe)
        float freq = currentFreq.load();
        float amp = currentAmp.load();
        int mode = currentMode.load();
        
        // Actualizar coeficientes solo si cambió algo significativamente
        // Usar umbrales más grandes para reducir actualizaciones innecesarias
        if (std::abs(freq - prevFreq) > 1.0f ||  // Cambió de 0.1f a 1.0f para reducir updates
            mode != prevMode)  // Removido check de amp (no afecta coeficientes)
        {
            updateFilterCoefficients();
            prevFreq = freq;
            prevAmp = amp;
            prevMode = mode;
        }
    }
    
    // Leer amplitud actual (atomic, thread-safe)
    float amp = currentAmp.load();
    
    // Optimización: Si amplitud es muy baja, no procesar (ahorro de CPU)
    if (amp < 0.001f && fadeOutGain < 0.001f)
    {
        // Silencio total, solo limpiar buffer
        buffer.clear(startSample, numSamples);
        return;
    }
    
    // Renderizar para cada canal
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        float* channelData = buffer.getWritePointer(channel, startSample);
        
        // Aplicar fade-out gain una vez (optimización)
        float effectiveAmp = amp * fadeOutGain;
        
        for (int sample = 0; sample < numSamples; sample++)
        {
            // Generar ruido blanco como excitación
            // Reducir ganancia base del plate para balancear con partículas (0.1 = -20dB aprox)
            // El plate es inherentemente más fuerte que las partículas, necesita más atenuación
            float noise = (noiseGen.nextFloat() * 2.0f - 1.0f) * effectiveAmp * 0.1f;
            
            // Procesar por cada modo resonante y sumar
            float output = 0.0f;
            for (int i = 0; i < NUM_PLATE_MODES; i++)
            {
                output += modes[i].process(noise);
            }
            
            // Escribir al buffer
            channelData[sample] = output;
        }
    }
}

//==============================================================================
void PlateSynth::reset()
{
    for (int i = 0; i < NUM_PLATE_MODES; i++)
    {
        modes[i].reset();
    }
    
    fadeOutGain = 1.0f;
    prevFreq = 220.0f;
    prevAmp = 0.0f;
    prevMode = 0;
    parameterUpdateCounter = 0;
    
    currentFreq.store(220.0f);
    currentAmp.store(0.0f);
    currentMode.store(0);
    lastUpdateTime.store(0);
}

//==============================================================================
float PlateSynth::getInharmonicFactor(int modeIndex, int plateMode) const
{
    // Factores inarmónicos base (similar a ModalVoice pero variados según plateMode)
    // Plate mode 0 = delgada (más inarmónico), mode 7 = gruesa (menos inarmónico)
    
    static constexpr float BASE_FACTORS[NUM_PLATE_MODES] = {
        1.0f,      // Modo 0: fundamental
        2.76f,     // Modo 1
        5.40f,     // Modo 2
        8.93f,     // Modo 3
        13.34f,    // Modo 4
        18.65f     // Modo 5
    };
    
    // Variar factores según plateMode (0-7)
    // Mode 0: más inarmónico (factores más altos)
    // Mode 7: menos inarmónico (factores más cercanos a armónicos)
    float modeVariation = 1.0f + (plateMode * 0.1f); // 1.0 a 1.7
    
    return BASE_FACTORS[modeIndex] * modeVariation;
}

//==============================================================================
float PlateSynth::getModeGain(int modeIndex, int plateMode) const
{
    // Ganancias relativas por modo (ajustadas según plateMode)
    
    static constexpr float BASE_GAINS[NUM_PLATE_MODES] = {
        1.0f,      // Modo 0: fundamental más fuerte
        0.8f,      // Modo 1
        0.9f,      // Modo 2
        0.7f,      // Modo 3
        0.6f,      // Modo 4
        0.5f       // Modo 5
    };
    
    // Variar ganancias según plateMode
    // Mode 0: más brillante (ganancias altas en modos superiores)
    // Mode 7: más oscuro (ganancias bajas en modos superiores)
    float modeGainMod = 1.0f - (plateMode * 0.05f); // 1.0 a 0.65
    
    return BASE_GAINS[modeIndex] * modeGainMod;
}

//==============================================================================
float PlateSynth::getModeQ(int modeIndex) const
{
    // Q del resonador (mayor Q = más resonante, más estrecho)
    // Modos más altos tienen Q más alto (más resonantes)
    
    static constexpr float MODE_Q[NUM_PLATE_MODES] = {
        20.0f,     // Modo 0: Q moderado
        25.0f,     // Modo 1
        30.0f,     // Modo 2
        35.0f,     // Modo 3
        40.0f,     // Modo 4
        45.0f      // Modo 5: Q más alto
    };
    
    return MODE_Q[modeIndex];
}

//==============================================================================
void PlateSynth::updateFilterCoefficients()
{
    // Leer parámetros actuales (atomic, thread-safe)
    float freq = currentFreq.load();
    int mode = currentMode.load();
    
    // Limitar frecuencia base a rango seguro (evitar problemas numéricos)
    freq = juce::jlimit(20.0f, 2000.0f, freq);
    
    // Calcular frecuencia Nyquist (mitad del sample rate)
    float nyquistFreq = (float)currentSampleRate * 0.5f;
    float maxSafeFreq = nyquistFreq * 0.95f; // 95% de Nyquist para seguridad
    
    // Actualizar coeficientes de cada filtro resonante
    for (int i = 0; i < NUM_PLATE_MODES; i++)
    {
        // Calcular frecuencia del modo
        float modeFreq = freq * getInharmonicFactor(i, mode);
        
        // Limitar frecuencia del modo a rango seguro (evitar aliasing e inestabilidad)
        modeFreq = juce::jlimit(20.0f, maxSafeFreq, modeFreq);
        
        // Calcular ganancia del modo
        float modeGain = getModeGain(i, mode);
        
        // Calcular Q del modo
        float modeQ = getModeQ(i);
        
        // Establecer coeficientes del filtro
        modes[i].setCoefficients(modeFreq, modeQ, modeGain, currentSampleRate);
    }
}

//==============================================================================
void PlateSynth::updateFailSafe()
{
    // Verificar timeout (RT-safe: leer atomic)
    juce::int64 lastUpdate = lastUpdateTime.load();
    juce::int64 currentTime = juce::Time::currentTimeMillis();
    juce::int64 timeSinceUpdate = currentTime - lastUpdate;
    
    // Si no hay updates en TIMEOUT_MS, hacer fade-out
    if (lastUpdate > 0 && timeSinceUpdate > TIMEOUT_MS)
    {
        // Aplicar fade-out exponencial
        fadeOutGain *= fadeOutDecay;
        
        // Si fade-out está muy bajo, resetear completamente
        if (fadeOutGain < 0.001f)
        {
            fadeOutGain = 0.0f;
            // Opcional: resetear filtros para evitar resonancias residuales
            // reset(); // Comentado para evitar clicks
        }
    }
    else
    {
        // Si hay updates recientes, restaurar gain gradualmente
        if (fadeOutGain < 1.0f)
        {
            fadeOutGain = juce::jmin(1.0f, fadeOutGain + (1.0f - fadeOutDecay));
        }
    }
}
