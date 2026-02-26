# Plan de implementación: sonificación multi-evento (ventana 20 ms, 4 cuadrantes)

Objetivo: mejorar la consistencia perceptual cuando hay muchas partículas (muchas colisiones en ISTR, menos eventos sonificados en PAS) mediante agregación por ventana y cuadrante, sin cambiar ISTR ni el código de PlateSynth; PlateSynth se desactiva por bypass en PAS.

---

## Plan Corrections (resumen de ajustes incorporados)

- **A1) Timing / jitter:** No depender solo del wall-clock del message thread. La **emisión** de “fused hit” se alinea al audio: agregación en message thread, pero los *FusedHitSnapshot* se encolan en una cola lock-free y **solo el audio thread** hace `triggerVoice` al drenar la cola. Así el playback queda cuantizado a bloques y se evita jitter respecto al schedule de audio.
- **A2) Semántica de “energy”:** El valor `energy` de ISTR es una heurística normalizada (0..1) por velocidad/distancia, **no** energía física. En el plan se trata como **impactIntensity** (intensidad perceptual). La agregación por suma de potencias y raíz se describe como “aggregation de amplitud inspirada en física”, no como físicamente exacta. Se especifica el mapeo previo (p. ej. `a_i = pow(impactIntensity, 1.5)`) y que curvas equal-loudness/Fletcher–Munson son posibles más adelante pero no definen la existencia del evento.
- **A3) PlateSynth bypass y documentación:** Se define que PAS ignora `/plate` **en ingestión** (MainComponent no llama a `triggerPlateFromOSC` cuando el bypass está activo). Se eliminan controles de Plate de la UI. OSC_SCHEMA.md, REVIEW_PACK_PAS.md y ARQUITECTURA.md se actualizan para indicar que, con bypass activo, `/plate` no afecta la salida de audio; el esquema de `/plate` se mantiene para ISTR y futuras versiones.

---

## 1) Problem statement

Con alta densidad de partículas, ISTR genera muchas colisiones (bordes y partícula-partícula) pero el audio en PAS representa menos eventos por:

- **ISTR:** rate limiting (cooldown por partícula, max_hits_per_second, max_hits_per_frame) que descarta hits.
- **PAS:** tamaño de cola (`EVENT_QUEUE_SIZE` 128), `MAX_HITS_PER_BLOCK` (32; 16 si backlog > 64), descarte cuando la cola está llena.
- **Polyfonía:** `maxVoices` (4–12) y voice stealing: sin voz libre se reutiliza una voz (reemplazo, no pérdida silenciosa), lo que se percibe como “menos golpes que colisiones”.

La mejora busca una representación **fusionada** (varios hits → menos eventos audibles pero con energía/espacio coherentes) en lugar de pérdida silenciosa.

---

## 2) Arquitectura propuesta (solo PAS): diseño en dos etapas

### 2.1 Dónde ocurre cada cosa

- **Message thread:** Recibe `/hit`, valida y extrae (x, y, impactIntensity, surface). Acumula en **buckets por cuadrante** con ventanas de 20 ms. El cierre de ventana se hace con un **Timer ligero en el message thread** que dispara cada 20 ms (v1 por defecto); alternativamente un timer de 10 ms que cierra ventana cuando han transcurrido ≥20 ms desde el inicio de la ventana. Al cerrar una ventana, construye un **FusedHitSnapshot** por cuadrante (si no está vacío) y lo **escribe en una cola lock-free** destinada al audio thread. *Nota:* el jitter del timer es aceptable porque el disparo real de voces ocurre en el audio thread (alineado a bloque); el timer solo define el agrupamiento.
- **Audio thread:** En `renderNextBlock` / `processEventQueue`, **drena la cola de fused hits** y por cada snapshot llama a `voiceManager.triggerVoice(...)`. Los eventos fusionados **solo se disparan aquí**; nunca se disparan voces desde el message thread.

Así se mantiene RT-safety (sin locks, allocs ni logging en audio thread) y se mejora el determinismo: la salida audible queda alineada a los límites de bloque de audio, evitando jitter/drift respecto al schedule de audio.

### 2.2 Estructuras de datos

