# PAS Review Pack — Re-evaluación externa del módulo PAS (JUCE)

Documento para que un revisor externo audite el módulo PAS del SMA: archivos relevantes, esquema OSC, mapeo física→sonido y puntos donde se pierden eventos (consistencia visual vs auditiva).

**Código de referencia:** PAS principal en `app-juce/app-JUCE-PAS1/`; ISTR en `Particles/` (y `entrega-istr/Particles/` como copia equivalente para OSC).

---

## A) PAS Review Pack (lista de archivos)

### Build / config

| Ruta | Propósito |
|------|-----------|
| `app-juce/app-JUCE-PAS1/app-JUCE-PAS1.jucer` | Proyecto Projucer; define Source (Main, MainComponent, SynthesisEngine, VoiceManager, ModalVoice, PlateSynth, SynthParameters) y módulos JUCE (incl. juce_osc). Export MacOSX en Builds/MacOSX. |
| — | No hay CMakeLists en PAS; solo Projucer → Xcode. |

### Docs / notas de ejecución

| Ruta | Propósito |
|------|-----------|
| `entrega-evaluacion-externa-pas1/README.md` | Cómo evaluar: abrir .jucer, exportar MacOSX, compilar; OSC puerto 9000; formato /hit id x y energy surface. |
| `app-juce/docs-evaluacion/ARQUITECTURA.md` | Flujo OSC → cola → audio thread; límites EVENT_QUEUE_SIZE, MAX_HITS_PER_BLOCK; métricas UI. |

### OSC ingest y cola de eventos

| Ruta | Propósito / clases o funciones clave |
|------|-------------------------------------|
| `app-juce/app-JUCE-PAS1/Source/MainComponent.h` | `OSCReceiver`, `OSCReceiver::Listener<MessageLoopCallback>`, `oscMessageReceived`, `mapOSCHitToEvent`, `mapOSCPlateToEvent`, `updateOSCState`. |
| `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp` | `oscReceiver.connect(9000)`, routing `/hit`, `/state`, `/plate`; llamada a `synthesisEngine.triggerVoiceFromOSC()` / `triggerPlateFromOSC()`. Mensajes procesados en message thread (callback JUCE). |
| `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h` | `HitEvent` (baseFreq, amplitude, damping, brightness, metalness, waveform, subOscMix); `AbstractFifo eventFifo` (EVENT_QUEUE_SIZE 128); `eventQueue[EVENT_QUEUE_SIZE]`; `triggerVoiceFromOSC` escribe en cola (RT-safe); si cola llena → `hitsDiscarded++`. Contadores `hitsReceived`, `hitsTriggered`, `hitsDiscarded` con atómicos (memory_order_relaxed). Clipper (soft clip) en salida. |
| `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.cpp` | `triggerVoiceFromOSC` escribe un `HitEvent` en la cola; `processEventQueue()` (audio thread) lee hasta MAX_HITS_PER_BLOCK (32), o 16 si available > 64; cada evento → `voiceManager.triggerVoice()`; contadores `hitsReceived`, `hitsTriggered`, `hitsDiscarded`. |

### Motor de audio

| Ruta | Propósito / funciones clave |
|------|-----------------------------|
| `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.cpp` | `renderNextBlock`: processEventQueue, actualización periódica de parámetros globales (metalness, brightness, damping), voiceManager.renderNextBlock, plateSynth en buffer temporal, suma al buffer principal, clipper, updateOutputLevel. |
| `app-juce/app-JUCE-PAS1/Source/VoiceManager.h`, `.cpp` | Pool de voces (`OwnedArray<ModalVoice>`), hasta MAX_VOICES_LIMIT 32 pre-asignadas; maxVoices 4–12 activas; `findAvailableVoice`, `findVoiceToSteal` (menor amplitud residual o más antigua); `triggerVoice`, `renderNextBlock` suma todas las voces activas. |
| `app-juce/app-JUCE-PAS1/Source/ModalVoice.h`, `.cpp` | Una voz modal: 6 modos (ResonantFilter biquad), excitación 4–8 ms (Noise/Sine/Square/Saw/Triangle/Click/Pulse), ADSR percusivo (Decay→Idle, sin sustain prolongado), sub-osc, formant opcional; `setParameters` / `setGlobalParametersOnly`, `trigger`, `renderNextSample`, `isActive`, `getResidualAmplitude`. |
| `app-juce/app-JUCE-PAS1/Source/PlateSynth.h`, `.cpp` | 6 modos resonantes, excitación ruido; `triggerPlate(freq, amp, mode)` vía atómicos; fail-safe 2 s sin updates (fade-out); `renderNextBlock` escribe en buffer; parámetros 20–2000 Hz, amp 0–1, mode 0–7. |
| `app-juce/app-JUCE-PAS1/Source/SynthParameters.h` | Struct de parámetros globales (maxVoices, metalness, brightness, damping, etc.); no usado directamente por el flujo OSC (el engine usa atómicos en SynthesisEngine). |

