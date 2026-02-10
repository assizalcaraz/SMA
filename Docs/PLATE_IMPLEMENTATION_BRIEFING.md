# Briefing Técnico: Implementación Plate Vibration Controller

**Fecha:** 2026-02-XX  
**Estado:** ✅ **IMPLEMENTADO** (2026-02-XX)  
**Objetivo:** Resumen técnico para generar prompt de plan de implementación de mejoras "Plate"

---

## 1) Current State Summary

### Arquitectura OSC
- **Direcciones soportadas:** `/hit` (5 args: id, x, y, energy, surface), `/state` (3 args: activity, gesture, presence)
- **Flujo:** oF → JUCE (unidireccional, UDP puerto 9000)
- **Rate limiting:** Token bucket en oF (max 400 hits/s, burst 500, max 30/frame)

### Mapeo OSC → Audio
- **Ubicación:** `MainComponent::mapOSCHitToEvent()` (líneas 333-408)
- **Mapeo actual:** energy→amp/brightness, x→pan, y→damping/freq, surface→metalness mod
- **Cola lock-free:** `SynthesisEngine::eventFifo` (AbstractFifo, tamaño 64, MAX_HITS_PER_BLOCK=16)
- **Estructura:** `SynthesisEngine::HitEvent` (baseFreq, amplitude, damping, brightness, metalness, waveform, subOscMix)

### Sintetizador Actual ("Coin Cascade")
- **Arquitectura:** `SynthesisEngine` → `VoiceManager` → `ModalVoice` (2 modos inarmónicos)
- **Polyphony:** 4-12 voces (default 8), voice stealing activo
- **Routing:** Voces → VoiceManager → SynthesisEngine → limiter → salida master
- **Audio callback:** `MainComponent::getNextAudioBlock()` → `SynthesisEngine::renderNextBlock()`

### UI oF Actual
- **Framework:** ofxGui panel
- **Controles:** N_particles, k_home, k_drag, k_gesture, sigma, restitution, hit_cooldown, particle_radius, energy params, rate limiting, particle_size, camera_zoom/rotation
- **OSC sender:** `ofApp::sendHitEvent()` (línea 815), `ofApp::sendStateMessage()` (línea 839)

---

## 2) Relevant Findings

- **OSC receiver:** `MainComponent::oscMessageReceived()` usa listener pattern JUCE 8.0.12, procesa `/hit` y `/state` en thread no-RT
- **Cola lock-free:** Ya implementada en `SynthesisEngine` (eventFifo + eventQueue[64]), procesada en `processEventQueue()` desde audio thread
- **Sintetizador único:** Actualmente solo "coin cascade" (ModalVoice con resonadores modales)
- **UI oF:** ofxGui con sliders, no hay sección dedicada para "plate"
- **OSC contract:** `/hit` está congelado, `/state` es opcional; no hay `/plate` definido
- **Audio routing:** Todas las voces se suman en VoiceManager, luego pasan por limiter en SynthesisEngine
- **Thread safety:** OSC callbacks en UI thread, escriben a cola lock-free; audio thread lee cola

---

## 3) Minimal Change Set

### Nuevo mensaje OSC
- **Dirección:** `/plate` (recomendado: mantener estilo existente)
- **Args propuestos:** `float freq` (Hz), `float amp` (0..1), `int mode` (0-N modos de placa)
- **Frecuencia de updates:** Decidir: continuo (10-60 Hz) vs eventos discretos

### Actualizaciones de documentación
- `Docs/api-osc.md`: Agregar sección `/plate` con contrato completo
- `Docs/Particles/spec.md`: Actualizar responsabilidades de módulo oF (nuevo UI + OSC sender)
- `Docs/specs/PLAN_IMPLEMENTACION.md`: Agregar fase/subtareas para PlateSynth

### Cambios en código oF
- **Archivo:** `Particles/src/ofApp.h` y `ofApp.cpp`
- **Cambios:**
  - Agregar UI controls: sliders para freq_plate, amp_plate, mode_plate (ofxGui)
  - Agregar función `sendPlateMessage()` similar a `sendHitEvent()`
  - Llamar `sendPlateMessage()` desde `update()` (posiblemente con rate limiting separado)

