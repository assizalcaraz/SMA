# Documentación del Sistema Modular Audiovisual

**Versión**: v0.1  
**Última actualización**: 2026-01-20

Este directorio contiene toda la documentación del proyecto, organizada en niveles de lectura claros y siguiendo la estructura del proyecto.

---

## Niveles de Lectura

### Nivel 1: Entendimiento General

Documentación para entender qué es el sistema, su arquitectura y cómo empezar.

- **[README principal](../readme.md)** — Descripción general del proyecto
- **[Estado actual](overview/ESTADO_ACTUAL.md)** — Estado técnico del proyecto y componentes implementados
- **[Arquitectura general](specs/spec.md#2-arquitectura-general)** — Arquitectura del sistema

**Para empezar**: Lee el [README principal](../readme.md) y luego el [Estado actual](overview/ESTADO_ACTUAL.md).

---

### Nivel 2: Especificaciones Técnicas

Documentación técnica de formatos, estructuras y especificaciones.

- **[Especificación completa](specs/spec.md)** — Especificación técnica completa del sistema
- **[Contrato OSC](api-osc.md)** — Contrato de mensajes OSC entre App A y App B (pendiente)
- **[Plan de implementación](specs/PLAN_IMPLEMENTACION.md)** — Plan detallado de desarrollo por fases

---

### Nivel 3: Documentación de Módulos

Documentación técnica de módulos y componentes del sistema.

#### App A - openFrameworks

- **[Particles](Particles/README.md)** — Módulo de partículas
  - [`readme.md`](Particles/readme.md) — Descripción general y arquitectura
  - [`manual.md`](Particles/manual.md) — Manual de usuario (parámetros, uso)
  - [`spec.md`](Particles/spec.md) — Especificación técnica detallada

#### App B - JUCE

- **JUCE App** — Documentación del sintetizador (pendiente de implementación)

---

### Nivel 3: Guías y Procesos

Guías específicas de procesos y mantenimiento.

- **[Scopes de documentación](guides/SCOPES_DOCUMENTACION.md)** — Guía para actualizar documentación según cambios
- **[Workflow Git](guides/GIT_WORKFLOW.md)** — Guía de control de versiones y convenciones de commits

---

## Rutas de Aprendizaje

### Para Usuarios Nuevos

1. **[README principal](../readme.md)** — Entender qué es el sistema
2. **[Manual de Particles](Particles/manual.md)** — Aprender a usar el módulo de partículas
3. **[Guía de uso](../readme.md#uso-y-ejecución)** — Cómo ejecutar el sistema

### Para Desarrolladores

1. **[Especificación técnica](specs/spec.md)** — Entender arquitectura y diseño
2. **[Documentación de módulos](Particles/)** — Entender implementación de código
3. **[Contrato OSC](api-osc.md)** — Entender comunicación entre apps (pendiente)
4. **[Plan de implementación](specs/PLAN_IMPLEMENTACION.md)** — Ver fases de desarrollo

### Para Contribuidores

1. **[Estado actual](overview/ESTADO_ACTUAL.md)** — Ver qué está implementado
2. **[Plan de implementación](specs/PLAN_IMPLEMENTACION.md)** — Ver qué falta implementar
3. **[Documentación de módulos](Particles/)** — Entender estructura del código
4. **[Índice código → documentación](index.md)** — Mapeo de archivos a documentación

---

## Estructura del Directorio

La estructura de documentación refleja la estructura del proyecto:

```
Docs/
├── README.md (este archivo)
├── index.md                    # Mapeo código → documentación
├── specs/                      # Nivel 2: Especificaciones técnicas
│   ├── spec.md                 # Especificación técnica completa
│   └── PLAN_IMPLEMENTACION.md  # Plan de desarrollo
├── overview/                   # Nivel 1: Entendimiento general
│   ├── ESTADO_ACTUAL.md
│   └── ANALISIS_DOCUMENTACION.md
├── guides/                     # Nivel 3: Guías y procesos
│   └── SCOPES_DOCUMENTACION.md
├── Particles/                  # Nivel 3: Documentación de módulos
│   ├── README.md
│   ├── readme.md
│   ├── manual.md
│   └── spec.md
└── requisitos/                 # Requisitos de entrega
    └── Trabajo Final ISTR.md
```

---

## Referencias Rápidas

- **Inicio rápido**: [README principal](../readme.md)
- **Instalación**: [README principal - Requisitos e instalación](../readme.md#requisitos-e-instalación)
- **Uso**: [README principal - Uso y ejecución](../readme.md#uso-y-ejecución)
- **Especificación técnica**: [specs/spec.md](specs/spec.md)
- **Contrato OSC**: [api-osc.md](api-osc.md)
- **Plan de desarrollo**: [specs/PLAN_IMPLEMENTACION.md](specs/PLAN_IMPLEMENTACION.md)
- **Documentación de Particles**: [Particles/](Particles/)
- **Estado del proyecto**: [overview/ESTADO_ACTUAL.md](overview/ESTADO_ACTUAL.md)
- **Scopes de documentación**: [guides/SCOPES_DOCUMENTACION.md](guides/SCOPES_DOCUMENTACION.md)

---

## Mantenimiento

### Actualizar Documentación

Cuando cambies código, actualiza la documentación correspondiente siguiendo la estructura del proyecto:

- **Nuevo módulo** (`Particles/`, `JUCE/`) → Crear carpeta en `Docs/` con `README.md`, `readme.md`, `manual.md`, `spec.md`
- **Cambios en contrato OSC** → Actualizar `api-osc.md` (cuando se cree)
- **Cambios arquitectónicos** → Actualizar `specs/spec.md` y `overview/ESTADO_ACTUAL.md`
- **Nuevas fases completadas** → Actualizar `overview/ESTADO_ACTUAL.md` y `specs/PLAN_IMPLEMENTACION.md`

**Guía completa**: Ver [Scopes de documentación](guides/SCOPES_DOCUMENTACION.md) para saber exactamente qué documentación actualizar según el tipo de cambio.

### Validar Links

Usa herramientas para verificar que todos los links funcionan después de reorganizaciones.

### Niveles Claros

Mantén la separación entre niveles de lectura:
- **Nivel 1**: Entendimiento general y estado del proyecto
- **Nivel 2**: Especificaciones técnicas y contratos
- **Nivel 3**: Documentación de módulos y código

---

**Última actualización**: 2026-01-20
