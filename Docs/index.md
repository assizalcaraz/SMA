# Índice de Documentación

Índice que relaciona la estructura del código fuente con su documentación correspondiente.

> **Nota**: Para una navegación completa de la documentación, ver [README.md](README.md) que incluye rutas de aprendizaje y organización por niveles.

---

## Mapeo Código → Documentación

### Particles (App A - openFrameworks)

**Código fuente:** `Particles/src/`

| Archivo | Documentación |
|---------|---------------|
| `Particles/src/main.cpp` | [`Particles/spec.md`](Particles/spec.md) - Punto de entrada |
| `Particles/src/ofApp.h` | [`Particles/spec.md`](Particles/spec.md) - Clase principal |
| `Particles/src/ofApp.cpp` | [`Particles/spec.md`](Particles/spec.md) - Implementación |
| `Particles/src/Particle.h` | [`Particles/spec.md`](Particles/spec.md) - Clase partícula |
| `Particles/src/Particle.cpp` | [`Particles/spec.md`](Particles/spec.md) - Física de partículas |

**Documentación del módulo:**

- [`Particles/README.md`](Particles/README.md) - Índice del módulo
- [`Particles/readme.md`](Particles/readme.md) - Descripción general y arquitectura
- [`Particles/manual.md`](Particles/manual.md) - Manual de usuario (parámetros, uso)
- [`Particles/spec.md`](Particles/spec.md) - Especificación técnica detallada

---

### JUCE (App B)

**Código fuente:** `app-juce/app-JUCE-PAS1/Source/`

| Archivo | Documentación | Descripción |
|---------|---------------|-------------|
| `app-juce/app-JUCE-PAS1/Source/Main.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Punto de entrada de la aplicación |
| `app-juce/app-JUCE-PAS1/Source/MainComponent.h` | [`JUCE/spec.md`](JUCE/spec.md) | Declaración de la clase principal (UI y OSC) |
| `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Implementación: UI, OSC receiver, audio callbacks |
| `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h` | [`JUCE/spec.md`](JUCE/spec.md) | Declaración del motor de síntesis |
| `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Implementación: orquestación, cola lock-free, limiter |
| `app-juce/app-JUCE-PAS1/Source/ModalVoice.h` | [`JUCE/spec.md`](JUCE/spec.md) | Declaración de voz modal |
| `app-juce/app-JUCE-PAS1/Source/ModalVoice.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Implementación: resonadores modales, excitación |
| `app-juce/app-JUCE-PAS1/Source/VoiceManager.h` | [`JUCE/spec.md`](JUCE/spec.md) | Declaración del gestor de polyphony |
| `app-juce/app-JUCE-PAS1/Source/VoiceManager.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Implementación: voice stealing, gestión de voces |
| `app-juce/app-JUCE-PAS1/Source/PlateSynth.h` | [`JUCE/spec.md`](JUCE/spec.md) | Declaración del sintetizador de placa |
| `app-juce/app-JUCE-PAS1/Source/PlateSynth.cpp` | [`JUCE/spec.md`](JUCE/spec.md) | Implementación: síntesis de placa metálica |
| `app-juce/app-JUCE-PAS1/Source/SynthParameters.h` | [`JUCE/spec.md`](JUCE/spec.md) | Parámetros globales del sintetizador |

**Documentación del módulo:**

- [`JUCE/README.md`](JUCE/README.md) - Índice del módulo
- [`JUCE/readme.md`](JUCE/readme.md) - Descripción general y arquitectura
- [`JUCE/manual.md`](JUCE/manual.md) - Manual de usuario (parámetros, uso)
- [`JUCE/spec.md`](JUCE/spec.md) - Especificación técnica detallada

---

### MAAD-2-CALIB (App C)

**Código fuente:** `maad-2-calib/src/`

| Archivo | Documentación | Descripción |
|---------|---------------|-------------|
| `maad-2-calib/src/main.cpp` | [`CALIB/spec.md`](CALIB/spec.md) | Punto de entrada de la aplicación (placeholder) |
| `maad-2-calib/notebooks/analysis_template.ipynb` | [`CALIB/spec.md`](CALIB/spec.md), [`CALIB/manual.md`](CALIB/manual.md) | Template de análisis Jupyter con técnicas de procesamiento de señales |

**Documentación del módulo:**

- [`CALIB/README.md`](CALIB/README.md) - Índice del módulo
- [`CALIB/readme.md`](CALIB/readme.md) - Descripción general y arquitectura
- [`CALIB/manual.md`](CALIB/manual.md) - Manual de usuario (comandos OSC, uso, análisis)
- [`CALIB/spec.md`](CALIB/spec.md) - Especificación técnica detallada

---

## Documentación General

### Nivel 1: Entendimiento General

- [`README.md`](README.md) - Índice central de documentación
- [`overview/ESTADO_ACTUAL.md`](overview/ESTADO_ACTUAL.md) - Estado del proyecto
- [`../readme.md`](../readme.md) - README principal del proyecto

### Nivel 2: Especificaciones Técnicas

- [`specs/spec.md`](specs/spec.md) - Especificación técnica completa del sistema
- [`api-osc.md`](api-osc.md) - Contrato OSC entre App A y App B (pendiente)
- [`specs/PLAN_IMPLEMENTACION.md`](specs/PLAN_IMPLEMENTACION.md) - Plan de desarrollo por fases

### Nivel 3: Documentación de Módulos

- [`Particles/README.md`](Particles/README.md) - Índice del módulo Particles
- [`JUCE/README.md`](JUCE/README.md) - Índice del módulo JUCE
- [`CALIB/README.md`](CALIB/README.md) - Índice del módulo CALIB

### Nivel 3: Guías y Procesos

- [`guides/SCOPES_DOCUMENTACION.md`](guides/SCOPES_DOCUMENTACION.md) - Guía para actualizar documentación

---

## Estructura de Documentación

```
Docs/
├── README.md                   # Índice central (navegación completa)
├── index.md                    # Este archivo (mapeo código → docs)
├── specs/                      # Nivel 2: Especificaciones técnicas
│   ├── spec.md                 # Especificación técnica completa
│   └── PLAN_IMPLEMENTACION.md  # Plan de desarrollo
├── overview/                   # Nivel 1: Entendimiento general
│   ├── ESTADO_ACTUAL.md
│   └── ANALISIS_DOCUMENTACION.md
├── guides/                     # Nivel 3: Guías y procesos
│   └── SCOPES_DOCUMENTACION.md
├── Particles/                  # Nivel 3: Documentación de módulos
│   ├── README.md              # Índice del módulo
│   ├── readme.md              # Descripción general
│   ├── manual.md              # Manual de usuario
│   └── spec.md                # Especificación técnica
├── JUCE/                       # Nivel 3: Documentación de módulos
│   ├── README.md              # Índice del módulo
│   ├── readme.md              # Descripción general
│   ├── manual.md              # Manual de usuario
│   └── spec.md                # Especificación técnica
├── CALIB/                      # Nivel 3: Documentación de módulos
│   ├── README.md              # Índice del módulo
│   ├── readme.md              # Descripción general
│   ├── manual.md              # Manual de usuario
│   └── spec.md                # Especificación técnica
└── requisitos/                 # Requisitos de entrega
    └── Trabajo Final ISTR.md