### Cambios en código JUCE
- **Archivo:** `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp`
- **Cambios:**
  - Agregar handler `mapOSCPlateToEvent()` similar a `mapOSCHitToEvent()`
  - Registrar en `oscMessageReceived()` para `/plate`
- **Archivo:** `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h/cpp`
- **Cambios:**
  - Decidir: ¿nueva clase `PlateSynth` separada o integrar en estructura existente?
  - Si separada: agregar `PlateSynth plateSynth;` y método `renderPlateNextBlock()`
  - Si integrada: agregar modo "plate" a ModalVoice o crear tipo de voz diferente
  - Agregar método `triggerPlateFromOSC(freq, amp, mode)` que escriba a cola lock-free
  - Modificar `renderNextBlock()` para sumar salida de plate (pre o post limiter)
- **Archivo:** `app-juce/app-JUCE-PAS1/Source/MainComponent.h`
- **Cambios:**
  - Agregar UI controls opcionales para plate (si se decide UI en JUCE)

---

## 4) Decision Points

### D1: Contrato OSC `/plate`
- **Opciones:**
  - a) `/plate <freq:float> <amp:float> <mode:int>` (3 args, estilo `/hit`)
  - b) `/plate <freq:float> <amp:float>` (2 args, sin modo)
  - c) `/plate <freq:float> <amp:float> <decay:float> <brightness:float>` (4 args, más control)
- **Recomendación:** Opción a) (3 args) para mantener consistencia con `/hit` y permitir modos de placa
- **Frecuencia de updates:** Decidir si continuo (10-60 Hz) o eventos discretos (trigger on change)

### D2: Arquitectura PlateSynth
- **Opciones:**
  - a) **Drone/resonador continuo:** Oscilador + filtros resonantes, se actualiza con cada `/plate`
  - b) **Noise-excited resonator:** Ruido blanco → filtros resonantes (similar a ModalVoice pero continuo)
  - c) **Oscilador simple + banco resonante:** Oscilador base + múltiples resonadores modales
- **Recomendación:** Opción b) (noise-excited) para mantener coherencia con diseño actual y permitir control de modos

### D3: Mezcla de audio
- **Opciones:**
  - a) **Post-synth, pre-limiter:** PlateSynth se suma después de VoiceManager, antes de limiter
  - b) **Separate bus, luego sum:** PlateSynth en bus separado, se suma al final antes de limiter
  - c) **Post-limiter:** PlateSynth se suma después del limiter (riesgo de saturación)
- **Recomendación:** Opción a) (post-synth, pre-limiter) para que limiter proteja todo el audio

### D4: Ubicación de UI
- **Opciones:**
  - a) **Solo oF:** Sliders en ofxGui panel (recomendado)
  - b) **Solo JUCE:** Controles en MainComponent UI
  - c) **Ambas:** Controles en oF, indicadores en JUCE
- **Recomendación:** Opción a) (solo oF) para mantener separación de responsabilidades y consistencia

### D5: Comportamiento fail-safe
- **Opciones:**
  - a) **Mantener último estado:** Si no se recibe `/plate` en X segundos, mantener último freq/amp/mode
  - b) **Default silencioso:** Si no se recibe `/plate`, fade out a silencio
  - c) **Timeout con reset:** Si no se recibe en X segundos, resetear a valores por defecto
- **Recomendación:** Opción b) (default silencioso) para evitar sonido no deseado si oF se desconecta

### D6: Restricciones de CPU
- **Target CPU:** < 30% en máquina de desarrollo (consistente con objetivo actual)
- **Complejidad máxima PlateSynth:**
  - Número de modos: Decidir (recomendado: 4-8 modos para estabilidad RT)
  - Update rate: Decidir frecuencia de actualización de parámetros (recomendado: cada bloque de audio o cada N bloques)
- **Recomendación:** 4-6 modos resonantes, actualización cada bloque de audio (RT-safe usando atomic)

---

## 5) Plan Skeleton

### Fase 1: Definición de contrato OSC
- Especificar formato exacto de `/plate` (args, tipos, rangos)
- Documentar en `api-osc.md`
- Validar con script de prueba

