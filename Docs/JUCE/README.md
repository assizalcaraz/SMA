# JUCE — Descripción General y Arquitectura

Descripción general y arquitectura del módulo de síntesis en JUCE. Visión de alto nivel del sintetizador "Coin Cascade".

**Última actualización:** v0.1 - Documentación inicial

**Ver también:**
- [`README.md`](README.md) - Índice del módulo
- [`manual.md`](manual.md) - Manual de usuario (parámetros, uso)
- [`spec.md`](spec.md) - Especificación técnica para desarrolladores

---

## Visión General

El módulo JUCE (App B) implementa un sintetizador modal en tiempo real que genera sonido a partir de eventos OSC recibidos desde el módulo Particles (App A). El objetivo sonoro es crear una textura metálica característica llamada "Coin Cascade" (cascada de monedas), donde múltiples hits percusivos breves se acumulan formando una cascada granular.

### Características Principales

- **Síntesis modal**: Resonadores modales con modos inarmónicos para timbre metálico
- **Polyphony**: Sistema de múltiples voces (4-12) con voice stealing inteligente
- **Receptor OSC**: Escucha mensajes `/hit`, `/state` y `/plate` en puerto 9000
- **PlateSynth**: Sintetizador de placa metálica paralelo con 8 modos configurables
- **RT-safe**: Diseño optimizado para tiempo real sin allocations en audio thread
- **UI completa**: Controles en tiempo real y indicadores de estado

---

## Arquitectura de Componentes

### MainComponent

**Responsabilidad**: Orquestación principal, UI y comunicación OSC.

**Componentes**:
- **UI**: Sliders, comboboxes, botones y labels para control y visualización
- **OSC Receiver**: Escucha en puerto 9000, procesa mensajes `/hit`, `/state`, `/plate`
- **Audio Callbacks**: `prepareToPlay()`, `getNextAudioBlock()`, `releaseResources()`
- **Mapeo OSC**: Convierte parámetros OSC a parámetros de síntesis

**Flujo**:
```
OSC Message → oscMessageReceived() → mapOSCHitToEvent() → SynthesisEngine
```

### SynthesisEngine

**Responsabilidad**: Motor principal de síntesis, orquestación de voces y procesamiento master.

**Componentes**:
- **VoiceManager**: Gestión de polyphony y voice stealing
- **PlateSynth**: Sintetizador de placa metálica paralelo
- **Cola lock-free**: `AbstractFifo` para eventos OSC (tamaño 128)
- **Limiter**: Master limiter para prevenir saturación
- **Mezcla**: Combina audio de voces y PlateSynth antes del limiter

**Flujo**:
```
Eventos OSC → Cola lock-free → processEventQueue() → VoiceManager → Audio
```

### ModalVoice

**Responsabilidad**: Resonador modal individual con múltiples modos inarmónicos.

**Componentes**:
- **6 modos resonantes**: Filtros biquad bandpass resonantes
- **Factores inarmónicos**: [1.0, 2.76, 5.40, 8.93, 13.34, 18.65] para timbre metálico
- **Excitación**: Noise burst (~5ms, 128 samples) o formas de onda configurables
- **Envolvente**: Decaimiento controlado por parámetro damping
- **Sub-oscillator**: Oscilador adicional para timbre más rico
- **Formant filter**: Filtro de formante para carácter tímbrico adicional

**Características**:
- Cada modo tiene frecuencia, Q y ganancia independientes
- Variación aleatoria sutil en frecuencias (±2%) para timbre más rico
- Múltiples formas de onda de excitación (Noise, Sine, Square, Saw, Triangle, Click, Pulse)

### VoiceManager

**Responsabilidad**: Gestión de polyphony y voice stealing.

**Componentes**:
- **Pool de voces**: Pre-allocation de hasta 32 voces (solo 4-12 activas según configuración)
- **Voice stealing**: Roba voz con menor amplitud residual o más antigua
- **RT-safe**: Sin allocations en audio thread, todas las voces pre-allocadas

**Algoritmo de voice stealing**:
1. Buscar voz disponible
2. Si todas están activas, encontrar voz con menor amplitud residual
3. Si hay empate, robar la más antigua
4. Configurar nueva voz y activar

### PlateSynth

**Responsabilidad**: Sintetizador de placa metálica paralelo con excitación continua.

