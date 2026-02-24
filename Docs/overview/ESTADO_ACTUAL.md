# Estado Actual del Proyecto

**Última actualización**: 2026-02-XX (v0.3)

Este documento describe el estado actual de implementación del Sistema Modular Audiovisual, qué componentes están completados, en desarrollo o pendientes.

**Rama de entrega ISTR/PAS1:** Esta rama (`entrega-istr-pas1`) es la base para empaquetar y entregar los módulos **Particles (ISTR)** y **app-juce (PAS1)** sin referencias a CALIB. OSC: un solo destino en puerto 9000; no hay mensajes `/test/*` ni grabación.

---

## Resumen Ejecutivo

El proyecto está en desarrollo activo, con el módulo Particles (App A) parcialmente implementado. La estrategia de implementación prioriza el loop modular completo (partículas → colisiones → OSC → JUCE) antes de integrar MediaPipe.

---

## Componentes Implementados

### 🟢 Completado

#### Documentación y Planificación
- ✅ Arquitectura del sistema definida
- ✅ Especificación técnica completa (`Docs/specs/spec.md`)
- ✅ Contrato OSC establecido y documentado (pendiente de crear `api-osc.md`)
- ✅ Plan de implementación detallado (`Docs/specs/PLAN_IMPLEMENTACION.md`)
- ✅ Estructura de documentación organizada

#### App A - openFrameworks (Particles)

**Fase 2: Sistema de Partículas Básico**
- ✅ Sistema de partículas con física básica
  - Fuerza de retorno al origen (`F_home`)
  - Fuerza de drag (`F_drag`)
  - Integración semi-implícita Euler
- ✅ UI con sliders ajustables en tiempo real
  - Control de número de partículas (`N_particles`)
  - Control de `k_home` y `k_drag`
- ✅ Rendering de puntos con estilo metálico
- ✅ Debug overlay (FPS, número de partículas)
- ✅ Inicialización con grid + jitter

**Fase 3: Input Básico (Mouse)**
- ✅ Captura de posición y velocidad del mouse
- ✅ Suavizado de input
- ✅ Fuerza de gesto con influencia gaussiana
- ✅ Parámetros ajustables: `k_gesture`, `sigma`, `speed_ref`
- ✅ Sistema de partículas responde a input del mouse

**v0.3: Chladni State - Auto-organización** ✅ **COMPLETADA**
- ✅ **Chladni State**: Modo que permite auto-organización sin mouse
  - Toggle con tecla **SPACE**:** activa/desactiva el modo
  - Cuando activo: `k_home = 0.01f` (partículas libres de retorno al origen)
  - Cuando inactivo: comportamiento idéntico a v0.2
- ✅ **Plate Shaker**: Sistema de inyección de energía coherente
  - Fuerza proporcional a energía local `E = U_norm²` (más fuerte en antinodos)
  - Dirección coherente usando `ofSignedNoise()` (permite settling en nodos)
  - Energía normalizada y shaped para concentrar agitación
  - Solo activo cuando Chladni State está ON y `plate_amp >= 0.01`
- ✅ **Mejoras técnicas**:
  - Clamp de `dt` para consistencia FPS
  - Clamp relativo de fuerza shaker (`F_SHAKER_MAX = 0.5 * F_MAX`)
  - Preservación completa de comportamiento v0.2 cuando Chladni está OFF

---

### 🔧 En Desarrollo

#### App A - openFrameworks (Particles)

**Fase 4: Colisiones y Eventos** ✅ **COMPLETADA**
- ✅ Detección de colisiones con bordes
- ✅ **Detección de colisiones partícula-partícula** (nuevo)
- ✅ Sistema de rebote de partículas
- ✅ Generación de eventos de hit
- ✅ Sistema de cooldown por partícula
- ✅ Rate limiting global
- ✅ Formato final de `/hit` (contrato congelado)