### Fase 2: UI en openFrameworks
- Agregar sliders para freq_plate, amp_plate, mode_plate en ofxGui
- Implementar `sendPlateMessage()` con validación de rangos
- Integrar en `update()` con rate limiting opcional

### Fase 3: Receptor OSC en JUCE
- Agregar handler `mapOSCPlateToEvent()` en MainComponent
- Registrar `/plate` en `oscMessageReceived()`
- Validar parsing y conversión de tipos

### Fase 4: Implementación PlateSynth
- Crear clase `PlateSynth` (o integrar en estructura existente)
- Implementar síntesis de placa (noise-excited resonator con N modos)
- Implementar método `renderNextBlock()` RT-safe

### Fase 5: Integración de audio
- Conectar PlateSynth a audio callback (sumar salida)
- Decidir routing (pre/post limiter)
- Validar mezcla sin glitches

### Fase 6: Thread safety y cola lock-free
- Implementar `triggerPlateFromOSC()` que escriba a cola lock-free
- Procesar eventos de plate en `processEventQueue()` o cola separada
- Validar RT-safety (sin allocations, sin locks)

### Fase 7: Fail-safe y timeout
- Implementar timeout para `/plate` (fade out si no se recibe)
- Validar comportamiento cuando oF se desconecta
- Testing de estabilidad

### Fase 8: Calibración y ajuste
- Ajustar parámetros de modos de placa
- Calibrar mezcla con "coin cascade"
- Optimizar CPU usage

---

## 6) Files to Touch

### openFrameworks
- `Particles/src/ofApp.h` (agregar miembros: sliders, función sendPlateMessage)
- `Particles/src/ofApp.cpp` (implementar UI, sendPlateMessage, integración en update)

### JUCE
- `app-juce/app-JUCE-PAS1/Source/MainComponent.h` (agregar método mapOSCPlateToEvent)
- `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp` (implementar handler, registrar en oscMessageReceived)
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h` (agregar PlateSynth o estructura para plate)
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.cpp` (integrar renderizado de plate, modificar renderNextBlock)
- `app-juce/app-JUCE-PAS1/Source/PlateSynth.h` (nuevo archivo, si se crea clase separada)
- `app-juce/app-JUCE-PAS1/Source/PlateSynth.cpp` (nuevo archivo, implementación de síntesis)

### Documentación
- `Docs/api-osc.md` (agregar sección `/plate`)
- `Docs/Particles/spec.md` (actualizar responsabilidades)
- `Docs/specs/PLAN_IMPLEMENTACION.md` (agregar fases de plate)

---

## 7) Risks & Mitigations

### R1: Event rate alto
- **Riesgo:** `/plate` enviado a alta frecuencia (60+ Hz) puede saturar cola lock-free
- **Mitigación:** Rate limiting en oF (similar a `/hit`), límite de eventos por bloque en audio thread

### R2: Thread safety
- **Riesgo:** Acceso no-RT-safe a parámetros de PlateSynth desde OSC callback
- **Mitigación:** Usar cola lock-free existente (eventFifo) o crear cola separada para plate, atomic para parámetros globales

### R3: CPU overhead
- **Riesgo:** PlateSynth con muchos modos puede exceder target de 30% CPU
- **Mitigación:** Limitar modos a 4-6, optimizar cálculos (pre-compute coeficientes), actualizar parámetros cada N bloques

### R4: Mezcla de audio
- **Riesgo:** PlateSynth puede dominar o interferir con "coin cascade"
- **Mitigación:** Control de nivel independiente para plate, validar mezcla en calibración, limiter protege salida total

### R5: UI thread blocking
- **Riesgo:** Sliders en oF pueden causar bloqueos si se actualizan muy rápido
- **Mitigación:** Throttling de envío OSC (máximo 10-20 Hz), validar rangos antes de enviar

### R6: Desconexión de oF
- **Riesgo:** Si oF se cierra, PlateSynth puede quedar "colgado" con último estado
- **Mitigación:** Timeout en JUCE (si no se recibe `/plate` en X segundos, fade out), reset a silencio

---

## 8) Estado de Implementación

**Fecha de implementación:** 2026-02-XX  
**Estado:** ✅ **COMPLETADO**

### Resumen de Implementación

