#pragma once

/**
 * Snapshot de un evento fusionado (varios hits en ventana 20 ms por cuadrante).
 * Escrito en message thread, leído en audio thread (cola lock-free).
 * waveformAsInt: ModalVoice::ExcitationWaveform como int (0=Noise, 1=Sine, ...).
 * quadrant: 0..3 para presupuesto de voces (R4); -1 si no aplica.
 */
struct FusedHitSnapshot
{
    float amplitude   = 0.0f;
    float baseFreq    = 300.0f;
    float damping     = 0.5f;
    float brightness  = 0.5f;
    float metalness   = 0.5f;
    int   waveformAsInt = 0;
    float subOscMix   = 0.0f;
    float gainL       = 1.0f;  // constant-power pan
    float gainR       = 1.0f;
    int   quadrant    = -1;    // 0..3 para reserva por cuadrante
};