**Fase 5: Comunicación OSC** ✅ **COMPLETADA**
- ✅ Integración de `ofxOsc`
- ✅ Envío de mensajes `/hit`
- ✅ Envío de mensajes `/state` (opcional)
- ✅ Configuración de host y puerto (127.0.0.1:9000)
- ✅ Validación de mensajes antes de envío
- ✅ **Confirmado funcionando**: app-juce recibe correctamente los hits y el sintetizador responde

#### App B - JUCE

**Fase 6: Sintetizador Básico** ✅ **COMPLETADA**
- ✅ Proyecto base JUCE Standalone (`app-juce/app-JUCE-PAS1/`)
- ✅ Estructura completa de sintetizador modal
- ✅ Motor de síntesis modal con resonadores inarmónicos
- ✅ Timbre "coin cascade" con 2 modos por voz
- ✅ Sistema de polyphony con voice stealing (4-12 voces)
- ✅ UI básica con controles y indicadores
- ✅ Botón "Test Trigger" para testing manual
- ✅ Optimizaciones RT-safe (cola lock-free, pre-allocation)
- ✅ Master limiter y saturación opcional

**Fase 7: Receptor OSC y Mapeo** ✅ **COMPLETADA**
- ✅ Receptor OSC en puerto 9000 (JUCE 8.0.12 listener pattern)
- ✅ Parsing de mensajes `/hit` y `/state`
- ✅ Mapeo de parámetros a síntesis
- ✅ Integración con cola lock-free existente
- ⚠️ **Problemas conocidos**: Sliders no afectan el sonido significativamente, timbre suena más a "pluc" de madera que metálico

**Fase 8: Sistema Plate Paralelo** ✅ **COMPLETADA** (2026-02-XX)
- ✅ Plate Controller en openFrameworks (UI con sliders: freq, amp, mode)
- ✅ Mensaje OSC `/plate` implementado y documentado
- ✅ Receptor OSC `/plate` en JUCE con validación completa
- ✅ PlateSynth implementado (clase independiente con 6 modos modales)
- ✅ 8 modos de placa (0-7) con diferentes configuraciones tímbricas
- ✅ Fail-safe con fade-out automático (2 segundos timeout)
- ✅ Integración en SynthesisEngine (mezcla pre-limiter)
- ✅ Thread-safe (atomic para parámetros, buffer pre-allocado)
- ✅ **Control de volumen en UI JUCE** con mapeo exponencial (slider 0.1 → volumen 0.5)
- ✅ **Corrección física de Chladni** (2026-02-10):
  - Separación patrón espacial (plate_mode) / frecuencia temporal (plate_freq)
  - Sistema de coordenadas fijo (centro inmutable en centro de ventana)
  - Campo estacionario U(x̂,ŷ) = sin(mπx̂)*sin(nπŷ) según teoría de Chladni
  - Partículas se acumulan en nodos (líneas nodales estacionarias)
- ✅ **Mejoras de simetría y estabilidad** (2026-02-10):
  - Mezcla de modos degenerados para restaurar simetría en modos altos
  - Fuerza basada en energía (E = U²) físicamente más correcta
  - Normalización por modo para evitar dominancia de modos altos
  - Centrado suave de excitación con peso gaussiano espacial
  - Amortiguación adicional cerca de nodos para adherir partículas
  - Clamp de magnitud de fuerza para mayor estabilidad
- ⏳ **Pendiente**: Calibración de niveles y optimización (requiere testing en tiempo de ejecución)

---

### 📋 Pendiente

#### App A - openFrameworks

**Fase 3b: Integración MediaPipe (Opcional/Tardía)**
- ⏳ Integración de MediaPipe para tracking
- ⏳ Detección de manos o pose corporal
- ⏳ Mapeo de tracking a input de gestos
- ⏳ Calibración y ajuste de parámetros

#### App B - JUCE

**Fase 8: Calibración y Ajuste Conjunto**
- ⏳ Sincronización de parámetros entre apps
- ⏳ Ajuste fino de mapeo OSC → audio
- ⏳ Optimización de rendimiento

**Fase 9: Documentación y Demo**
- ⏳ Documentación de código
- ⏳ Guías de usuario final
- ⏳ Demo y presentación

#### General

