# Guía: Scopes de Documentación

Esta guía define los **scopes de documentación** para evitar malinterpretar qué documentación actualizar cuando se realizan cambios en el código.

---

## Propósito

Cuando se realizan cambios en el código, es importante actualizar la documentación correspondiente, pero **solo la documentación relevante**. Los scopes ayudan a:

- Identificar qué documentación actualizar según el tipo de cambio
- Evitar actualizar documentación no relacionada
- Mantener consistencia en las actualizaciones

---

## Scopes Definidos

### 1. `particles_module` — Módulo Particles (App A)

**Descripción**: Documentación del módulo de partículas en openFrameworks.

**Archivos incluidos**:
- `Docs/Particles/readme.md` (descripción general y arquitectura)
- `Docs/Particles/manual.md` (manual de usuario, parámetros)
- `Docs/Particles/spec.md` (especificación técnica)
- `Docs/Particles/README.md` (índice del módulo)

**Cuándo actualizar**:
- ✅ Cambios en `Particles/src/*.cpp` o `*.h`
- ✅ Nuevos parámetros configurables
- ✅ Cambios en física de partículas
- ✅ Cambios en comportamiento visual
- ✅ Nuevas funcionalidades de input (mouse, MediaPipe)
- ✅ Cambios en sistema de colisiones
- ✅ Cambios en generación de eventos

**Ejemplo**:
```bash
# Cambio: Agregar nuevo parámetro k_spring a Particle
# Actualizar: 
#   - Docs/Particles/spec.md (especificación técnica)
#   - Docs/Particles/manual.md (parámetro nuevo en UI)
#   - Docs/Particles/readme.md (si afecta arquitectura)
```

---

### 2. `juce_module` — Módulo JUCE (App B)

**Descripción**: Documentación del sintetizador en JUCE.

**Archivos incluidos**:
- `Docs/JUCE/README.md` (índice del módulo)
- `Docs/JUCE/readme.md` (descripción general y arquitectura)
- `Docs/JUCE/manual.md` (manual de usuario, parámetros)
- `Docs/JUCE/spec.md` (especificación técnica)

**Cuándo actualizar**:
- ✅ Cambios en `app-juce/Source/*.cpp` o `*.h`
- ✅ Nuevos parámetros de síntesis
- ✅ Cambios en motor de síntesis modal
- ✅ Cambios en receptor OSC
- ✅ Cambios en mapeo de parámetros
- ✅ Cambios en polyphony o voice stealing

**Ejemplo**:
```bash
# Cambio: Agregar nuevo modo de síntesis
# Actualizar: 
#   - Docs/JUCE/spec.md (especificación técnica del nuevo modo)
#   - Docs/JUCE/manual.md (parámetro nuevo en UI)
#   - Docs/JUCE/readme.md (si afecta arquitectura)
```

---

### 3. `calib_module` — Módulo CALIB (App C)

**Descripción**: Documentación del módulo de calibración y validación MAAD-2-CALIB.

**Archivos incluidos**:
- `Docs/CALIB/README.md` (índice del módulo)
- `Docs/CALIB/readme.md` (descripción general y arquitectura)
- `Docs/CALIB/manual.md` (manual de usuario, comandos OSC, análisis)
- `Docs/CALIB/spec.md` (especificación técnica)

**Cuándo actualizar**:
- ✅ Cambios en `maad-2-calib/src/*.cpp` o `*.h`
- ✅ Cambios en `maad-2-calib/notebooks/*.ipynb`
- ✅ Nuevos comandos OSC de control (`/test/*`)
- ✅ Cambios en estructura de artefactos de salida (NDJSON, WAV, metadata)
- ✅ Cambios en técnicas de análisis (DFT, STFT, métricas)
- ✅ Cambios en formato de datos o estructura de sesiones

**Ejemplo**:
```bash
# Cambio: Agregar nuevo comando OSC /test/pause
# Actualizar: 
#   - Docs/CALIB/spec.md (especificación técnica del nuevo comando)
#   - Docs/CALIB/manual.md (documentación del comando para usuarios)
#   - Docs/CALIB/readme.md (si afecta arquitectura)
```

---

### 4. `osc_contract` — Contrato OSC