- **Agregador por cuadrante:** 4 buckets (uno por quad 0..3). Cada uno: ventana 20 ms, acumuladores para agregación (véase sección 3), contador de hits (con cap `MAX_HITS_PER_QUAD_PER_WINDOW`), ratio bordes vs p-p.
- **FusedHitSnapshot:** Estructura compacta por evento fusionado (amplitud, baseFreq, damping, brightness, waveform, **pan o x_rep** para constant-power; véase punto 3 sobre dónde se aplica pan). Se escribe en la cola lock-free.
- **Cola de fused hits:** Lock-free (AbstractFifo o equivalente). **Capacidad:** `FUSED_QUEUE_SIZE` (ej. 256 o 128). Justificación: hasta 4 snapshots por ventana de 20 ms → ~200 snapshots/s en pico; 256 da margen. **Comportamiento en overflow (v1):** descartar el **nuevo** snapshot (drop-new), incrementar `fusedHitsDiscardedQueue`; no sobrescribir. Alternativa más compleja (reemplazar el más antiguo por cuadrante) queda fuera de v1.

### 2.3 Contadores y métricas (claridad)

- **hitsReceivedRaw:** incrementado por cada `/hit` recibido y validado (antes de agregación).
- **fusedWindowsProduced** (o **fusedHitsProduced**): número de FusedHitSnapshot generados al cerrar ventanas (por ejemplo, hasta 4 por ventana de 20 ms).
- **fusedHitsEnqueued:** cuántos de esos se escribieron en la cola con éxito.
- **fusedHitsDiscardedQueue:** cuando la cola no tenía espacio para un snapshot.
- **rawHitsDiscardedAggregatorOverflow** (opcional): hits que no cupieron en un cuadrante porque se alcanzó `MAX_HITS_PER_QUAD_PER_WINDOW` (comportamiento de overflow definido en sección 3).

**Panning (v1 – opción A):** El pan (gL/gR constant-power) **no** se aplica dentro de cada voz; VoiceManager/ModalVoice siguen mezclando a mono. Se aplica **en la etapa de mezcla del engine**: SynthesisEngine (o capa de mix) renderiza cada voz a un buffer mono temporal y, al sumar a la salida estéreo, aplica gL/gR del FusedHitSnapshot correspondiente. Así los cambios en el motor de voces son mínimos.

Archivos implicados (conceptual): `MainComponent.cpp` (mapOSCHitToEvent → alimentar agregadores; Timer 20 ms; enqueue de FusedHitSnapshot), módulo o clase de agregación (p. ej. HitAggregator); `SynthesisEngine` (cola de fused events, drenado en `processEventQueue`, **aplicación de pan en mix**; o cola unificada raw vs fused); `VoiceManager` sin cambios de interfaz para `triggerVoice`.

---

## 3) Algoritmo de fusión por cuadrante y ventana

### 3.1 Entradas por hit

- x, y (0..1), **impactIntensity** (0..1; en OSC aparece como argumento `energy`), surface (0..3 bordes, -1 p-p). Todas disponibles en `mapOSCHitToEvent`.

### 3.2 Cuadrante y convención de ejes

- **Cuadrante:** `qx = (x >= 0.5f) ? 1 : 0`, `qy = (y >= 0.5f) ? 1 : 0`, `quad = qx + 2*qy` (0..3). El split usa `y >= 0.5` pero **no asume** si y=0 es “arriba” o “abajo” en pantalla.
- **Convención de coordenadas (Coordinate Convention):** Cualquier mapeo que use `y` (p. ej. damping, brightness, baseFreq) debe **declarar explícitamente** si y=0 es top o bottom y debe coincidir con ISTR. **Acción previa al afinado:** Verificar en ISTR la normalización (p. ej. `pos.y / height`) y si el origen es top-left (típico en oF) antes de fijar reglas basadas en y.

### 3.3 Semántica de intensidad y amplitud (A2)

- En el plan y en la documentación se usa de forma consistente:
  - **impactIntensity:** valor 0..1 enviado por ISTR (heurística de velocidad/distancia), no energía física.
  - **Amplitud (para síntesis):** resultado de mapeo/agregación, p. ej. `a_i = pow(impactIntensity, 1.5)` (mapeo actual de PAS) y luego agregación.
  - **Power (para agregación):** se suma **power** (p. ej. `a_i²`) para obtener una magnitud agregada; luego `a_out = sqrt(E)` (aggregation de amplitud inspirada en física, no físicamente exacta).
