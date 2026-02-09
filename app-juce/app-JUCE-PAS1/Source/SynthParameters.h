#pragma once

//==============================================================================
/**
    Parámetros globales del sintetizador
*/
struct SynthParameters
{
    // Parámetros de polyphony
    int maxVoices = 32;
    
    // Parámetros de síntesis
    float metalness = 0.5f;      // 0-1: Spread inarmónico
    float brightness = 0.5f;     // 0-1: Balance graves/agudos
    float damping = 0.5f;         // 0-1: Decaimiento (0 = largo, 1 = corto)
    
    // Parámetros de procesamiento master
    float drive = 0.0f;           // 0-1: Saturación suave
    float reverbMix = 0.0f;       // 0-1: Mezcla de reverb (opcional para Fase 6)
    bool limiterEnabled = true;   // Enable/disable limiter
    
    // Parámetros de trigger manual (para testing)
    float testFreq = 220.0f;      // Frecuencia para trigger manual
    float testAmplitude = 0.7f;   // Amplitud para trigger manual
};