### UI y parámetros

| Ruta | Propósito |
|------|-----------|
| `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp` | Sliders: Voices (4–12), Metalness, Brightness, Damping, SubOsc Mix, Pitch Range; ComboBox Waveform; Toggle Clipper; Botón Test Trigger. Controles de Plate eliminados en v1 (PlateSynth deshabilitado por defecto; `enablePlateSynth` permite reactivar). Labels: Output dB, Active Voices, Hit Coverage, Hits triggered/received/discarded, OSC status, OSC messages/s. `globalPresence` se guarda desde `/state` pero no se aplica a ganancia master (pendiente). |

### Diagnósticos y constantes

| Ubicación | Detalle |
|-----------|---------|
| `SynthesisEngine.h` | `MAX_HITS_PER_BLOCK = 32`, `EVENT_QUEUE_SIZE = 128`; `hitsReceived`, `hitsTriggered`, `hitsDiscarded` atómicos; `getHitCoverageRatio()` = triggered/received. |
| `SynthesisEngine.cpp` processEventQueue | Si `available > MAX_HITS_PER_BLOCK * 2` entonces `numToRead = MAX_HITS_PER_BLOCK / 2` (16). |
| — | No hay profiling hooks adicionales; comentarios DBG en prepareToPlay. |

---

## B) ISTR OSC Schema Pack (solo lo necesario para esquema y métricas)

### Envío OSC y formato

| Ruta | Propósito |
|------|-----------|
| `Particles/src/ofApp.h` | `HitEvent` (id, x, y, energy, surface); `RateLimiter`; `sendHitEvent`, `sendStateMessage`, `sendPlateMessage`; `oscSender`, `oscPort` (9000), `oscHost` (127.0.0.1). |
| `Particles/src/ofApp.cpp` | `setupOSC()` (oscSender.setup(oscHost, oscPort)); `sendHitEvent`: msg `/hit` + addIntArg(id), addFloatArg(x), addFloatArg(y), addFloatArg(energy), addIntArg(surface); `sendStateMessage`: `/state` activity, gesture, presence; `sendPlateMessage`: `/plate` freq, amp, mode (rate limit 20 Hz con plateSendInterval 0.05). |

### Métricas de colisión y throttling

| Ruta | Detalle |
|------|---------|
| `Particles/src/ofApp.cpp` | `checkCollisions()`: bordes (surface 0=L, 1=R, 2=T, 3=B), `generateHitEvent(p, surface)`. |
| — | `calculateHitEnergy(Particle& p, int surface)`: speed_norm = \|vel_pre\|/vel_ref, dist_norm = last_hit_distance/dist_ref, energy = clamp(energy_a * speed_norm + energy_b * dist_norm, 0..1); mínimo 0.001. |
| — | `checkParticleCollisions()`: grid espacial, narrow-phase; `generateParticleHitEvent(p1, p2, collisionPoint)`: energy por velocidad relativa y dist_ref; surface = -1; x,y = centro de colisión normalizado. |
| — | Cooldown por partícula: `hit_cooldown_ms` (default 40 ms); si timeSinceLastHit < cooldown → descarte (hits_discarded_cooldown). |
| — | `processPendingHits()`: para cada pending_hit, si `canEmitHit()` → validated_hits.push_back, consumeToken; si no → hits_discarded_rate. Token bucket: tokens += rate*dt, cap burst; canEmitHit = tokens >= 1 y hits_this_frame < max_per_frame. Defaults: max_hits_per_second 800, burst 1000, max_hits_per_frame 50. |