- Opción fijada en el plan: usar `a_i = pow(impactIntensity, 1.5)` antes de agregar; entonces `E = Σ(a_i²)`, `a_out = sqrt(E)`, y clamp si hace falta. (Alternativa: agregar en intensity y aplicar exponente después de la fusión; dejar documentado cuál se eligió.)
- Nota: curvas equal-loudness/Fletcher–Munson pueden usarse después como compensación perceptual, pero no definen la existencia del evento.

### 3.4 Cap por cuadrante por ventana (R2)

- **MAX_HITS_PER_QUAD_PER_WINDOW:** límite máximo de hits que se incorporan a la agregación por cuadrante por ventana.
- **Comportamiento en overflow:** seguir acumulando solo **power** (y centroide ponderado) sin aumentar el count, O descartar los más débiles (drop weakest). El plan debe dejar definido uno (p. ej. “se sigue sumando power y centroide; count se capa a MAX_HITS_PER_QUAD_PER_WINDOW para las heurísticas de brightness/damping”).

### 3.5 Agregación por (quad, ventana)

- Power: `E = Σ(a_i²)` con `a_i = pow(impactIntensity, 1.5)` (o el mapeo elegido).
- Centroide x (para pan): `x_rep = Σ(x_i * w_i) / Σ(w_i)`, `w_i = a_i²`; pan = 2*x_rep - 1; constant-power: `gL = sqrt(0.5*(1 - pan))`, `gR = sqrt(0.5*(1 + pan))`.
- Contador de hits (cap por MAX_HITS_PER_QUAD_PER_WINDOW) y ratio bordes (surface 0..3) vs p-p (surface -1).

### 3.6 Parámetros de sonido derivados

- **Amplitud:** `a_out = sqrt(E)`, clamp, luego clipper existente.
- **baseFreq (R1 – determinista para eventos fusionados):** mapeo determinista a partir de y y/o surface (p. ej. por cuadrante o por y representativo), en rango 100–800 Hz. **Sin aleatoriedad** en eventos fusionados para mejorar repetibilidad y claridad.
- **Brightness:** aumentar con número de eventos (más densidad → más ancho de banda); saturar en un máximo.
- **Damping:** ligeramente menor con más densidad (más cuerpo).
- **Waveform:** según intensidad agregada o count (más eventos → más Noise/Saw).
- **Bordes vs p-p:** surface predominante o ratio → bordes (0..3) algo más bajo y grave (-3 a -5 dB en amplitud o baseFreq/brightness más bajo); p-p (-1) línea base o ligeramente más alto.

### 3.7 Determinismo y coste

- Sin aleatoriedad en la fusión (baseFreq determinista). Coste acotado por MAX_HITS_PER_QUAD_PER_WINDOW y por ventana.

---

## 4) Gestión de voces (evaluar, sin código)

- **Opción B (recomendada como primera a probar – R4): Presupuesto mínimo por cuadrante.** Reservar `maxVoicesPerQuadrant` (ej. 2–4) por cuadrante; el resto compartido. Pros: mejor distribución espacial, evita que un cuadrante acapare todas las voces. Contras: más lógica en VoiceManager, posible subutilización. Criterios de aceptación: cobertura por cuadrante, estabilidad RT, pan L/R estable.
- **Opción A: Pool global con límite por cuadrante por ventana.** Un solo pool; limitar eventos fusionados encolados por cuadrante por ventana (ej. 1 por cuadrante cada 20 ms). Pros: simple. Contras: un cuadrante muy activo puede seguir acaparando voces.
- **Opción C: Aumentar maxVoices con salvaguardas.** Subir maxVoices (ej. 16–24) con techo de CPU y voice stealing mejorado. Pros: más eventos audibles. Contras: riesgo de picos de CPU.

Evaluar primero B; si no basta, combinar con A o C según criterios de aceptación (cobertura, RT, distribución L/R).

---

## 5) Bypass de PlateSynth (A3)