```

---

## Convenciones

- Cada módulo de código tiene su carpeta en `Docs/`
- Cada módulo contiene:
  - `README.md` - Índice del módulo (nuevo)
  - `readme.md` - Visión general y arquitectura
  - `manual.md` - Guía de uso para usuarios
  - `spec.md` - Especificación técnica para desarrolladores
- El `index.md` mapea archivos de código a su documentación
- El `README.md` central organiza toda la documentación por niveles y rutas de aprendizaje

---

## Rutas de Aprendizaje

### Para Usuarios Nuevos
1. [README principal](../readme.md)
2. [Manual de Particles](Particles/manual.md)
3. [Guía de uso](../readme.md#uso-y-ejecución)

### Para Desarrolladores
1. [Especificación técnica](specs/spec.md)
2. [Documentación de módulos](Particles/)
3. [Documentación de JUCE](JUCE/README.md)
4. [Documentación de CALIB](CALIB/README.md)
5. [Contrato OSC](api-osc.md)
6. [Plan de implementación](specs/PLAN_IMPLEMENTACION.md)

### Para Contribuidores
1. [Estado actual](overview/ESTADO_ACTUAL.md)
2. [Plan de implementación](specs/PLAN_IMPLEMENTACION.md)
3. [Documentación de módulos](Particles/)
4. [Documentación de JUCE](JUCE/README.md)
5. [Documentación de CALIB](CALIB/README.md)

Ver [README.md](README.md) para más detalles sobre rutas de aprendizaje.

---

**Última actualización:** 2026-02-11