---

## C) Esquema OSC (resumen)

Detalle completo en [OSC_SCHEMA.md](OSC_SCHEMA.md).

- **`/hit`** — (int32 id, float32 x, float32 y, float32 energy, int32 surface). ISTR: x,y 0..1 normalizado; energy 0..1 (vel_ref, dist_ref, energy_a, energy_b); surface 0..3 bordes, -1 partícula-partícula. PAS: valida size==5 y tipos; descarta si mal formado; mapOSCHitToEvent → amplitude=energy^1.5, brightness=0.3+0.7*energy, damping=0.2+0.6*(1-y), baseFreq 100–800 Hz con random pitchRange, metalness global, waveform por energy; luego triggerVoiceFromOSC.
- **`/state`** — (float32 activity, float32 gesture, float32 presence). ISTR: activity = hits_per_second/max_hits_per_second; PAS: updateOSCState guarda globalPresence (no aplicado a ganancia).
- **`/plate`** — (float32 freq, float32 amp, int32 mode). ISTR: 20–2000 Hz, 0–1, 0–7; envío 20 Hz. PAS: mapOSCPlateToEvent → triggerPlateFromOSC **solo si `enablePlateSynth` es true**. **En esta versión (v1), PlateSynth está deshabilitado por defecto** (`enablePlateSynth = false`): PAS ignora `/plate` en ingestión y la salida de audio no depende de `/plate`; el esquema se mantiene para futura reactivación.

---

## D) Tabla de mapeo física → sonido (quién calcula qué)

| Parámetro sonido | ISTR (calcula/envía) | PAS (calcula/usa) |
|------------------|----------------------|-------------------|
| Posición x,y     | Sí (normalizada 0..1) | Solo usa: damping por (1-y), pan por x (no aplicado), baseFreq no depende de x,y |
| Energy           | Sí (vel_pre, last_hit_distance, vel_ref, dist_ref, energy_a/b) | Sí: amplitude = energy^1.5; brightness = 0.3+0.7*energy; waveform por umbrales energy |
| Surface          | Sí (0..3 bordes, -1 p-p) | No modifica timbre (metalness unificado) |
| Frecuencia base  | No | Sí: 300 Hz ± pitchRange*200*random, clamp 100–800 |
| Metalness        | No | Global (slider), no por surface |
| Damping         | No | Sí: 0.2+0.6*(1-y) |
| Brightness      | No | Sí: 0.3+0.7*energy |
| Waveform        | No | Sí: por energy (Square/Saw/Noise/Sine) |
| SubOscMix        | No | Global (slider) |

---

## E) Nota densidad/consistencia (dónde se pierden eventos)

- **ISTR:** (1) Cooldown por partícula: hit_cooldown_ms (40 ms) → muchos hits de la misma partícula descartados. (2) Token bucket: max_hits_per_second (800), max_hits_per_frame (50) → si en un frame hay >50 validated o la tasa supera 800/s, el resto se descarta (hits_discarded_rate). (3) processPendingHits recorre pending_hits en orden; el orden de descarte es el de llegada (los que no caben en tokens/frame se pierden).
- **PAS:** (1) Cola llena: si eventFifo no tiene espacio, triggerVoiceFromOSC descarta y hitsDiscarded++. (2) Por bloque de audio: processEventQueue lee como máximo MAX_HITS_PER_BLOCK (32) por bloque; si hay >64 pendientes, lee solo 16. Con 48 kHz y bloque 512, ~94 bloques/s → máximo ~3008 eventos/s teóricos si siempre 32/block; en práctica la cola se llena si ISTR envía más de lo que se drena. (3) Voice stealing: solo maxVoices (4–12) sonidos simultáneos; hits que llegan cuando no hay voz libre reutilizan la de menor amplitud residual (o más antigua) — no se “pierden” pero un hit reemplaza a otro. (4) **/plate:** con bypass de PlateSynth activo, los mensajes `/plate` se ignoran en ingestión (no se llama a triggerPlateFromOSC); no afectan la salida de audio.
- **Conclusión breve:** A alto número de partículas, ISTR genera muchos más eventos (border + p-p); el rate limiter (50/frame, 800/s) y el cooldown recortan; luego PAS puede recibir aún más de los que puede procesar por bloque (32 o 16) y/o cola 128, y además solo maxVoices sonidos simultáneos. La percepción de “menos golpes de los que se ven” es coherente con: throttling en ISTR + cola/capa por bloque + polyphony limitada en PAS.

