# Esquema OSC ISTR → PAS

Descripción concisa de direcciones, tipos y orden de argumentos, unidades/rangos, y dónde se producen (ISTR) y consumen (PAS).

---

## Puerto

- **PAS:** escucha UDP en el puerto **9000**.
- **ISTR:** envía a **127.0.0.1:9000** (configurable: `oscHost`, `oscPort` en ofApp).

---

## Direcciones y argumentos

### `/hit`

| Índice | Tipo    | Nombre  | Unidades / rango | Producción (ISTR) | Consumo (PAS) |
|--------|---------|---------|-------------------|-------------------|---------------|
| 0      | int32   | id      | ID de partícula   | `HitEvent.id`     | Validado; no usado en mapeo (reservado). |
| 1      | float32 | x       | 0..1 (normalizado) | pos.x / width     | Clamp 0..1; damping no usa x; pan = (x*2)-1 no aplicado a salida. |
| 2      | float32 | y       | 0..1 (normalizado) | pos.y / height   | Clamp 0..1; damping = 0.2 + 0.6*(1-y). |
| 3      | float32 | energy  | 0..1              | calculateHitEnergy / calculateParticleCollisionEnergy | Clamp 0..1; amplitude = energy^1.5; brightness = 0.3+0.7*energy; waveform por umbrales. |
| 4      | int32   | surface | 0=L, 1=R, 2=T, 3=B, -1=p-p | Bordes o -1 para colisión partícula-partícula | Validado; PAS no modifica timbre por surface (metalness global). |

- **Producción (ISTR):** `ofApp::sendHitEvent(const HitEvent& event)` — un mensaje por evento en `validated_hits` (después de rate limiting y cooldown).
- **Consumo (PAS):** `MainComponent::mapOSCHitToEvent(const juce::OSCMessage& message)`. Validación: `message.size() == 5` y tipos correctos; si no, se descarta. Clamps en x, y, energy. Mapeo a baseFreq (300 ± pitchRange*200*random, clamp 100–800 Hz), amplitude, damping, brightness, metalness (global), waveform (por energy), subOscMix (global) → `synthesisEngine.triggerVoiceFromOSC(...)`.

---

### `/state`

| Índice | Tipo    | Nombre    | Unidades / rango | Producción (ISTR) | Consumo (PAS) |
|--------|---------|-----------|-------------------|-------------------|---------------|
| 0      | float32 | activity  | 0..1              | hits_per_second / max_hits_per_second | No usado. |
| 1      | float32 | gesture   | 0..1              | Velocidad mouse normalizada, etc.   | No usado. |
| 2      | float32 | presence  | 0..1              | calculatePresence()                 | Guardado en `globalPresence`; no aplicado a ganancia master. |

- **Producción (ISTR):** `ofApp::sendStateMessage()` — enviado a ~10 Hz cuando hay actividad (activity >= umbral); rate limit por `stateSendInterval`.
- **Consumo (PAS):** `MainComponent::updateOSCState(const juce::OSCMessage& message)`. Validación: size==3, float32; guarda `globalPresence` (atomic). No modifica nivel de salida.

---

### `/plate`

| Índice | Tipo    | Nombre | Unidades / rango | Producción (ISTR) | Consumo (PAS) |
|--------|---------|--------|-------------------|-------------------|---------------|
| 0      | float32 | freq   | 20–2000 Hz        | plateFreq (slider) | Clamp 20–2000; PlateSynth::triggerPlate. |
| 1      | float32 | amp    | 0..1              | plateAmp (slider)  | Clamp 0..1; PlateSynth::triggerPlate. |
| 2      | int32   | mode   | 0–7               | plateMode (slider)  | Clamp 0..7; PlateSynth::triggerPlate. |

- **Producción (ISTR):** `ofApp::sendPlateMessage()` — rate limit 20 Hz (`plateSendInterval = 0.05` s). Valores clamp antes de enviar.
- **Consumo (PAS):** `MainComponent::mapOSCPlateToEvent(const juce::OSCMessage& message)`. Validación: size==3, tipos float32, float32, int32; clamp freq 20–2000, amp 0..1, mode 0..7 → `synthesisEngine.triggerPlateFromOSC(freq, amp, mode)`.

**Nota (comportamiento v1 / bypass por defecto):** En esta versión, **PAS ignora `/plate` por defecto** (PlateSynth deshabilitado: `enablePlateSynth = false`). No se llama a `triggerPlateFromOSC` para mensajes `/plate`, por lo que la salida de audio no depende de `/plate`. ISTR puede seguir enviando `/plate` para futura reactivación del módulo. El esquema anterior es válido cuando el bypass esté desactivado (`enablePlateSynth = true`).

---

## Unidades y rangos (resumen)

| Dirección | Argumentos | Rangos / notas |
|-----------|------------|----------------|
| `/hit`    | id, x, y, energy, surface | x,y 0..1; energy 0..1; surface 0,1,2,3,-1. |
| `/state`  | activity, gesture, presence | Los tres 0..1. |
| `/plate`  | freq, amp, mode | freq 20–2000 Hz; amp 0..1; mode 0..7. |

---

## Procesamiento y RT-safety (PAS)

- **Clipper:** salida master con soft clip por sample (umbral ~0.95) antes de DAC; sin envelope follower.
- **Envelope percusivo (ModalVoice):** ADSR con transición Decay → Idle (sin sustain sostenido); decay depende de damping.
- **Parámetros globales:** metalness, brightness, damping se aplican vía atómicos y `updateGlobalParameters` en voces activas (periodic update en audio thread).
- **Contadores de hits:** hitsReceived, hitsTriggered, hitsDiscarded con `std::atomic` (memory_order_relaxed).

---

## Nota

No hay otras direcciones OSC en uso. Los mensajes con dirección desconocida se ignoran en `MainComponent::oscMessageReceived` (sin crash ni log).