**Descripción**: Documentación del contrato de mensajes OSC entre App A y App B.

**Archivos incluidos**:
- `Docs/api-osc.md` (contrato OSC principal)
- `spec.md` (sección de contrato OSC, si existe)
- `Docs/Particles/spec.md` (sección de envío OSC)
- `Docs/JUCE/spec.md` (sección de recepción OSC, cuando exista)

**Cuándo actualizar**:
- ✅ Cambios en formato de mensajes OSC
- ✅ Nuevos mensajes OSC (`/hit`, `/state`, `/ctrl`)
- ✅ Cambios en parámetros de mensajes
- ✅ Cambios en protocolo de comunicación
- ✅ Cambios en puerto o host

**Ejemplo**:
```bash
# Cambio: Modificar formato de mensaje /hit para incluir nuevo parámetro
# Actualizar:
#   - Docs/api-osc.md (contrato principal)
#   - Docs/Particles/spec.md (implementación de envío)
#   - Docs/JUCE/spec.md (implementación de recepción, cuando exista)
```

---

### 5. `architecture` — Arquitectura y Diseño

**Descripción**: Documentación de arquitectura, diseño y estado del proyecto.

**Archivos incluidos**:
- `Docs/specs/spec.md` (especificación técnica completa)
- `Docs/overview/ESTADO_ACTUAL.md` (estado del proyecto)
- `readme.md` (README principal, sección de arquitectura)
- `Docs/specs/PLAN_IMPLEMENTACION.md` (plan de desarrollo)

**Cuándo actualizar**:
- ✅ Cambios arquitectónicos significativos
- ✅ Nuevas decisiones de diseño
- ✅ Actualizaciones de estado del proyecto
- ✅ Cambios en estrategia de implementación
- ✅ Nuevas fases completadas
- ✅ Cambios en principios o filosofía del proyecto

**Ejemplo**:
```bash
# Cambio: Completar Fase 4 (Colisiones y eventos)
# Actualizar:
#   - Docs/overview/ESTADO_ACTUAL.md (marcar como completado)
#   - Docs/specs/PLAN_IMPLEMENTACION.md (marcar tareas como completadas)
```

---

### 6. `setup` — Instalación y Configuración

**Descripción**: Documentación de instalación, requisitos y configuración.

**Archivos incluidos**:
- `readme.md` (secciones de requisitos e instalación)
- `Docs/Particles/manual.md` (configuración del módulo)
- `Docs/JUCE/manual.md` (configuración del módulo)
- `Docs/CALIB/manual.md` (configuración del módulo)

**Cuándo actualizar**:
- ✅ Cambios en requisitos del sistema
- ✅ Cambios en proceso de instalación
- ✅ Nuevas dependencias
- ✅ Cambios en configuración inicial
- ✅ Cambios en scripts de setup

**Ejemplo**:
```bash
# Cambio: Agregar nueva dependencia (ej: ofxMediaPipe)
# Actualizar:
#   - readme.md (sección de requisitos e instalación)
#   - Docs/Particles/manual.md (si afecta configuración del módulo)
```

---

### 7. `workflows` — Flujos de Trabajo y Guías

**Descripción**: Documentación de procesos, workflows y guías de uso.

**Archivos incluidos**:
- `readme.md` (sección de uso y ejecución)
- `Docs/README.md` (rutas de aprendizaje)
- `Docs/index.md` (índice de documentación)

**Cuándo actualizar**:
- ✅ Cambios en flujos de trabajo documentados
- ✅ Nuevos workflows
- ✅ Actualizaciones de procesos
- ✅ Cambios en recomendaciones de uso
- ✅ Reorganización de documentación

**Ejemplo**:
```bash
# Cambio: Nuevo workflow de calibración
# Actualizar: readme.md (sección de uso), Docs/README.md (si afecta rutas)
```

---

## Uso en Desarrollo

### Proceso Recomendado

1. **Identificar el scope**: Determinar qué scope(s) aplican según el cambio realizado
2. **Revisar triggers**: Verificar si el cambio activa algún trigger del scope
3. **Actualizar documentación**: Actualizar solo los archivos del scope relevante
4. **Commit separado**: Hacer commit de documentación con scope claro

### Ejemplo de Workflow

