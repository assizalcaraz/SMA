# Plan de integración ISTR–PAS y workflow Git (SMA)

Documento único de referencia para: (0) estado del repo y corrección de ramas, (1) commit(s) descriptivo(s) de código ISTR, (2) actualización de documentación, (3) commit solo documentación, (4) continuación de mejoras PAS (solo plan, sin código).

---

## 0) Pre-vuelo: estado del repo e higiene de ramas

### 0.1 Verificar estado actual

Ejecutar en la raíz del repo:

```bash
git status
git branch --show-current
git log -5 --oneline
```

**Estado observado (Feb 2026):**

- **Rama actual:** `entrega-pas-1`
- **Ramas existentes:** `main`, `master`, `synth`, `v0.4`, `entrega-istr-pas1`, `entrega-pas-1`
- **Cambios sin commit:** modificados `.DS_Store`, `*.xcuserstate`, binarios en `Particles/bin/` y `app-juce/.../build/`; sin trackear: `ISTR_AUDIT_PACK.md`, `app-juce/docs-evaluacion/`, `entrega-evaluacion-externa-pas1/`

**Diagnóstico:** Los cambios ISTR recientes (rest gate, energy, counters, token bucket, budgeted selection, quadrant fairness, target=500, overlay) están en la rama **entrega-pas-1**, que es una rama de entrega PAS. Es decir, **el trabajo ISTR está en la rama equivocada** (debería vivir en una rama tipo `istr-improvements`).

Commits ISTR solo en Particles (para cherry-pick), en orden cronológico ascendente:

- `fc808d9` — fix: correct hits_sent_osc counter placement and overlay timebase
- `92ad1bc` — fix: token bucket refill/consume dt units + p2p tokens not starving
- `8cefbcb` — perf: budgeted top-energy selection before rate limiter
- `9a50824` — chore: add discarded_by_budget counters to overlay + docs
- `441fcd2` — tune: lower default target_hits_per_second to 500
- `7d05ce7` — perf: quadrant-budgeted selection for spatial fairness + overlay counters

(El commit `a513b0f` es solo docs en `Docs/Particles/readme.md`; puede ir en istr-improvements como doc asociada al código ISTR.)

### 0.2 Estrategia elegida: A — Cherry-pick a istr-improvements

**Motivo:** En `entrega-pas-1` hay commits mezclados ISTR (Particles) y PAS (app-juce). La opción más segura es no reescribir historia de la rama de entrega y crear una rama nueva `istr-improvements` desde `main` trayendo solo los commits ISTR por cherry-pick. La rama PAS queda intacta.

**Comandos exactos:**

```bash
# 1) Crear rama istr-improvements desde main
git checkout main
git pull origin main   # si usas remoto
git checkout -b istr-improvements

# 2) Cherry-pick de los commits ISTR (orden: del más antiguo al más reciente)
git cherry-pick fc808d9   # fix: hits_sent_osc + overlay timebase
git cherry-pick 92ad1bc   # fix: token bucket dt + p2p tokens
git cherry-pick 8cefbcb   # perf: budgeted selection
git cherry-pick 9a50824   # chore: discarded_by_budget overlay + docs
git cherry-pick 441fcd2   # tune: target_hits_per_second 500
git cherry-pick 7d05ce7   # perf: quadrant-budgeted selection + overlay
git cherry-pick a513b0f   # docs: readme presupuesto y fairness (opcional, solo docs)
```

Si en algún cherry-pick hay conflicto: resolver en los archivos indicados, `git add` y `git cherry-pick --continue`. Tras terminar, verificar build y ejecución de Particles.

```bash
# 3) Verificación
cd Particles && make clean && make && make run   # o abrir Xcode y build/run
```

Dejar la rama `entrega-pas-1` sin modificar (sigue con todo el historial ISTR+PAS).

### 0.3 Ramas objetivo

| Rama | Uso |
|------|-----|
| **main** | Estable; no hacer commits directos de features. |
| **istr-improvements** | openFrameworks Particles (ISTR): rest gate, energy, counters, budgeting, quadrant fairness, target=500. Crear desde main + cherry-picks anteriores. |
| **pas-improvements** | JUCE PAS-1-SYNTH. Usar `entrega-pas-1` como base o crear `pas-improvements` desde main y cherry-pick solo commits PAS (p. ej. `0d35fe3`, `dae5d12`) si se quiere historial limpio por módulo. |

---

## 1) Commit de código (descriptivo) — Finalización ISTR

Objetivo: dejar el estado ISTR estable en commits pequeños y legibles. Si en `istr-improvements` ya aplicaste los cherry-picks uno a uno, los mensajes ya son descriptivos. Si prefieres agrupar en 2–3 commits (squash), puedes hacerlo en una rama temporal y luego un solo merge a `istr-improvements`.

