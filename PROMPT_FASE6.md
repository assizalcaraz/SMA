# Prompt para Fase 6: Sintetizador JUCE Standalone

## Contexto del Proyecto

Estoy trabajando en un **Sistema Modular Audiovisual** que conecta openFrameworks (App A) con JUCE (App B) mediante OSC. El proyecto está dividido en fases de implementación.

**Estado actual:**
- ✅ Fase 1: Setup inicial — Completada
- ✅ Fase 2: Sistema de partículas básico — Completada
- ✅ Fase 3: Input básico (Mouse) — Completada
- ✅ Fase 4: Colisiones y eventos — Completada
- ✅ Fase 5: Comunicación OSC — Completada (ofxOsc enviando mensajes `/hit` y `/state`)
- ✅ **Fase 6: Sintetizador básico (JUCE Standalone) — COMPLETADA**

**Último commit:** `b601137 🦾 perf: estabilización RT-safe del motor de audio JUCE`

## Situación Actual

Acabo de usar **Projucer** para generar un proyecto JUCE Standalone en la carpeta:
```
app-juce/app-JUCE-PAS1/
```

El proyecto ya tiene:
- Estructura base de JUCE Standalone
- Archivos fuente en `Source/`
- Build configurado para macOS

## Objetivo: Implementar Fase 6

Necesito implementar un **sintetizador con resonadores modales** que genere un timbre metálico "Coin Cascade" (cascada de monedas). **IMPORTANTE:** Esta fase es SIN OSC aún — solo el motor de síntesis básico.

### Especificaciones Técnicas

**Diseño sonoro objetivo: "Coin Cascade"**
- Textura de múltiples hits percusivos metálicos breves
- Capas de eventos pequeños que se acumulan en textura granular
- Timbre metálico percusivo (como monedas cayendo)

**Arquitectura de síntesis:**
- Cada voz = resonador modal independiente
- Excitación: impulso corto (click / noise burst 5-20ms)
- Banco de 3-8 resonadores bandpass
- Suma de resonadores → saturación suave opcional → salida
- Modos inarmónicos para timbre metálico (frecuencias no armónicas)

**Parámetros por voz:**
- Frecuencia base
- Damping (decaimiento)
- Brightness
- Amplitud

**Sistema de polyphony:**
- `maxVoices` configurable (16-64 según CPU)
- Voice stealing (robar voz con menor amplitud residual o más antigua)
- Limiter master para anti-saturación

**UI básica requerida:**
- Controles: `Voices`, `Metalness`, `Brightness`, `Damping`, `Drive`, `ReverbMix` (opcional), `Limiter on/off`
- Indicadores: Nivel de salida, Número de voces activas

**Testing:**
- Probar con MIDI interno o triggers manuales (sin OSC aún)
- Verificar estabilidad de audio (sin glitches)
- Medir CPU usage (< 30% con 32 voces)

## Referencias Importantes

**Documentación del proyecto:**
- `Docs/specs/PLAN_IMPLEMENTACION.md` — Plan completo con detalles de Fase 6 (líneas 377-469)
- `Docs/specs/spec.md` — Especificación técnica completa (sección 5.3 para motor sonoro)
- `readme.md` — Overview general del proyecto

**Contrato OSC (para referencia futura, Fase 7):**
- Mensajes `/hit`: `id, x, y, energy, surface`
- Mensajes `/state`: `activity, gesture, presence`
- Host: `127.0.0.1`, Puerto: `9000`

## Tareas Específicas

1. **Revisar estructura del proyecto JUCE generado**
   - Verificar archivos en `app-juce/app-JUCE-PAS1/Source/`
   - Entender la estructura base del Standalone

2. **Implementar clase `ModalVoice`**
   - Resonador modal con modos inarmónicos
   - Excitación por impulso
   - Parámetros: frecuencia, damping, brightness, amplitud

3. **Implementar sistema de polyphony**
   - Pool de voces
   - Voice stealing
   - Gestión de voces activas

4. **Implementar procesamiento de audio**
   - Callbacks de audio en el MainComponent o AudioProcessor
   - Render de voces activas
   - Master limiter

5. **Crear UI básica**
   - Controles de parámetros
   - Indicadores de estado
   - Trigger manual para testing (sin OSC)

6. **Testing y optimización**
   - Probar timbre metálico
   - Verificar estabilidad
   - Medir CPU usage

## Notas Importantes

- **Standalone = entrega principal** (no plugin aún)
- **Sin OSC en esta fase** — solo motor de síntesis
- **Testing con MIDI o triggers manuales** para desarrollo
- **Timbre objetivo:** Metálico, percusivo, granular ("Coin Cascade")
- **Rendimiento objetivo:** CPU < 30% con 32 voces, sin glitches

## Formato de Commits

Usar formato: `🦾 tipo: mensaje` (sin [dia])
Autor: `Cursor Assistant <cursor@dia.local>`

---

**¿Puedes ayudarme a implementar la Fase 6 del sintetizador JUCE Standalone?** Empieza revisando la estructura del proyecto generado y luego implementa el resonador modal básico.