```bash
# 1. Cambio en código
# Editar: Particles/src/Particle.cpp (agregar nueva fuerza F_spring)

# 2. Identificar scope
# Scope: particles_module
# Archivos a actualizar:
#   - Docs/Particles/spec.md (especificación técnica de F_spring)
#   - Docs/Particles/manual.md (nuevo parámetro k_spring en UI)

# 3. Actualizar documentación
# Editar archivos del scope

# 4. Commits
git add Particles/src/Particle.cpp Particles/src/Particle.h
git commit -m "feat: agregar fuerza de resorte a partículas"

git add Docs/Particles/spec.md Docs/Particles/manual.md
git commit -m "docs(particles_module): documentar nueva fuerza F_spring"
```

---

## Casos Especiales

### Cambios que Afectan Múltiples Scopes

Algunos cambios pueden afectar múltiples scopes. En estos casos, actualiza todos los scopes relevantes:

**Ejemplo**: Cambio en contrato OSC que afecta tanto Particles como JUCE
```bash
# Cambio: Modificar formato de /hit
# Scopes afectados: osc_contract, particles_module, juce_module
# Actualizar:
#   - Docs/api-osc.md (osc_contract)
#   - Docs/Particles/spec.md (particles_module)
#   - Docs/JUCE/spec.md (juce_module)
```

**Ejemplo**: Cambio en comandos OSC de control que afecta CALIB
```bash
# Cambio: Agregar nuevo comando /test/pause
# Scopes afectados: osc_contract, calib_module
# Actualizar:
#   - Docs/api-osc.md (osc_contract, si se documenta allí)
#   - Docs/CALIB/spec.md (calib_module)
#   - Docs/CALIB/manual.md (calib_module)
```

### Cambios Arquitectónicos

Los cambios arquitectónicos suelen requerir actualizar múltiples documentos:
- `Docs/specs/spec.md` (especificación)
- `Docs/overview/ESTADO_ACTUAL.md` (estado)
- `Docs/specs/PLAN_IMPLEMENTACION.md` (plan)
- Documentación de módulos afectados

---

## Notas Importantes

- **No actualizar documentación no relacionada**: Si cambias código de Particles, no actualices documentación de JUCE (a menos que afecte el contrato OSC)
- **Scopes pueden solaparse**: Un cambio puede afectar múltiples scopes (ej: nuevo parámetro que afecta contrato OSC)
- **Priorizar relevancia**: Actualizar solo si el cambio es significativo para el usuario/desarrollador
- **Mantener consistencia**: Usar el mismo formato y estilo en todas las actualizaciones
- **Actualizar índices**: Si agregas nuevos documentos, actualiza `Docs/README.md` y `Docs/index.md`

---

## Preguntas Frecuentes

**¿Qué pasa si un cambio afecta múltiples scopes?**
- Actualiza todos los scopes relevantes, pero en commits separados si es posible

**¿Debo actualizar documentación para cambios menores?**
- Solo si el cambio afecta el comportamiento visible, la API pública, o los parámetros configurables

**¿Cómo sé qué archivos actualizar exactamente?**
- Revisa la lista de archivos incluidos en el scope correspondiente arriba

**¿Debo actualizar ESTADO_ACTUAL.md para cada cambio?**
- Solo para cambios significativos: nuevas fases completadas, nuevos módulos, cambios arquitectónicos

**¿Qué hacer cuando se crea un nuevo módulo?**
- Crear carpeta en `Docs/` con `README.md`, `readme.md`, `manual.md`, `spec.md`
- Actualizar `Docs/README.md` con el nuevo módulo
- Actualizar `Docs/index.md` con el mapeo código → documentación
- Actualizar `Docs/overview/ESTADO_ACTUAL.md` con el estado del nuevo módulo
- Agregar scope correspondiente en `Docs/guides/SCOPES_DOCUMENTACION.md`

---

## Referencias

- [README de documentación](../README.md) — Estructura completa de documentación
- [Estado actual](../overview/ESTADO_ACTUAL.md) — Estado del proyecto
- [Análisis de documentación](../overview/ANALISIS_DOCUMENTACION.md) — Mejores prácticas adoptadas

---

**Última actualización**: 2026-01-20
