# Documentación del Módulo Particles

**Módulo**: App A - openFrameworks  
**Estado**: Fase 3 completada (input mouse funcionando)

Este módulo implementa el sistema de partículas físicas que responde a gestos del usuario (mouse o MediaPipe) y genera eventos sonoros mediante colisiones.

---

## Documentación Disponible

- **[readme.md](readme.md)** — Descripción general y arquitectura del módulo
- **[manual.md](manual.md)** — Manual de usuario (parámetros, uso, configuración)
- **[spec.md](spec.md)** — Especificación técnica detallada (implementación, física, código)

---

## Mapeo Código → Documentación

| Archivo | Documentación | Descripción |
|---------|---------------|-------------|
| `Particles/src/main.cpp` | [spec.md](spec.md) | Punto de entrada de la aplicación |
| `Particles/src/ofApp.h` | [spec.md](spec.md) | Declaración de la clase principal |
| `Particles/src/ofApp.cpp` | [spec.md](spec.md) | Implementación: setup, update, draw, input, física |
| `Particles/src/Particle.h` | [spec.md](spec.md) | Declaración de la clase partícula |
| `Particles/src/Particle.cpp` | [spec.md](spec.md) | Implementación de física de partículas |

---

## Estado de Implementación

### ✅ Completado

- **Fase 2**: Sistema de partículas básico
  - Física de partículas (F_home, F_drag)
  - Integración semi-implícita Euler
  - UI con sliders ajustables
  - Rendering de puntos
  - Debug overlay

- **Fase 3**: Input básico (Mouse)
  - Captura de posición y velocidad del mouse
  - Suavizado de input
  - Fuerza de gesto con influencia gaussiana
  - Parámetros ajustables

### ⏳ En Desarrollo

- **Fase 4**: Colisiones y eventos
  - Detección de colisiones con bordes
  - Sistema de rebote
  - Generación de eventos de hit
  - Rate limiting

### 📋 Pendiente

- **Fase 5**: Comunicación OSC
- **Fase 3b**: Integración MediaPipe (opcional/tardía)

---

## Características Principales

### Física de Partículas

- Sistema de partículas con fuerzas dinámicas
- Retorno al origen (home)
- Influencia de gestos del usuario
- Integración numérica estable

### Input

- **Mouse**: Implementado y funcionando
- **MediaPipe**: Pendiente (opcional)

### Parámetros Ajustables

- `N_particles` (500-8000) — Número de partículas
- `k_home` (0.5-6.0) — Fuerza de retorno al origen
- `k_drag` (0.5-3.0) — Fuerza de drag
- `k_gesture` — Fuerza de gesto
- `sigma` — Radio de influencia del gesto
- `speed_ref` — Velocidad de referencia

---

## Dependencias

- **openFrameworks** 0.12.1
- **ofxGui** — Para interfaz de parámetros
- **OpenGL** — Para rendering
- **ofxOsc** — Para comunicación OSC (pendiente)

---

## Documentación Relacionada

- **[Contrato OSC](../api-osc.md)** — Contrato de mensajes OSC (cuando se implemente)
- **[Especificación técnica completa](../specs/spec.md)** — Arquitectura general del sistema
- **[Plan de implementación](../specs/PLAN_IMPLEMENTACION.md)** — Fases de desarrollo

---

## Referencias Rápidas

- **Descripción general**: [readme.md](readme.md)
- **Cómo usar**: [manual.md](manual.md)
- **Detalles técnicos**: [spec.md](spec.md)
- **Estado del proyecto**: [../overview/ESTADO_ACTUAL.md](../overview/ESTADO_ACTUAL.md)

---

**Última actualización**: 2026-01-20