**Componentes**:
- **6 modos modales resonantes**: Filtros biquad bandpass
- **8 modos de placa (0-7)**: Diferentes configuraciones de factores inarmónicos y ganancias
- **Excitación**: Ruido blanco continuo filtrado por amplitud
- **Fail-safe**: Fade-out automático después de 2 segundos sin updates
- **Thread-safe**: Parámetros usando `std::atomic`, buffer pre-allocado

**Características**:
- Independiente del sistema de partículas (`/hit`)
- Se mezcla con audio de partículas antes del limiter
- Control de volumen con mapeo exponencial (slider 0.1 → volumen 0.5)

---

## Flujo de Señal

### Flujo Principal: OSC → Audio

```
┌─────────────────────────────────────────────────────────────┐
│                    App A (Particles)                        │
│  Partículas → Colisiones → Eventos → OSC Sender            │
└──────────────────────┬──────────────────────────────────────┘
                       │ UDP OSC (puerto 9000)
                       │ Mensajes: /hit, /state, /plate
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              MainComponent (OSC Receiver)                   │
│  oscMessageReceived() → mapOSCHitToEvent()                 │
└──────────────────────┬──────────────────────────────────────┘
                       │ Eventos mapeados
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              SynthesisEngine (Cola lock-free)               │
│  triggerVoiceFromOSC() → eventFifo.write()                │
└──────────────────────┬──────────────────────────────────────┘
                       │ Audio Thread
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              SynthesisEngine::processEventQueue()          │
│  eventFifo.read() → VoiceManager::triggerVoice()           │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
┌───────────────┐            ┌─────────────────┐
│ VoiceManager │            │   PlateSynth    │
│ (Polyphony)  │            │ (Placa Metálica)│
└───────┬───────┘            └────────┬────────┘
        │                              │
        ▼                              │
┌───────────────┐                      │
│ ModalVoice    │                      │
│ (Resonadores) │                      │
└───────┬───────┘                      │
        │                              │
        └──────────────┬───────────────┘
                       │ Mezcla
                       ▼
              ┌─────────────────┐
              │   Limiter        │
              │ (Master)        │
              └────────┬─────────┘
                       │
                       ▼
              ┌─────────────────┐
              │   Audio Output  │
              └─────────────────┘
```

### Flujo de Mensaje `/hit`

1. **Recepción OSC**: `MainComponent::oscMessageReceived()` recibe mensaje `/hit`
2. **Validación**: Verifica que el mensaje tenga 5 parámetros (id, x, y, energy, surface)
3. **Mapeo**: `mapOSCHitToEvent()` convierte parámetros OSC a parámetros de síntesis:
   - `energy` → `amplitude` (con exponente γ = 1.5)
   - `energy` → `brightness` (lerp 0.3-1.0)
   - `y` → `damping` (lerp 0.2-0.8, invertido)
   - `y` → `baseFreq` (200-600 Hz)
   - `x` → `pan` (futuro, no implementado aún)
   - `surface` → modulación tímbrica (futuro)
4. **Escritura a cola**: `SynthesisEngine::triggerVoiceFromOSC()` escribe evento a cola lock-free
5. **Procesamiento en audio thread**: `processEventQueue()` lee eventos y activa voces

### Flujo de Mensaje `/plate`

1. **Recepción OSC**: `MainComponent::oscMessageReceived()` recibe mensaje `/plate`
2. **Validación**: Verifica que el mensaje tenga 3 parámetros (freq, amp, mode)
3. **Mapeo**: `mapOSCPlateToEvent()` valida rangos y actualiza PlateSynth
4. **Actualización atómica**: `PlateSynth::triggerPlate()` actualiza parámetros usando `std::atomic`
5. **Renderizado continuo**: `PlateSynth::renderNextBlock()` genera audio en cada bloque

---

## Mapeo OSC → Parámetros de Síntesis

### Mensaje `/hit`

| Parámetro OSC | Mapeo en JUCE | Fórmula |
|---------------|---------------|---------|
| `energy` (0-1) | `amplitude` | `amp = energy^1.5` |
| `energy` (0-1) | `brightness` | `brightness = lerp(0.3, 1.0, energy)` |
| `y` (0-1) | `damping` | `damping = lerp(0.2, 0.8, 1 - y)` |
| `y` (0-1) | `baseFreq` | `freq = 200 + (y * 400)` Hz |
| `x` (0-1) | `pan` | `pan = (x * 2) - 1` (futuro, no implementado) |
| `surface` (0-3, -1) | Modulación tímbrica | (futuro, no implementado) |