- **Comportamiento v1 de este workstream:** Para enfocarse en colisiones (hits) y simplificar la UI, **PAS ignora `/plate` de forma incondicional** en esta fase: no se llama a `mapOSCPlateToEvent` / `triggerPlateFromOSC` para ningún mensaje `/plate`. Existe un flag (p. ej. `enablePlateSynth`) para poder reactivar PlateSynth más adelante, pero **por defecto está OFF** (PlateSynth deshabilitado).
- **Dónde:** En MainComponent, en `oscMessageReceived`, no invocar la rama `/plate` (o invocarla solo cuando `enablePlateSynth` sea true; por defecto false). Justificación: un solo punto de control, sin tocar el camino de render de SynthesisEngine.
- **UI:** Quitar (u ocultar) controles de Plate (gain/freq/mode/labels) en MainComponent.
- **Documentación:** Actualizar OSC_SCHEMA.md, REVIEW_PACK_PAS.md y ARQUITECTURA.md con una **declaración versionada**: “PAS ignora `/plate` (PlateSynth deshabilitado por defecto en esta versión); ISTR puede seguir enviando `/plate` para futura reactivación.” El esquema de `/plate` **no se elimina**.

**Criterios de aceptación (A3):** Los mensajes `/plate` no afectan la salida de audio (comportamiento por defecto v1). La documentación describe este comportamiento sin eliminar el esquema.

---

## 6) Criterios de aceptación y verificación (generales)

- Con muchas partículas, la “cobertura” pasa a ser representación fusionada: menos descartes por desbordamiento de cola; representación estable por cuadrante.
- Espacialización: pan L/R coherente con la distribución en x (centroide por cuadrante/ventana).
- Colisiones de borde distinguibles de partícula-partícula (más graves/quietas en bordes).
- CPU estable; sin violaciones RT (sin locks, allocs ni logging en audio thread).

**A1 – Acceptance criteria:** Los eventos fusionados se disparan **solo desde el audio thread** (alineados a bloque), nunca desde el message thread. No hay locks, allocs ni logging en el audio thread.

**A2 – Acceptance criteria:** En la documentación y en el plan se usa de forma consistente: impactIntensity / amplitude / power; y se indica explícitamente qué se suma (power) y por qué (aggregation inspirada en física).

**Métrica de cobertura de eventos audibles (Definition of audible-event coverage):**

- **coverage_raw_to_fused** = `fusedHitsEnqueued / hitsReceivedRaw` (ratio de hits raw que pasan a eventos fusionados encolados).
- **queue_loss** = `fusedHitsDiscardedQueue / fusedHitsProduced` (ratio de snapshots fusionados perdidos por cola llena).
- **aggregator_overflow_loss** (opcional) = `rawHitsDiscardedAggregatorOverflow / hitsReceivedRaw` (ratio de hits raw descartados por overflow en el agregador).

En un run “sano” en los escenarios objetivo: **queue_loss** debe estar cerca de 0 (p. ej. menos del 1%); coverage_raw_to_fused refleja la reducción por fusión (varios raw → 1 fused por cuadrante por ventana). Documentar rangos esperados en pruebas de aceptación.

Checklist: hitsReceivedRaw, fusedHitsEnqueued, fusedHitsDiscardedQueue, fusedHitsProduced; coverage_raw_to_fused y queue_loss; comprobación de pan; prueba bordes vs centro; monitoreo de CPU y latencia.

---

## 7) Estrategia de rollback por hito

- Flags de compilación o runtime: `enableAggregation` (bool), `enablePlateSynth` (bool). Por milestone: desactivar agregación y volver a encolar hits crudos; reactivar PlateSynth sin tocar ISTR.

---

## Constants & Conventions (v1)

- **Ventana:** 20 ms. **Cierre de ventana:** Timer en message thread cada 20 ms (o 10 ms con cierre cuando ≥20 ms transcurridos).
- **FUSED_QUEUE_SIZE:** 256 (o 128). Overflow: drop-new, incrementar `fusedHitsDiscardedQueue`.
- **MAX_HITS_PER_QUAD_PER_WINDOW:** definido en sección 3; overflow: seguir sumando power/centroide, count capado (o drop-weakest si se elige).
- **Pan:** v1 opción A — pan (gL/gR) aplicado en SynthesisEngine en la mezcla, no en cada voz.
- **Convención y:** Cuadrante usa `y >= 0.5` sin asumir top/bottom; verificar ISTR (pos.y/height, origen típico top-left en oF) antes de reglas basadas en y.
- **Plate:** PAS ignora `/plate` incondicionalmente en v1 (PlateSynth deshabilitado por defecto; flag `enablePlateSynth` existe para reactivar).