### 1.1 Working tree limpio (excluir builds y user state)

Añadir o confirmar en `.gitignore`:

- `Particles/bin/`
- `*.app` (o mantener `bin/*.app/` si ya cubre todos los binarios)
- `build/` (ya está)
- `*.xcuserdata` / `**/xcuserdata/` (ya hay entradas Xcode)
- `.DS_Store` (ya está)

Si algún archivo de build/bin ya estaba trackeado, dejar de trackearlo sin borrarlo en disco:

```bash
git rm --cached Particles/bin/ParticlesDebug.app/Contents/Info.plist
git rm --cached Particles/bin/ParticlesDebug.app/Contents/MacOS/ParticlesDebug
# y análogo para app-juce/.../build/... si aparecen en git status
```

### 1.2 Agrupar commits (opcional)

- **Opción A:** Mantener los 6–7 commits del cherry-pick (cada uno con prefijo fix/perf/tune/chore/docs).
- **Opción B:** Tras cherry-pick, hacer squash en 2–3 commits con mensajes:
  - **Commit 1:** `fix: ISTR — rest hits eliminados, energy weighting, counters y timebase` — Incluye: rest gate (velocidad normal), energía (dist_norm * speed_norm), contadores candidate/pending/validated/sent y colocación correcta de hits_sent_osc, timebase del overlay.
  - **Commit 2:** `perf: ISTR — budgeted selection, quadrant fairness, overlay background` — Incluye: presupuesto por frame (top-energy), fairness por cuadrante, contador discarded_by_budget, fondo semi-opaco del overlay.
  - **Commit 3:** `tune: ISTR — default target_hits_per_second=500`

### 1.3 Mensajes de commit (convencional)

Prefijos: `fix:`, `perf:`, `tune:`, `chore:`, `docs:`.

Cada mensaje debe indicar **módulo (ISTR/Particles)** y **resultado**:

- Rest hits eliminados (rest gate), energy con ponderación correcta, contadores y timebase.
- Budgeted selection, quadrant fairness, overlay con fondo legible, rate limiter como red de seguridad (drops ~0 esperados).

---

## 2) Actualizar toda la documentación relacionada (solo cambios en working tree)

Objetivo: que la documentación refleje el comportamiento real de ISTR y la relación ISTR–PAS.

### 2.1 Documentos a tocar (lista verificada en repo)

| Documento | Acción |
|-----------|--------|
| Docs/Particles/readme.md | Añadir sección "Cambios (Feb 2026)" con: target=500 por defecto, selección por cuadrante, rate limiter como red de seguridad, criterios de aceptación con overlay. |
| ISTR_AUDIT_PACK.md | Añadir sección "Cambios (Feb 2026)" o "Latest": rest gate, energy, contadores, budgeting (target_hits_per_second=500), quadrant fairness, rate limiter safety net. Incluir "Cómo verificar" con overlay y criterios. |
| OSC_SCHEMA.md | Añadir nota (schema sin cambios): PAS recibe menos mensajes /hit y de mayor calidad; ISTR estable con target=500 y fairness por cuadrante. |
| app-juce/docs-evaluacion/ARQUITECTURA.md | Indicar que ISTR ahora usa presupuesto + quadrant fairness; PAS recibe menos hits, mismo esquema. Opcional: "Configuración ISTR recomendada (congelada)" mientras se mejoran PAS. |
| REVIEW_PACK_PAS.md | En la parte que describe pérdida de eventos (ISTR): mencionar presupuesto por frame, target_hits_per_second=500, quadrant fairness; rate limiter como red de seguridad; "PAS recibe menos, hits de mayor calidad". |

### 2.2 Contenido mínimo en documentación ISTR

- **Comportamiento final ISTR:** Rest gate (componente normal de velocidad), energía `dist_norm * speed_norm`, contadores (candidate, pending, validated, sent, discarded_by_budget), budgeting con `target_hits_per_second=500`, selección con fairness por cuadrante, rate limiter como red de seguridad (drops cercanos a 0 en uso normal).
- **Cómo verificar:** Uso del overlay (líneas de contadores, tokens, discarded_by_budget, sent_osc) y criterios: reposo → candidate/sent ~0; alta densidad → dropped_by_rate_limiter bajo vs sent_osc, discarded_by_budget alto aceptable, sent_osc acotado por target.

### 2.3 Cross-módulo y versionado