- ⏳ Tests de integración
- ⏳ Calibración y ajuste de parámetros
- ⏳ Documentación de código fuente
- ⏳ Optimización de rendimiento

---

## Estado por Módulo

### Módulo Particles (App A)

**Ubicación**: `Particles/src/`

**Archivos principales**:
- `main.cpp` — Punto de entrada ✅
- `ofApp.h/cpp` — Clase principal ✅
- `Particle.h/cpp` — Clase partícula ✅

**Funcionalidades**:
- ✅ Física básica de partículas
- ✅ Input mouse funcionando
- ✅ Colisiones con bordes y partícula-partícula
- ✅ Generación de eventos de hit
- ✅ Comunicación OSC funcionando
- ✅ **Plate Controller** - Control de vibración de placa de Chladni:
  - Fuerzas físicas basadas en modos propios de vibración
  - Partículas se acumulan en líneas nodales según modo
  - Patrón espacial estacionario (no depende de frecuencia)
  - Mezcla de modos degenerados para simetría en modos altos
  - Fuerza basada en energía (E = U²) con normalización por modo
  - Estabilidad mejorada con clamp de fuerza y amortiguación nodal

**Documentación**: Completa en `Docs/Particles/`

---

### Módulo JUCE (App B)

**Ubicación**: `app-juce/app-JUCE-PAS1/Source/`

**Estado**: ✅ Fase 6 completada - Sintetizador básico funcionando

**Archivos principales**:
- `Main.cpp` — Punto de entrada ✅
- `MainComponent.h/cpp` — UI y orquestación ✅
  - Controles: Voices, Pitch (metalness), Waveform, SubOsc Mix, **Plate Volume**, Limiter toggle
  - **Plate Volume**: Control de volumen con mapeo exponencial (0.1 → 0.5 en amplitud)
- `ModalVoice.h/cpp` — Resonador modal individual ✅
- `VoiceManager.h/cpp` — Gestión de polyphony ✅
- `SynthesisEngine.h/cpp` — Motor de síntesis principal ✅
- `PlateSynth.h/cpp` — **NUEVO** - Sintetizador de placa metálica ✅
- `SynthParameters.h` — Parámetros globales ✅

**Funcionalidades implementadas**:
- ✅ Sintetizador modal con resonadores inarmónicos (6 modos por voz en ModalVoice)
- ✅ Sistema de polyphony con voice stealing (4-12 voces configurables)
- ✅ Cola lock-free para eventos de partículas
- ✅ Receptor OSC para mensajes `/hit` y `/state`
- ✅ Mapeo de parámetros OSC a síntesis de partículas
- ✅ **PlateSynth** - Sintetizador de placa metálica paralelo:
  - 6 modos modales resonantes
  - 8 modos de placa (0-7) con diferentes timbres
  - Excitación por ruido blanco continuo
  - Fail-safe con fade-out automático
- ✅ Receptor OSC para mensajes `/plate`
- ✅ Mezcla pre-limiter (plate + partículas → limiter)
- ✅ UI básica con controles y indicadores
- ✅ **Control de volumen Plate** en UI JUCE con mapeo exponencial:
  - Slider 0.0 → Volumen 0.0 (silencio)
  - Slider 0.1 → Volumen 0.5 (mitad del rango de amplitud)
  - Slider 1.0 → Volumen 1.0 (volumen completo)
  - Mapeo exponencial en rango 0.0-0.1 para control preciso en volúmenes bajos
- ✅ Master limiter y saturación opcional
- ✅ Optimizaciones RT-safe (sin allocations en audio thread)

**Características técnicas**:
- **ModalVoice (partículas):**
  - Modos resonantes: 6 por voz (optimizado para RT)
  - Excitación: noise burst de ~5ms (128 samples)
  - Voces: 4-12 configurables (por defecto: 8)
  - Límite de eventos: 16 por bloque de audio (MAX_HITS_PER_BLOCK)
  - Pre-allocation: hasta 32 voces (solo se activan según maxVoices)