---

## Milestones (Cambios / Archivos y funciones / Riesgos / Pruebas de aceptación / Rollback)

### Milestone 1 – Repo hygiene + docs

- **Cambios:** Commit de cambios ya implementados (envelope Decay→Idle, setGlobalParametersOnly, Clipper, atomics relaxed); actualización de OSC_SCHEMA, REVIEW_PACK_PAS, ARQUITECTURA y Changelog.
- **Archivos/funciones:** Repo (git); OSC_SCHEMA.md, REVIEW_PACK_PAS.md, app-juce/docs-evaluacion/ARQUITECTURA.md.
- **Riesgos:** Ninguno funcional.
- **Pruebas de aceptación:** `git status` limpio; docs referencian Clipper, envelope percusivo, parámetros globales.
- **Rollback:** Revert del commit.

---

### Milestone 2 – PlateSynth bypass + UI + documentación (A3)

- **Cambios:** **PAS ignora `/plate` (PlateSynth deshabilitado por defecto).** En MainComponent no se llama a triggerPlateFromOSC para mensajes `/plate` (o solo cuando el flag `enablePlateSynth` está true; por defecto false). Eliminar controles de Plate de la UI. Actualizar documentación con declaración versionada: “PAS ignora /plate (PlateSynth deshabilitado por defecto en esta versión); ISTR puede seguir enviando /plate para futura reactivación” en OSC_SCHEMA.md, REVIEW_PACK_PAS.md y ARQUITECTURA.md (diagrama de flujo y sección de eventos perdidos/procesados).
- **Archivos/funciones:** MainComponent.cpp (.h): rama `/plate` en oscMessageReceived (omitida o gated por enablePlateSynth); eliminación de controles Plate. OSC_SCHEMA.md, REVIEW_PACK_PAS.md, app-juce/docs-evaluacion/ARQUITECTURA.md.
- **Riesgos:** ISTR sigue enviando /plate; no debe haber regresiones en /hit y /state.
- **Pruebas de aceptación:** /plate no afecta la salida de audio (comportamiento por defecto); docs describen el comportamiento sin eliminar el esquema /plate.
- **Rollback:** Poner `enablePlateSynth` en true y restaurar rama /plate y controles.

---

### Milestone 3 – Agregación cuadrante/ventana (diseño en dos etapas, A1)

- **Cambios:** Message thread: agregar buckets por cuadrante; **cierre de ventana con Timer en message thread cada 20 ms** (v1; determinista y fácil de testear). Al cerrar ventana, producir FusedHitSnapshot por cuadrante (no vacío) y escribir en cola lock-free de capacidad FUSED_QUEUE_SIZE; overflow: drop-new, fusedHitsDiscardedQueue++. Audio thread: en processEventQueue (o equivalente) drenar cola de fused hits y llamar a triggerVoice por cada snapshot. Aplicar pan (gL/gR) en la etapa de mezcla del engine (opción A). Contadores: hitsReceivedRaw, fusedWindowsProduced/fusedHitsProduced, fusedHitsEnqueued, fusedHitsDiscardedQueue, (opcional) rawHitsDiscardedAggregatorOverflow. Definir MAX_HITS_PER_QUAD_PER_WINDOW y comportamiento en overflow (R2). baseFreq determinista para fused (R1). Terminología impactIntensity/amplitude/power (A2). Métricas: coverage_raw_to_fused, queue_loss, (opcional) aggregator_overflow_loss.
- **Archivos/funciones:** MainComponent.cpp: mapOSCHitToEvent → agregadores; **Timer 20 ms** para cierre de ventana; enqueue FusedHitSnapshot. Nuevo módulo/clase agregación (p. ej. HitAggregator). SynthesisEngine: cola de fused events (FUSED_QUEUE_SIZE), drenado en renderNextBlock/processEventQueue, **aplicación de pan en mix**; contadores.
- **Riesgos:** Aumento de uso de CPU en message thread; cola de fused acotada (overflow drop-new).
- **Pruebas de aceptación:** Fused events solo se disparan desde audio thread; sin locks/alloc/logging en audio thread; documentación consistente impactIntensity/amplitude/power; queue_loss cerca de 0 en escenarios objetivo; coverage_raw_to_fused documentado.
- **Rollback:** Flag `enableAggregation`; con false, encolar hits crudos como hasta ahora.