- Dejar explícito: **esquema OSC sin cambios**; PAS recibe menos hits y de mayor calidad.
- Añadir en un solo lugar la **configuración ISTR recomendada "congelada"** mientras se mejoran PAS (target=500, quadrant fairness, resto de defaults documentados).
- Añadir sección "Cambios (Feb 2026)" o "Latest" con bullets; no eliminar información antigua; marcar como "comportamiento anterior" si aplica.

---

## 3) Commit solo documentación

- Añadir **solo** los archivos de documentación modificados: `Docs/Particles/readme.md`, `ISTR_AUDIT_PACK.md`, `OSC_SCHEMA.md`, `app-juce/docs-evaluacion/ARQUITECTURA.md`, `REVIEW_PACK_PAS.md`.
- **No** incluir código ni binarios.
- Mensaje de commit sugerido:

```
docs: update ISTR audit, runtime verification, and SMA pipeline notes

- ISTR: rest gate, energy formula, counters, budgeting (target=500), quadrant fairness, rate limiter as safety net; verification via overlay.
- Cross-module: OSC schema unchanged; PAS receives fewer, higher-quality hits; recommended frozen ISTR settings while PAS is improved.
```

---

## 4) Continuar mejoras PAS (solo plan, sin código)

Objetivo: definir los siguientes pasos de PAS asumiendo que ISTR ya está estable (target=500, quadrant fairness).

### 4.1 Rama y estado PAS

- Crear o usar rama **pas-improvements**. Opción 1: rama desde `main` y cherry-pick solo commits PAS (p. ej. `0d35fe3`, `dae5d12`). Opción 2: usar `entrega-pas-1` como `pas-improvements` (ya incluye Clipper rename, setGlobalParametersOnly, atomics, agregación, etc.).
- Confirmar qué cambios PAS ya existen: Clipper en salida, `setGlobalParametersOnly` RT-safe, atomics relaxed, agregación 20 ms / 4 cuadrantes, UI compacta, etc.

### 4.2 Próximos hitos PAS (solo plan)

- **PAS-1 — PlateSynth bypass:** Audio de placa desactivado, quitar controles de placa de la UI, documentar que `/plate` se ignora.
- **PAS-2 — Fusion aggregator 20 ms / 4 cuadrantes:** Agregación en thread de mensajes, disparo en audio thread; ventana 20 ms y hasta 4 eventos fusionados por cuadrante.
- **PAS-3 — Dos clases de sonido:** Base p2p; "border" con -3 dB y -3 semitonos; eliminar pitch aleatorio.
- **PAS-4 — Política de voces:** Revisar tras fusion; valorar subir maxVoices si el CPU lo permite.
- **PAS-5 — Limpieza UI:** Quitar SubOsc Mix si no se usa; valorar quitar Brightness/Damping de la UI si se confirma que son inaudibles.

### 4.3 Documentación PAS (a hacer después de cada hito)

- Actualizar REVIEW_PACK_PAS.md y app-juce/docs-evaluacion/ARQUITECTURA.md tras cada milestone PAS (bypass, aggregator, dos clases de sonido, voces, UI).

---

## Resumen de orden sugerido

1. **Pre-vuelo:** `git status`, `git branch --show-current`, `git log -5`; decidir si aplicar corrección de ramas (Strategy A con comandos de la sección 0.2).
2. **Rama istr-improvements:** Crear desde main y cherry-pick commits ISTR; verificar build/run de Particles; opcionalmente squash en 2–3 commits (sección 1).
3. **.gitignore y limpieza:** Añadir/confirmar exclusiones; `git rm --cached` para binarios ya trackeados (sección 1.1).
4. **Documentación:** Editar los archivos listados en 2.1 con el contenido de 2.2 y 2.3.
5. **Commit solo docs:** Stage solo docs; commit con mensaje de la sección 3.
6. **PAS:** Cambiar a pas-improvements (o entrega-pas-1); seguir hitos PAS-1 a PAS-5 y actualizar REVIEW_PACK_PAS y ARQUITECTURA tras cada uno.

No se incluye código DSP ni cambios en síntesis en este plan; solo planificación, Git y documentación.

---

## Nota: Reconciliación con main/master (diferida)

main/master están desactualizados; fusionar ahora reintroduciría regresiones (p. ej. dependencia obligatoria de app-calib, que se dejó opcional). El trabajo PAS (M2 fusion, etc.) continúa desde **pas-improvements** en ramas como **pas-m2-fusion**, sin forzar merge a main/master. Plan de reconciliación posterior: (1) fusionar **istr-improvements** en main/master primero (sin cambios PAS); (2) luego merge o cherry-pick de **pas-m2-fusion** sobre eso, resolviendo conflictos una vez y minimizando regresiones.
