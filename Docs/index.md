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

**Código fuente:** `app-juce/Source/`

**Estado**: Pendiente de implementación

**Documentación**: Pendiente

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
- **JUCE**: Pendiente

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
3. [Contrato OSC](api-osc.md) (pendiente)
4. [Plan de implementación](specs/PLAN_IMPLEMENTACION.md)

### Para Contribuidores
1. [Estado actual](overview/ESTADO_ACTUAL.md)
2. [Plan de implementación](specs/PLAN_IMPLEMENTACION.md)
3. [Documentación de módulos](Particles/)

Ver [README.md](README.md) para más detalles sobre rutas de aprendizaje.

---

**Última actualización:** 2026-01-20