- **PlateSynth (placa):**
  - Modos resonantes: 6 modos modales
  - Excitación: ruido blanco continuo filtrado por amplitud
  - Modos de placa: 8 configuraciones (0-7) con diferentes timbres
  - Fail-safe: fade-out automático después de 2 segundos sin updates
  - Thread-safe: parámetros usando atomic, buffer pre-allocado

**Documentación**: Ver `PROMPT_FASE6.md` para detalles de implementación

---

## Próximos Pasos

### Corto Plazo (Próximas 2-4 semanas)

1. **Calibración del Sistema Plate** (Fase 6 pendiente)
   - Ajustar niveles de mezcla entre plate y partículas
   - Calibrar factores inarmónicos y ganancias de modos de placa
   - Optimizar parámetros de fail-safe
   - Profilar y optimizar CPU usage

### Mediano Plazo (1-2 meses)

1. **Completar loop modular completo**
   - Partículas → Colisiones → OSC → JUCE → Audio
   - Validar funcionamiento end-to-end

2. **Calibración y ajuste**
   - Sincronizar parámetros
   - Optimizar rendimiento

### Largo Plazo (Opcional)

1. **Integración MediaPipe**
   - Solo si el sistema base funciona correctamente
   - Tratado como "swap de input"

---

## Estrategia de Implementación

**Prioridad**: Loop modular completo antes de MediaPipe

1. ✅ Sistema de partículas básico
2. ✅ Input mouse (validación rápida)
3. ✅ Colisiones y eventos
4. ✅ Comunicación OSC
5. ✅ Sintetizador JUCE básico
6. ✅ Receptor OSC y mapeo (partículas)
7. ✅ Sistema Plate paralelo (Plate Controller + PlateSynth)
8. ⏳ Calibración conjunta (plate + partículas)
9. ⏳ MediaPipe (opcional/tardía)

---

## Issues Conocidos

### App B - JUCE (Sintetizador)

**Problema 1: Sliders no afectan el sonido significativamente**
- **Estado**: Reportado 2025-02-09
- **Descripción**: Los controles de la UI (metalness, brightness, damping, etc.) no parecen tener efecto audible notable en el timbre
- **Impacto**: Limitación en la expresividad del sintetizador
- **Prioridad**: Media-Alta (afecta usabilidad)

**Problema 2: Timbre no suena metálico**
- **Estado**: Reportado 2025-02-09
- **Descripción**: El sonido actual suena más a "pluc" de madera que a timbre metálico característico
- **Impacto**: No cumple con el objetivo de timbre "coin cascade" metálico
- **Prioridad**: Alta (afecta objetivo principal del proyecto)
- **Posibles causas**:
  - Parámetros de resonadores modales no calibrados correctamente
  - Frecuencias de modos no optimizadas para timbre metálico
  - Excitación (noise burst) no adecuada
  - Falta de modulación o efectos que refuercen el carácter metálico

---

## Métricas de Progreso

- **Fases completadas**: 8 de 10 (80%)
  - ✅ Fase 1: Setup inicial
  - ✅ Fase 2: Sistema de partículas básico
  - ✅ Fase 3: Input básico (Mouse)
  - ✅ Fase 4: Colisiones y eventos
  - ✅ Fase 5: Comunicación OSC
  - ✅ Fase 6: Sintetizador básico (JUCE Standalone)
  - ✅ Fase 7: Receptor OSC y mapeo (partículas)
  - ✅ Fase 8: Sistema Plate paralelo (Plate Controller + PlateSynth)
- **Módulos funcionales**: 2 de 2 (100%)
  - ✅ Particles (App A) - Completo (incluye Plate Controller)
  - ✅ Sintetizador JUCE (App B) - Completo (partículas + plate)
- **Documentación**: Completa para módulos implementados

---

## Referencias

- [Plan de implementación](../specs/PLAN_IMPLEMENTACION.md) — Detalles de fases y tareas
- [Especificación técnica](../specs/spec.md) — Arquitectura y diseño
- [Documentación de Particles](../Particles/README.md) — Detalles del módulo

---

**Última actualización**: 2026-02-10 (Mejoras de simetría y estabilidad en simulación Chladni)
