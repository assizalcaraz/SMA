# Prompt para Plan v0.4 - Sistema Modular Audiovisual

Copia y pega este prompt en un nuevo chat en modo plan:

---

## Contexto

Necesito elaborar un plan de implementación para la versión v0.4 del Sistema Modular Audiovisual. Se han realizado cambios documentales para incluir el módulo CALIB de forma orgánica en la especificación.

## Documentos Modificados (Referencia para Construir Plan Consistente)

Los siguientes documentos han sido actualizados y deben ser considerados para construir un plan consistente:

1. **`Docs/specs/spec.md`** — Actualizado con:
   - Sección completa "App C — MAAD-2-CALIB" (sección 6)
   - Diagrama de flujo actualizado incluyendo CALIB
   - Contrato OSC de control (`/test/*`)
   - Arquitectura e integración no invasiva documentada

2. **`Docs/specs/PLAN_IMPLEMENTACION.md`** — Actualizado con:
   - Fase 10 completa: "App C — MAAD-2-CALIB" con subfases:
     - Fase 10.1: Modo Test en Apps (oF y JUCE)
     - Fase 10.2: CONTROL — Orquestación OSC
     - Fase 10.3: REGISTRATION — Captura de Datos
     - Fase 10.4: ANALYSIS — Notebook de Análisis
   - Estado actual actualizado

3. **`Docs/overview/ESTADO_ACTUAL.md`** — Actualizado con:
   - Estado detallado de implementación de CALIB
   - Componentes implementados vs. pendientes
   - Próximos pasos priorizados para v0.4

4. **`Docs/CALIB/README.md`** — Referencias verificadas:
   - Rutas relativas correctas a `spec.md`, `PLAN_IMPLEMENTACION.md`, `ESTADO_ACTUAL.md`
   - Arquitectura y flujo de datos documentados

## Estado Actual del Proyecto

- **oF (Particles)**: ✅ Funcional, suficiente para v0.4
- **JUCE**: ✅ Funcional, pero con problemas conocidos:
  - Timbre no suena metálico (más a "pluc" de madera)
  - Sliders no afectan el sonido significativamente
  - Falta validación de que todos los impactos generen sonido
- **CALIB**: ✅ Scaffolding completado (documentación + estructura)
  - CONTROL: No implementado (placeholder)
  - REGISTRATION: No implementado (placeholder)
  - ANALYSIS: Notebook template vacío (solo estructura)

## Objetivo del Plan

Elaborar un plan de implementación detallado para v0.4 que incluya:

1. **JUCE — Mejoras de Timbre y Validación**:
   - Implementar ADSR en ModalVoice
   - Ajustar parámetros para timbre metálico
   - Warnings de saturación
   - Sistema de logging de hits recibidos vs. sonorizados
   - Mejorar mapeo de sliders

2. **CALIB — Implementación Completa**:
   - Modo test en apps oF y JUCE (receptores `/test/*`)
   - CONTROL (orquestación OSC)
   - REGISTRATION (captura NDJSON + WAV)
   - Notebook de análisis completo con validación de coherencia simulación ↔ síntesis

3. **oF — Mejoras Opcionales** (si hay tiempo):
   - Mejorar controles de cámara
   - Implementar MediaPipe

## Pasos Iniciales Requeridos

Antes de elaborar el plan, ejecutar:

1. **Commit de cambios documentales actuales**:
   ```bash
   git add Docs/specs/spec.md Docs/specs/PLAN_IMPLEMENTACION.md Docs/overview/ESTADO_ACTUAL.md
   git commit -m "docs: actualizar especificación y plan con módulo CALIB (v0.4)"
   ```

2. **Crear branch v0.4**:
   ```bash
   git checkout -b v0.4
   ```

3. **Opcional: Backup del estado actual**:
   ```bash
   git checkout -b v0.3-backup
   git checkout v0.4
   ```

## Referencias Clave

- Plan original: `~/.cursor/plans/consistencia_y_roadmap_v0.4_474f4e68.plan.md`
- Especificación técnica: `Docs/specs/spec.md`
- Plan de implementación: `Docs/specs/PLAN_IMPLEMENTACION.md`
- Estado actual: `Docs/overview/ESTADO_ACTUAL.md`
- Documentación CALIB: `Docs/CALIB/README.md`

## Notas Importantes

- El notebook CALIB es **CRÍTICO**: debe demostrar claramente qué datos se estudian y por qué, para validar coherencia entre simulación y síntesis
- La sincronización temporal es crítica para validación experimental
- La reproducibilidad (semillas y parámetros controlados) es esencial para análisis científico

---

**Instrucciones**: Elabora un plan detallado de implementación para v0.4 considerando los documentos modificados y el estado actual del proyecto. El plan debe ser específico, con tareas accionables y referencias a archivos concretos.