---

### Milestone 4 – Gestión de voces (Option B como primera – R4)

- **Cambios:** Implementar presupuesto de voces por cuadrante (maxVoicesPerQuadrant); evaluar cobertura y distribución L/R; si hace falta, combinar con límite por ventana (A) o aumento de maxVoices (C).
- **Archivos/funciones:** VoiceManager (asignación por cuadrante o prioridad por cuadrante); posiblemente SynthesisEngine para exponer cuadrante en evento.
- **Riesgos:** Subutilización de voces o complejidad extra.
- **Pruebas de aceptación:** Cobertura estable por cuadrante; pan coherente; RT estable.
- **Rollback:** Volver a pool global sin reserva por cuadrante.

---

### Milestone 5 – Ajuste perceptual

- **Cambios:** Centro bias (opcional); afinado bordes vs p-p; curvas brightness/damping vs densidad.
- **Archivos/funciones:** Lógica de derivación de parámetros en agregación y/o SynthesisEngine.
- **Riesgos:** Subjetivo; posible sobresaturación.
- **Pruebas de aceptación:** Bordes más graves/quietos; centro más presente si se aplica bias; CPU estable.
- **Rollback:** Revertir curvas a valores por defecto.

---

## Resumen de cambios en este documento

- **A1:** Diseño en dos etapas: agregación en message thread, FusedHitSnapshot a cola lock-free, **solo audio thread** dispara voces; sección implícita sobre RT-safety y determinismo; criterios de aceptación añadidos.
- **A2:** Uso consistente de **impactIntensity** (no “energy” como energía física); agregación descrita como “inspirada en física”; mapeo explícito (power sum, sqrt); nota sobre Fletcher–Munson; criterios de aceptación de nomenclatura y de “qué se suma y por qué”.
- **A3:** Bypass de PlateSynth en **ingestión** (MainComponent); eliminación de controles Plate; actualización explícita de OSC_SCHEMA, REVIEW_PACK_PAS y ARQUITECTURA; criterios de aceptación para /plate y docs.
- **R1:** baseFreq determinista para eventos fusionados (y, surface), sin aleatoriedad.
- **R2:** MAX_HITS_PER_QUAD_PER_WINDOW y comportamiento en overflow definidos en el plan.
- **R3:** Contadores separados: hitsReceivedRaw, fusedWindowsProduced/fusedHitsProduced, fusedHitsEnqueued, fusedHitsDiscardedQueue, rawHitsDiscardedAggregatorOverflow (opcional).
- **R4:** Opción B (presupuesto por cuadrante) como primera estrategia a evaluar antes de subir maxVoices.

Todos los milestones mantienen: Cambios / Archivos y funciones / Riesgos / Pruebas de aceptación / Rollback.

---

## Plan vNext adjustments (resumen de aclaraciones)

- **Ventana:** Se eligió **Timer en message thread cada 20 ms** como mecanismo de cierre de ventana para v1 (determinista y fácil de testear). El jitter del timer no afecta el momento del disparo audible, que sigue siendo block-aligned en el audio thread.
- **Cola de fused:** Se definió **FUSED_QUEUE_SIZE** (256 o 128) y política de overflow **drop-new** (descartar el snapshot nuevo e incrementar fusedHitsDiscardedQueue); no overwrite por cuadrante en v1.
- **Pan:** v1 usa **opción A**: pan (gL/gR constant-power) aplicado en **SynthesisEngine en la mezcla**, no en cada voz; FusedHitSnapshot incluye pan o x_rep.
- **Convención y:** Cuadrante usa y>=0.5 sin asumir top/bottom; se añadió caja de convención de coordenadas y acción de verificar ISTR (pos.y/height, origen típico top-left) antes de reglas basadas en y.
- **Plate:** Para v1, PAS **ignora /plate incondicionalmente** (PlateSynth deshabilitado por defecto); flag enablePlateSynth existe pero por defecto OFF. Documentación con declaración versionada.
- **Métricas de cobertura:** Definición explícita de coverage_raw_to_fused, queue_loss y (opcional) aggregator_overflow_loss; rangos esperados “sanos” (queue_loss cerca de 0) en escenarios objetivo.
