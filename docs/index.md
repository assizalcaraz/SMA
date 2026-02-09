# Índice de Documentación

Índice que relaciona la estructura del código fuente con su documentación correspondiente.

---

## Mapeo Código → Documentación

### Particles (App A - openFrameworks)

**Código fuente:** `Particles/src/`

| Archivo | Documentación |
|---------|---------------|
| `Particles/src/main.cpp` | [`Docs/Particles/spec.md`](Particles/spec.md) - Punto de entrada |
| `Particles/src/ofApp.h` | [`Docs/Particles/spec.md`](Particles/spec.md) - Clase principal |
| `Particles/src/ofApp.cpp` | [`Docs/Particles/spec.md`](Particles/spec.md) - Implementación |
| `Particles/src/Particle.h` | [`Docs/Particles/spec.md`](Particles/spec.md) - Clase partícula |
| `Particles/src/Particle.cpp` | [`Docs/Particles/spec.md`](Particles/spec.md) - Física de partículas |

**Documentación del módulo:**

- [`Docs/Particles/readme.md`](Particles/readme.md) - Descripción general y arquitectura
- [`Docs/Particles/manual.md`](Particles/manual.md) - Manual de usuario (parámetros, uso)
- [`Docs/Particles/spec.md`](Particles/spec.md) - Especificación técnica detallada

---

## Documentación General

### API y Contratos

- [`Docs/api-osc.md`](api-osc.md) - Contrato OSC entre App A y App B

### Especificaciones del Proyecto

- [`spec.md`](../spec.md) - Especificación técnica completa del sistema
- [`PLAN_IMPLEMENTACION.md`](../PLAN_IMPLEMENTACION.md) - Plan de desarrollo por fases
- [`readme.md`](../readme.md) - README principal del proyecto

---

## Estructura de Documentación

```
Docs/
├── index.md                    # Este archivo (índice)
├── Particles/
│   ├── readme.md              # Descripción general del módulo
│   ├── manual.md              # Manual de usuario
│   └── spec.md                # Especificación técnica
└── (otros módulos futuros)
```

---

## Convenciones

- Cada módulo de código tiene su carpeta en `Docs/`
- Cada módulo contiene:
  - `readme.md` - Visión general y arquitectura
  - `manual.md` - Guía de uso para usuarios
  - `spec.md` - Especificación técnica para desarrolladores
- El `index.md` mapea archivos de código a su documentación

---

**Última actualización:** Fase 3 completada