**Nota**: El mapeo está diseñado para crear textura "Coin Cascade" donde:
- Colisiones más energéticas → sonidos más brillantes y fuertes
- Posiciones superiores → sonidos más secos (menos damping)
- Posiciones inferiores → sonidos más reverberantes (más damping)

### Mensaje `/state`

| Parámetro OSC | Mapeo en JUCE | Estado Actual |
|---------------|---------------|---------------|
| `activity` (0-1) | Global wet (reverb) | No implementado aún |
| `gesture` (0-1) | Global drive/saturación | No implementado aún |
| `presence` (0-1) | Master level | Parcialmente implementado |

**Nota**: El mapeo de `/state` está parcialmente implementado. Algunos parámetros globales aún no están conectados.

### Mensaje `/plate`

| Parámetro OSC | Mapeo en JUCE | Fórmula |
|---------------|---------------|---------|
| `freq` (20-2000 Hz) | `PlateSynth::currentFreq` | Clamp a rango válido |
| `amp` (0-1) | `PlateSynth::currentAmp` | Clamp a 0-1 |
| `mode` (0-7) | `PlateSynth::currentMode` | Clamp a 0-7 |

**Nota**: El mapeo es directo, solo validación y clamp de rangos. PlateSynth actualiza sus parámetros atómicamente.

---

## Relación con el Sistema Global

### App A (Particles) → App B (JUCE)

**Comunicación**: Unidireccional vía OSC (UDP puerto 9000)

**Mensajes**:
- `/hit`: Eventos de colisión de partículas (principal)
- `/state`: Estado global del sistema (opcional)
- `/plate`: Control de vibración de placa de Chladni (paralelo)

**Flujo completo**:
```
Particles (oF) → Colisiones → Eventos → OSC → JUCE → Audio
```

### App C (MAAD-2-CALIB)

**Relación**: App C escucha los mismos mensajes OSC para registro y análisis, pero no modifica el comportamiento de App B.

**Integración**: App C se conecta al mismo puerto OSC (9000) para capturar datos de sesiones de calibración.

---

## Diseño RT-Safe

### Principios

1. **Sin allocations en audio thread**: Todas las estructuras pre-allocadas
2. **Cola lock-free**: `AbstractFifo` para comunicación entre threads
3. **Parámetros atómicos**: `std::atomic` para parámetros globales
4. **Buffers pre-allocados**: Tamaño máximo conocido, sin reallocations

### Implementación

**Cola de eventos**:
- Tamaño fijo: 128 eventos
- `AbstractFifo` para sincronización lock-free
- `MAX_HITS_PER_BLOCK = 32`: Límite de eventos procesados por bloque

**Voces**:
- Pre-allocation: hasta 32 voces (solo se activan según `maxVoices`)
- Sin allocations en `triggerVoice()`
- Voice stealing sin allocations

**PlateSynth**:
- Buffer pre-allocado: `MAX_BLOCK_SIZE = 2048` samples
- Parámetros usando `std::atomic`
- Sin allocations en `renderNextBlock()`

---

## Características Técnicas

### Síntesis Modal

- **Modos por voz**: 6 modos resonantes
- **Factores inarmónicos**: Optimizados para timbre metálico
- **Filtros**: Biquad bandpass resonantes
- **Excitación**: Noise burst o formas de onda configurables

### Polyphony

- **Voces máximas**: 4-12 configurables (default: 8)
- **Pre-allocation**: Hasta 32 voces (solo se activan según configuración)
- **Voice stealing**: Inteligente (menor amplitud residual o más antigua)

### Rendimiento

- **CPU usage objetivo**: < 30% con 8 voces activas
- **Latencia objetivo**: < 10ms end-to-end (OSC → audio)
- **Estabilidad**: Sin glitches bajo carga normal (hasta 200 hits/segundo)

---

## Referencias

- **[Contrato OSC](../api-osc.md)** — Contrato completo de mensajes OSC
- **[Especificación técnica completa](../specs/spec.md)** — Arquitectura general del sistema
- **[Plan de implementación](../specs/PLAN_IMPLEMENTACION.md)** — Fases de desarrollo
- **[Estado actual](../overview/ESTADO_ACTUAL.md)** — Estado del proyecto

---

**Última actualización**: 2026-02-11
