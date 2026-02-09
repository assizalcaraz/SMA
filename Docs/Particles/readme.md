# Particles — Módulo de Partículas

Módulo principal de la aplicación openFrameworks (App A) que implementa el sistema de partículas físicas con input de gestos.

---

## Descripción

El módulo `Particles` simula un sistema de partículas físicas que responde a gestos del usuario (mouse o MediaPipe). Las partículas se mueven bajo la influencia de fuerzas físicas y pueden generar eventos sonoros al colisionar con los bordes.

**Estado actual:** Fase 3 completada (input mouse funcionando)

---

## Arquitectura

### Componentes Principales

1. **Clase `Particle`** (`Particle.h/cpp`)
   - Representa una partícula individual
   - Propiedades: posición, velocidad, masa, home
   - Física: fuerzas de retorno y drag

2. **Clase `ofApp`** (`ofApp.h/cpp`)
   - Aplicación principal
   - Gestión del sistema de partículas
   - Input (mouse/gestos)
   - Rendering y UI

3. **Input `MouseEfector`**
   - Captura de posición y velocidad del mouse
   - Suavizado de input
   - Cálculo de fuerzas de gesto

### Flujo de Datos

```
Input (mouse) → updateMouseInput() → applyGestureForce() → 
Partículas → update() → draw()
```

---

## Características Implementadas

### Fase 2: Sistema de Partículas Básico
- ✅ Sistema de partículas con física (F_home, F_drag)
- ✅ Integración semi-implícita Euler
- ✅ UI con sliders ajustables en tiempo real
- ✅ Rendering de puntos
- ✅ Debug overlay

### Fase 3: Input Básico (Mouse)
- ✅ Captura de posición y velocidad del mouse
- ✅ Suavizado de input
- ✅ Fuerza de gesto con influencia gaussiana
- ✅ Parámetros ajustables: k_gesture, sigma, speed_ref

### Próximas Fases
- ⏳ Colisiones y eventos (Fase 4)
- ⏳ Comunicación OSC (Fase 5)
- ⏳ Integración MediaPipe (Fase 3b)

---

## Archivos del Módulo

| Archivo | Descripción |
|---------|-------------|
| `main.cpp` | Punto de entrada de la aplicación |
| `ofApp.h` | Declaración de la clase principal |
| `ofApp.cpp` | Implementación: setup, update, draw, input, física |
| `Particle.h` | Declaración de la clase partícula |
| `Particle.cpp` | Implementación de física de partículas |

---

## Documentación Relacionada

- [`manual.md`](manual.md) - Manual de usuario (parámetros, uso)
- [`spec.md`](spec.md) - Especificación técnica detallada
- [`../api-osc.md`](../api-osc.md) - Contrato OSC (cuando se implemente)

---

## Dependencias

- **openFrameworks** 0.12.1
- **ofxGui** - Para interfaz de parámetros
- **OpenGL** - Para rendering

---

**Última actualización:** Fase 3 completada
