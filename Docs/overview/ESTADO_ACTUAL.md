# Estado Actual del Proyecto

**Última actualización**: 2025-02-09

Este documento describe el estado actual de implementación del Sistema Modular Audiovisual, qué componentes están completados, en desarrollo o pendientes.

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

**Fase 5: Comunicación OSC**
- ⏳ Integración de `ofxOsc`
- ⏳ Envío de mensajes `/hit`
- ⏳ Envío de mensajes `/state` (opcional)
- ⏳ Configuración de host y puerto
- ⏳ Validación de mensajes antes de envío

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

**Documentación**: Completa en `Docs/Particles/`

---

### Módulo JUCE (App B)

**Ubicación**: `app-juce/app-JUCE-PAS1/Source/`

**Estado**: ✅ Fase 6 completada - Sintetizador básico funcionando

**Archivos principales**:
- `Main.cpp` — Punto de entrada ✅
- `MainComponent.h/cpp` — UI y orquestación ✅
- `ModalVoice.h/cpp` — Resonador modal individual ✅
- `VoiceManager.h/cpp` — Gestión de polyphony ✅
- `SynthesisEngine.h/cpp` — Motor de síntesis principal ✅
- `SynthParameters.h` — Parámetros globales ✅

**Funcionalidades implementadas**:
- ✅ Sintetizador modal con resonadores inarmónicos (2 modos por voz)
- ✅ Sistema de polyphony con voice stealing (4-12 voces configurables)
- ✅ Cola lock-free para eventos (preparada para OSC)
- ✅ UI básica con controles y indicadores
- ✅ Master limiter y saturación opcional
- ✅ Optimizaciones RT-safe (sin allocations en audio thread)
- ⏳ Receptor OSC (pendiente Fase 7)
- ⏳ Mapeo de parámetros OSC (pendiente Fase 7)

**Características técnicas**:
- Modos resonantes: 2 por voz (optimizado para RT)
- Excitación: noise burst de ~5ms (128 samples)
- Voces: 4-12 configurables (por defecto: 8)
- Límite de eventos: 16 por bloque de audio (MAX_HITS_PER_BLOCK)
- Pre-allocation: hasta 32 voces (solo se activan según maxVoices)

**Documentación**: Ver `PROMPT_FASE6.md` para detalles de implementación

---

## Próximos Pasos

### Corto Plazo (Próximas 2-4 semanas)

1. **Completar Fase 7**: Receptor OSC y mapeo
   - Integrar receptor OSC en JUCE (puerto 9000)
   - Conectar mensajes `/hit` a cola lock-free existente
   - Implementar mapeo de parámetros OSC → síntesis
   - Validar loop completo: oF → OSC → JUCE → Audio

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
6. ⏳ Receptor OSC y mapeo
7. ⏳ Calibración conjunta
8. ⏳ MediaPipe (opcional/tardía)

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

- **Fases completadas**: 6 de 10 (60%)
  - ✅ Fase 1: Setup inicial
  - ✅ Fase 2: Sistema de partículas básico
  - ✅ Fase 3: Input básico (Mouse)
  - ✅ Fase 4: Colisiones y eventos
  - ✅ Fase 5: Comunicación OSC
  - ✅ Fase 6: Sintetizador básico (JUCE Standalone)
- **Módulos funcionales**: 2 de 2 (100%)
  - ✅ Particles (App A) - Completo
  - ✅ Sintetizador JUCE (App B) - Básico funcionando (pendiente OSC)
- **Documentación**: Completa para módulos implementados

---

## Referencias

- [Plan de implementación](../specs/PLAN_IMPLEMENTACION.md) — Detalles de fases y tareas
- [Especificación técnica](../specs/spec.md) — Arquitectura y diseño
- [Documentación de Particles](../Particles/README.md) — Detalles del módulo

---

**Última actualización**: 2025-02-09