---

## F) Diagrama de flujo de datos (ASCII)

Comportamiento actual: `/hit` y `/state` se procesan; `/plate` se procesa solo si el bypass de PlateSynth está desactivado. **Con bypass activo**, la rama `/plate` no se ejecuta y la salida de audio no depende de `/plate`.

```
ISTR (oF)                          PAS (JUCE)
-----------                        -----------
checkCollisions()                  oscReceiver (port 9000)
checkParticleCollisions()               |
  -> pending_hits                   oscMessageReceived() [message thread]
  -> processPendingHits()               |
     rate limit (tokens,               /hit -> mapOSCHitToEvent()
      max_per_frame)                    |    -> triggerVoiceFromOSC()
     -> validated_hits                  |         -> hitsReceived++
  -> sendHitEvent() for each            |         -> eventFifo.write(HitEvent)
       /hit id x y energy surface      |              or hitsDiscarded++
sendStateMessage() /state               /state -> updateOSCState() -> globalPresence
sendPlateMessage() /plate (20 Hz)       /plate -> mapOSCPlateToEvent() -> triggerPlateFromOSC()
                                       |        (si bypass desactivado; si bypass activo: /plate ignorado)
getNextAudioBlock() [audio thread]
  -> renderNextBlock()
       -> processEventQueue()  [max 32 or 16 per block]
            -> voiceManager.triggerVoice() x N  -> hitsTriggered++
       -> voiceManager.renderNextBlock()
       -> plateSynth.renderNextBlock() + mix   (si bypass desactivado; si bypass activo: plate no se mezcla)
       -> clipper -> output
```

---

## G) Open Questions

- **Dónde poner masking/thinning:** Hoy el “thinning” está en ISTR (rate limiter + cooldown); PAS no hace masking ni fusión perceptual. Si se quiere que “a más partículas no suene a caos”, las opciones son: (1) ISTR: reducir más max_hits_per_second / max_hits_per_frame o priorizar por energy; (2) PAS: antes de escribir en la cola, submuestrear o fusionar por ventana de tiempo/energía. La implementación actual es solo ISTR.
- **Dónde se pierden eventos y por qué “menos sonidos que colisiones”:** Resumen: ISTR descarta por cooldown y por rate limit; PAS descarta por cola llena y procesa como máximo 32 (o 16) por bloque; además solo maxVoices sonidos a la vez (voice stealing). Para evaluar: comparar hits_per_second (ISTR enviados) con hitsReceived en PAS, y hitsTriggered vs hitsReceived (coverage); si coverage < 90% con muchas partículas, el cuello está en cola o en MAX_HITS_PER_BLOCK/maxVoices.

---

## Resumen de constantes clave (para el revisor)

| Lugar | Constante | Valor | Efecto |
|-------|-----------|-------|--------|
| SynthesisEngine | EVENT_QUEUE_SIZE | 128 | Cola de HitEvent; si llena, descarte |
| SynthesisEngine | MAX_HITS_PER_BLOCK | 32 | Máx. eventos leídos por bloque de audio |
| SynthesisEngine | (backlog) | available > 64 | Entonces numToRead = 16 |
| VoiceManager | maxVoices | 4–12 | Voces activas; voice stealing si todas ocupadas |
| ISTR ofApp | max_hits_per_second | 800 | Token bucket rate |
| ISTR ofApp | max_hits_per_frame | 50 | Máx. hits enviados por frame |
| ISTR ofApp | hit_cooldown_ms | 40 | Cooldown por partícula entre hits |

Con esto el revisor puede auditar RT-safety (cola lock-free, atómicos, sin alloc en audio thread), correctitud del mapeo (MainComponent + ModalVoice/PlateSynth) y consistencia visual/auditiva (puntos de descarte ISTR y PAS).
