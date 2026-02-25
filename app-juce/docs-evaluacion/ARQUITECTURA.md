# Arquitectura del módulo app-juce (PAS1)

Descripción del flujo de audio, cola de eventos y componentes del sintetizador para evaluación externa.

## Flujo de datos

**En esta versión (v1), PlateSynth está deshabilitado por defecto** (`enablePlateSynth = false`): PAS ignora `/plate` en ingestión y la salida de audio no depende de `/plate`. El esquema siguiente describe el flujo cuando el bypass está desactivado; con bypass activo, la rama `/plate` no se ejecuta y PlateSynth no recibe triggers.

```
OSC (/hit, /state, /plate)  →  MainComponent  →  Cola lock-free (HitEvent)
                                                         ↓
                                              processEventQueue() [audio thread]
                                                         ↓
                                              VoiceManager.triggerVoice()
                                                         ↓
                                              ModalVoice (N voces) + PlateSynth
                                                         ↓
                                              Suma de voces + plate  →  Clipper  →  Salida
```

1. **MainComponent** recibe mensajes OSC en el message thread, valida `/hit` (id, x, y, energy, surface) y llama a `synthesisEngine.triggerVoiceFromOSC(...)`. Si el bypass de PlateSynth está activo, los mensajes `/plate` se ignoran (no se llama a triggerPlateFromOSC).
2. **SynthesisEngine** no procesa el hit de inmediato: escribe un `HitEvent` en una **cola lock-free** (`AbstractFifo` + `eventQueue[]`). Si la cola está llena, el evento se descarta y se incrementa `hitsDiscarded`.
3. En el **audio thread**, `renderNextBlock()` llama a `processEventQueue()`: se leen hasta `MAX_HITS_PER_BLOCK` eventos por bloque y cada uno se convierte en `voiceManager.triggerVoice(...)`.
4. **VoiceManager** asigna una voz del pool (o hace voice stealing si no hay libres) y la voz **ModalVoice** genera el sonido (excitación + 6 modos resonantes + ADSR percusivo Decay→Idle + opcional sub-osc). Parámetros globales (metalness, brightness, damping) se aplican periódicamente desde atómicos vía `updateGlobalParameters`.
5. **PlateSynth** se dispara por mensajes `/plate` (parámetros atómicos) y se mezcla con las voces modales antes del clipper — solo cuando el bypass de PlateSynth está desactivado.

## Componentes y archivos

| Componente | Archivo | Rol |
|------------|---------|-----|
| **SynthesisEngine** | `Source/SynthesisEngine.h`, `.cpp` | Orquesta cola de eventos, VoiceManager, PlateSynth; aplica clipper y mide nivel. |
| **VoiceManager** | `Source/VoiceManager.h`, `.cpp` | Pool de voces (4–12 activas, hasta 32 pre-allocadas), voice stealing, `renderNextBlock` sumando voces. |
| **ModalVoice** | `Source/ModalVoice.h`, `.cpp` | Una voz modal: 6 modos resonantes (biquad), excitación 4–8 ms (Noise/Sine/Square/Saw/etc.), ADSR, sub-osc, formant opcional. |
| **PlateSynth** | `Source/PlateSynth.h`, `.cpp` | Síntesis de placa: 6 modos, excitación por ruido, 8 modos de placa (0–7), fail-safe 2 s sin updates. |
| **MainComponent** | `Source/MainComponent.h`, `.cpp` | UI, receptor OSC (puerto 9000), mapeo `/hit` → parámetros de síntesis y llamada a `triggerVoiceFromOSC`. |

## Agregación multi-evento (v1, opcional)

Con **agregación activa** (`enableAggregation = true`, por defecto en v1), los mensajes `/hit` no se encolan como eventos crudos: se acumulan en **HitAggregator** por cuadrante (4 quads) en ventanas de 20 ms. Al cerrar cada ventana se generan hasta 4 **FusedHitSnapshot** (uno por cuadrante no vacío) que se escriben en una **cola lock-free de fused** (FUSED_QUEUE_SIZE 256). El **audio thread** drena esta cola en `processEventQueue` y llama a `triggerVoice` por cada snapshot; el pan (gL/gR constant-power) se aplica por voz en VoiceManager. Así se reduce el número de eventos audibles y se mejora la cobertura cuando hay muchas partículas. Rollback: `enableAggregation = false` restaura el flujo de hits crudos a la cola original.

## Presupuesto de voces por cuadrante (R4)

**VoiceManager** reserva `maxVoicesPerQuadrant` voces por cuadrante (0..3); el resto forman un pool compartido. Al disparar un evento fusionado con `quadrant` 0..3 se prioriza una voz libre del presupuesto de ese cuadrante y, si no hay, del pool compartido; el voice stealing prefiere robar dentro del mismo cuadrante. Así se evita que un solo cuadrante acapare todas las voces.

## Límites que afectan saturación y cobertura

| Constante / parámetro | Valor | Efecto |
|------------------------|-------|--------|
| **EVENT_QUEUE_SIZE** | 128 | Tamaño de la cola de hits crudos. Si se llena, los nuevos hits se descartan (`hitsDiscarded`). |
| **FUSED_QUEUE_SIZE** | 256 | Cola de eventos fusionados (agregación). Overflow: drop-new, `fusedHitsDiscardedQueue++`. |
| **MAX_HITS_PER_BLOCK** | 32 | Máximo de eventos (crudos + fused) procesados por bloque de audio. Si hay >64 pendientes, se procesan solo 16 por bloque. |
| **maxVoices** | 4–12 (configurable en UI) | Número de voces activas. Presupuesto por cuadrante (p. ej. 2 por quad) + pool compartido; voice stealing por menor amplitud residual o más antigua. |
| **Buffer de audio** | Recomendado 256–512 samples | Buffer muy grande (ej. 1024) implica menos llamadas a `renderNextBlock` por segundo y la cola se drena más lento. |

## Diagrama de bloques (simplificado)

```
                    MainComponent
                         │
    OSC /hit ───────────►│ mapOSCHitToEvent() ──► triggerVoiceFromOSC()
                         │                              │
                         │                    HitEvent  │
                         │                    escrito   │
                         │                    en cola   ▼
                         │                    lock-free ┌─────────────────┐
                         │                              │ eventFifo       │
                         │                              │ eventQueue[128] │
                         │                              └────────┬────────┘
                         │                                       │
    getNextAudioBlock()  │                              processEventQueue()
    (audio thread) ──────┼──────────────────────────────►│ (máx 32/block)
                         │                                       │
                         │                              triggerVoice() × N
                         │                                       ▼
                         │                              ┌─────────────────┐
                         │                              │ VoiceManager   │
                         │                              │ ModalVoice × 12│
                         │                              └────────┬────────┘
                         │                                       │
                         │  plateVolume                          │ + PlateSynth (si bypass desactivado)
                         │◄──────────────────────────────────────┘
                         │
                         ▼
                  Clipper ──► Salida (L/R)
```

## Métricas expuestas en UI

- **Hits received / triggered / discarded:** contadores atómicos; `hitsReceived` se incrementa al escribir en cola (si hay espacio), `hitsTriggered` al procesar en audio thread, `hitsDiscarded` cuando la cola está llena.
- **Hit Coverage:** `hits_triggered / hits_received` (ratio). Objetivo típico de evaluación: ≥90% en escenario normal.
- **Output level:** RMS aproximado de la salida.
- **Active voices:** número de voces activas en el VoiceManager.
