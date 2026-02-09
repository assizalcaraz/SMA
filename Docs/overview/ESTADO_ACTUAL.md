# Estado Actual del Proyecto

**Última actualización**: 2026-01-20

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

**Fase 4: Colisiones y Eventos**
- ⏳ Detección de colisiones con bordes
- ⏳ Sistema de rebote de partículas
- ⏳ Generación de eventos de hit
- ⏳ Sistema de cooldown por partícula
- ⏳ Rate limiting global
- ⏳ Formato final de `/hit` (contrato congelado)

**Fase 5: Comunicación OSC**
- ⏳ Integración de `ofxOsc`
- ⏳ Envío de mensajes `/hit`
- ⏳ Envío de mensajes `/state` (opcional)
- ⏳ Configuración de host y puerto
- ⏳ Validación de mensajes antes de envío

#### App B - JUCE

**Fase 6: Sintetizador Básico**
- ⏳ Proyecto base JUCE Standalone
- ⏳ Estructura básica de sintetizador
- ⏳ Motor de síntesis modal
- ⏳ Timbre "coin cascade" con modos inarmónicos

**Fase 7: Receptor OSC y Mapeo**
- ⏳ Receptor OSC en puerto 9000
- ⏳ Parsing de mensajes `/hit`
- ⏳ Mapeo de parámetros a síntesis
- ⏳ Polyphony (16-64 voces) con voice stealing
- ⏳ Master limiter

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
- ⏳ Colisiones y eventos (en desarrollo)
- ⏳ Comunicación OSC (pendiente)

**Documentación**: Completa en `Docs/Particles/`

---

### Módulo JUCE (App B)

**Ubicación**: `app-juce/Source/`

**Estado**: Pendiente de implementación

**Funcionalidades requeridas**:
- ⏳ Sintetizador modal
- ⏳ Receptor OSC
- ⏳ Mapeo de parámetros
- ⏳ Polyphony con voice stealing

**Documentación**: Pendiente

---

## Próximos Pasos

### Corto Plazo (Próximas 2-4 semanas)

1. **Completar Fase 4**: Colisiones y eventos
   - Implementar detección de colisiones
   - Sistema de rebote
   - Generación de eventos `/hit`
   - Rate limiting

2. **Iniciar Fase 5**: Comunicación OSC
   - Integrar `ofxOsc`
   - Implementar envío de mensajes

3. **Iniciar Fase 6**: Sintetizador JUCE básico
   - Crear proyecto base
   - Estructura de síntesis modal

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
3. ⏳ Colisiones y eventos
4. ⏳ Comunicación OSC
5. ⏳ Sintetizador JUCE
6. ⏳ Receptor OSC y mapeo
7. ⏳ Calibración conjunta
8. ⏳ MediaPipe (opcional/tardía)

---

## Issues Conocidos

- Ninguno reportado actualmente

---

## Métricas de Progreso

- **Fases completadas**: 2 de 10 (20%)
- **Módulos funcionales**: 1 de 2 (50% - Particles parcial)
- **Documentación**: Completa para módulos implementados

---

## Referencias

- [Plan de implementación](../specs/PLAN_IMPLEMENTACION.md) — Detalles de fases y tareas
- [Especificación técnica](../specs/spec.md) — Arquitectura y diseño
- [Documentación de Particles](../Particles/README.md) — Detalles del módulo

---

**Última actualización**: 2026-01-20