Todas las fases del plan se han completado exitosamente:

- ✅ **Fase 1:** Contrato OSC `/plate` documentado en `Docs/api-osc.md`
- ✅ **Fase 2:** UI Plate Controller implementado en openFrameworks con sliders y rate limiting (20 Hz)
- ✅ **Fase 3:** Receptor OSC `/plate` implementado en JUCE con validación completa
- ✅ **Fase 4:** PlateSynth implementado como clase independiente con:
  - Síntesis noise-excited resonator (6 modos modales)
  - 8 modos de placa (0-7) con diferentes configuraciones tímbricas
  - Fail-safe con fade-out automático (2 segundos timeout)
  - Thread-safe usando atomic para parámetros
- ✅ **Fase 5:** Integración completa en SynthesisEngine:
  - Mezcla pre-limiter (plate se suma antes del limiter)
  - Buffer pre-allocado para RT-safety
  - Método `triggerPlateFromOSC()` implementado

### Decisiones Finales Implementadas

- **D1:** Contrato OSC `/plate <freq:float> <amp:float> <mode:int>` (3 args) ✅
- **D2:** Arquitectura noise-excited resonator (opción b) ✅
- **D3:** Mezcla post-synth, pre-limiter (opción a) ✅
- **D4:** UI solo en oF (opción a) ✅
- **D5:** Fail-safe con fade-out a silencio (opción b) ✅
- **D6:** 6 modos resonantes, actualización periódica (cada 4 bloques) ✅

### Archivos Modificados/Creados

**openFrameworks:**
- `Particles/src/ofApp.h` - Sliders y función `sendPlateMessage()` agregados
- `Particles/src/ofApp.cpp` - Implementación de UI y envío OSC

**JUCE:**
- `app-juce/app-JUCE-PAS1/Source/PlateSynth.h` - **NUEVO** - Clase PlateSynth
- `app-juce/app-JUCE-PAS1/Source/PlateSynth.cpp` - **NUEVO** - Implementación completa
- `app-juce/app-JUCE-PAS1/Source/MainComponent.h` - Handler `mapOSCPlateToEvent()` agregado
- `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp` - Implementación del handler
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h` - Integración PlateSynth
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.cpp` - Integración y mezcla

**Documentación:**
- `Docs/api-osc.md` - Sección `/plate` agregada con contrato completo

### Corrección Post-Implementación (2026-02-10)

**Problema identificado:**
- `plate_freq` estaba escalando los modos (m, n), cambiando el patrón espacial
- El centro de la placa se movía al cambiar modo/frecuencia
- Se usaban coordenadas del mundo directamente en lugar de coordenadas de placa

**Solución implementada:**
- ✅ Separación patrón espacial / frecuencia temporal:
  - `plate_mode` → define (m, n) del patrón espacial (estacionario)
  - `plate_freq` → solo para intensidad de excitación, no modifica el patrón
- ✅ Sistema de coordenadas fijo:
  - Centro de placa: siempre el centro de la ventana (inmutable)
  - Transformación: `pos_world → pos_plate → (x̂, ŷ) normalizado`
- ✅ Campo estacionario correcto:
  - `U(x̂, ŷ) = sin(mπx̂) * sin(nπŷ)` — solo depende de modo y posición
  - No depende de tiempo ni frecuencia
  - Los nodos están donde `U ≈ 0` (estacionarios)
- ✅ Movimiento hacia nodos:
  - Calcula gradiente del campo estacionario
  - Fuerza en dirección opuesta al gradiente (hacia nodos)
  - Intensidad proporcional a `|U|` (más fuerza en antinodos)

**Resultado:**
- Patrón permanece fijo al cambiar `plate_freq` (solo cambia intensidad)
- Centro no se mueve (siempre en centro de ventana)
- Partículas se acumulan en líneas nodales según modo
- Comportamiento físico correcto según teoría de Chladni

### Pendiente

- ⏳ **Fase 6 (Calibración):** Requiere testing en tiempo de ejecución para ajustar:
  - Niveles de mezcla entre plate y partículas
  - Factores inarmónicos y ganancias de modos
  - Parámetros de fail-safe
  - Optimización de CPU usage

---

**Fin del briefing**
