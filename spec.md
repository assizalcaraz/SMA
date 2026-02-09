# SPEC — Sistema Modular Audiovisual (oF ↔ JUCE)

## 0. Objetivo

Construir un sistema audiovisual en tiempo real dividido en **dos aplicaciones independientes**:

* **App A (openFrameworks / ISTR):** tracking corporal + simulación de partículas + generación de eventos sonoros.
* **App B (JUCE / Standalone):** sintetizador que recibe **OSC y genera el audio (timbre "metálico/partículas").

Las apps se integran mediante un **contrato de mensajes estable** (OSC).

---

## 1. Alcance y restricciones

### 1.1 Requisitos funcionales

* Tracking en vivo (webcam) con MediaPipe (pose y/o manos).
* Sistema de partículas que responde a:

  * posición del usuario (centro del cuerpo o manos)
  * velocidad del gesto
* Las partículas tienden a volver a su “home/origen”.
* Los impactos/colisiones con bordes (o superficies laterales) generan **eventos** (triggers).
* El audio se genera exclusivamente desde JUCE, a partir de los eventos/params.

### 1.2 Requisitos no funcionales

* Tiempo real: 30–60 FPS visual; audio estable sin glitches (buffer típico 256/512).
* Modularidad: oF y JUCE deben correr separados; si uno cae, el otro no crashea.
* Control de densidad de eventos (anti-spam) para evitar ráfagas que saturen el audio.
* Presentable como **dos trabajos independientes**.

### 1.3 Exclusiones explícitas

* No LLM.
* No clasificación de objetos ni segmentación pesada.
* No dependencia de internet.

---

## 2. Arquitectura general

### 2.1 Componentes

**App A — oF (ISTR)**

* Capture: `ofVideoGrabber`
* Tracking: MediaPipe (pose o hands)
* Simulación:

  * Partículas (N)
  * Campo de fuerzas por gesto
  * Retorno a origen
  * Bordes como superficies (colisión)
* Messaging:

  * OSC out (principal)
  * MIDI out (opcional)

**App B — JUCE (PAS1)**

* Input:

  * OSC in (principal)
  * MIDI in (opcional/alternativo)
* Sound Engine:

  * Synth per-voice / modal resonator / bank resonant filters
* Output: audio

### 2.2 Flujo de datos

Webcam → MediaPipe → (puntos y velocidades) → Fuerzas → Partículas → Colisiones → Eventos → OSC → JUCE → Audio

---

## 3. App A (openFrameworks) — Especificación

### 3.1 Inputs de tracking

Elegir **uno** como “mínimo viable” y dejar el otro como opcional.

**Modo 1 (recomendado): Hands**

* `handL.pos` (x,y) normalizado 0..1
* `handR.pos` (x,y) normalizado 0..1
* `handL.vel`, `handR.vel` (px/frame o normalizado por dt)
* `confidence` por mano

**Modo 2: Pose (torso)**

* `torso.pos` (x,y): midpoint hombros o cadera
* `torso.size`: escala aproximada (distancia hombros) → proxy de distancia cámara
* `torso.vel`

**Suavizado obligatorio**

* `pos_smooth = lerp(pos_smooth, pos_raw, α)`
* α recomendado: 0.1–0.25 (depende del jitter)
* `vel` derivada de `pos_smooth` para evitar ruido

### 3.2 Sistema de partículas

#### 3.2.1 Estructura de partícula

Cada partícula `p[i]` tiene:

* `home` (x,y) — posición de reposo
* `pos` (x,y)
* `vel` (x,y)
* `mass` (opcional, default 1)
* `lastHitTime` (para cooldown)
* `id`

#### 3.2.2 Inicialización (home)

Opciones permitidas:

* grid jitter (recommended)
* distribución circular
* ruido Perlin (semillas fijas)

**Recomendación:** grid + jitter para formar “volumen” estable.

#### 3.2.3 Fuerzas (core)

Total:
`F = F_home + F_gesture + F_drag + F_bounds (+ F_noise opcional)`

**a) Retorno al origen (home)**

* Tipo resorte:
  `F_home = k_home * (home - pos)`
* Parámetros:

  * `k_home` ∈ [0.5, 6.0] (escala depende de dt)

**b) Gesto como fuerza (mano/torso)**
Definir una región de influencia radial.

* Para cada efector `e` (mano L/R o torso):

  * `dir = normalize(vel_e)` si `|vel_e|>ε`
  * `speed = clamp(|vel_e|/speed_ref, 0..1)`
  * Influencia por distancia:

    * `w = exp(-(r^2)/(2*sigma^2))` (gauss)
  * Fuerza:

    * **impulso direccional**:
      `F_gesture += k_gesture * w * speed * dir`
    * o **push radial**:
      `F_gesture += k_gesture * w * speed * normalize(pos - e.pos)`

**c) Drag (amortiguación)**
`F_drag = -k_drag * vel`

* `k_drag` ∈ [0.5, 3.0]

**d) Integración**

* Semi-implícita Euler:

  * `vel += (F/mass) * dt`
  * `pos += vel * dt`

> dt en segundos (ofGetLastFrameTime()).

### 3.3 Bordes / superficies y colisiones

Definir superficies:

* Opción A: 4 bordes del frame (rect)
* Opción B: laterales L/R + techo/piso opcional
* Opción C: anillo circular (más abstracto)

**MVP:** bordes rectangulares.

#### 3.3.1 Detección

Si `pos.x < xmin` o `> xmax` o `pos.y < ymin` o `> ymax` → colisión.

#### 3.3.2 Respuesta física

* Rebotar con restitución:

  * `vel.x *= -restitution` (según el borde)
* `restitution` ∈ [0.2, 0.85] (metal = más alto)
* Clampear pos al borde.

#### 3.3.3 Generación de evento sonoro (hit)

**Modelo refinado: "always sound on collision"**

Todas las colisiones producen sonido, incluyendo micro-colisiones ("scrapes"). El sistema mapea continuamente la física del impacto a parámetros sonoros.

**Cálculo de energía (mapeo continuo):**

* Velocidad normalizada:
  * `speed_norm = |v_pre| / vel_ref`
  * `v_pre` = velocidad PRE-colisión (antes de aplicar restitución)
  * `vel_ref` = velocidad de referencia (valor inicial recomendado: 500 px/s)
* Distancia normalizada:
  * `dist_norm = distance_traveled / dist_ref`
  * `distance_traveled` = distancia recorrida desde último hit (o desde último frame si es primer hit)
  * `dist_ref` = distancia de referencia (valor inicial recomendado: 50 px)
* Energía combinada:
  * `energy = clamp(a * speed_norm + b * dist_norm, 0..1)`
  * `a`, `b` = pesos configurables (valores iniciales recomendados: a=0.7, b=0.3)

**Anti-stuck rule (partículas pegadas al borde):**

* Si una partícula permanece en contacto con un borde por múltiples frames:
  * Tratar como "scrape" (roce) con throttling
  * Cooldown por partícula: 30-120ms (configurable)
  * Opcional: partícula debe re-entrar al interior por margen (hysteresis) antes de hits de fuerza completa
  * Scrapes producen eventos de energía muy baja pero audibles

**Validación y rate limiting:**

* Cooldown por partícula:
  * Verificar `timeNow - lastHitTime > hit_cooldown` (30-120ms)
  * Actualizar `lastHitTime` después de hit válido
* **NO usar threshold duro** excepto para ruido numérico extremo (p.ej. energy < 0.001)
* Rate limiting global (token bucket):
  * `tokens += rate * dt` (donde `rate = max_hits_per_second`)
  * `tokens = min(tokens, burst)`
  * Cada hit emitido consume 1 token
  * Si `tokens < 1` → descartar hit
  * Parámetros: `max_hits_per_second` = 200, `burst` = 300, `max_hits_per_frame` = 10

**Parámetros:**

* `vel_ref` ∈ [300, 1000] px/s (valor inicial: 500)
* `dist_ref` ∈ [20, 100] px (valor inicial: 50)
* `a` ∈ [0.5, 0.9] (peso velocidad, valor inicial: 0.7)
* `b` ∈ [0.1, 0.5] (peso distancia, valor inicial: 0.3)
* `hit_cooldown` ∈ [30ms, 120ms] por partícula

### 3.4 Rendering (visual)

Requisitos mínimos:

* Render de partículas (puntos/pequeños sprites)
* Opción “metálico”:

  * puntos blancos/azulados
  * halo leve (additive) opcional
* Fondo oscuro o gradiente suave
* Debug overlay opcional:

  * número de hits/seg
  * dt/fps
  * tracking confidence

### 3.5 Output messaging (OSC)

oF debe enviar:

* eventos de hit
* parámetros continuos opcionales (densidad/energía global)

---

## 4. Contrato OSC (definitivo)

### 4.1 Transporte

* UDP OSC
* Host: `127.0.0.1` (default)
* Port: `9000` (default, configurable)

### 4.2 Mensajes

#### 4.2.1 Evento de impacto

**Address:** `/hit`
**Args:**

1. `int32 id` — id de partícula
2. `float x` — 0..1 (posición normalizada en pantalla)
3. `float y` — 0..1
4. `float energy` — 0..1
5. `int32 surface` — enum superficie (0=L,1=R,2=T,3=B) o -1 si N/A

> Este es el mensaje principal. Con esto ya suena todo.

#### 4.2.2 Estado global opcional

**Address:** `/state`
**Args:**

1. `float activity` — 0..1 (hits por segundo normalizado)
2. `float gesture` — 0..1 (energía de gesto agregada)
3. `float presence` — 0..1 (confianza tracking)

#### 4.2.3 Control remoto opcional (de JUCE hacia oF)

**Address:** `/ctrl`
**Args:**

* `string key`, `float value`
  Ej: `("k_home", 2.5)`

*MVP: no hace falta implementarlo.*

---

## 5. App B (JUCE) — Especificación

### 5.1 Formato de entrega

**Decisión:** **Standalone JUCE = entrega principal. Plugin (VST3/AU) = bonus opcional.**

* **Standalone JUCE** (entrega principal, más fácil para demo)
* **Plugin (VST3/AU) + app host** (bonus opcional, más "pro", más fricción)

### 5.2 Input OSC

* Listener en puerto `9000` (configurable)
* Parse `/hit` y `/state`

### 5.3 Motor sonoro (timbre metálico)

Dos diseños posibles. Elegí 1.

#### Opción A (recomendada): Resonador modal simple

Cada hit excita un resonador con modos inarmónicos.

Parámetros por evento:

* `energy` → amplitude + brightness
* `x` → pan (-1..1)
* `y` → base frequency o damping

Estructura de voz:

* excitación: impulso corto (click / noise burst 5–20ms)
* filtros/resonadores: 3–8 reson bandpass
* sum → saturación suave opcional → salida

#### Opción B: Bank de resonant filters global (sin voces)

* cada hit suma excitación a un buffer
* el banco resuena
* menos control por partícula, más “masa”

**Para modularidad y claridad académica:** Opción A.

### 5.4 Asignación de voces (polyphony)

* `maxVoices`: 16–64 (según CPU)
* Voice stealing:

  * robar la voz con menor amplitud residual o más antigua
* Cooldown por partícula puede reducir densidad, pero igual manejar saturación.

### 5.5 Mapeos (contract → sonido)

**Diseño sonoro objetivo: "Coin Cascade" (cascada de monedas)**

Textura de múltiples hits percusivos metálicos breves que se acumulan en una cascada granular. Cada colisión produce sonido, desde micro-scrapes (casi imperceptibles) hasta golpes fuertes.

#### `/hit(id, x, y, energy, surface)`

* `amp = energy^γ` (γ 1.2–2.0 para mejor dinámica)
  * Micro-colisiones (energy muy baja) → amplitud muy baja pero audible (no mute)
  * Golpes fuertes (energy alta) → amplitud alta
* `pan = (x * 2 - 1)` (-1 = izquierda, 1 = derecha)
  * Distribución espacial de la cascada
* `brightness = lerp(minBright, maxBright, energy)`
  * Colisiones más energéticas = más brillantes
* `damping = lerp(short, long, 1 - y)` (arriba más seco o al revés)
  * Partes superiores = más secas, inferiores = más reverberantes
* `baseFreq`:
  * opción 1: fijo (p.ej. 220Hz) + microdesviación por y
  * opción 2: escala discreta por bandas de y (más musical, recomendado para "cascade")
* `surface` → modulación opcional (diferentes timbres por superficie)

#### `/state(activity, gesture, presence)` (opcional)

* `globalWet = activity`
* `globalDrive = gesture`
* `master = presence` (si presence cae, bajar nivel)

### 5.6 Anti-saturación / seguridad

* **Voice stealing strategy:**
  * Robar voz más antigua O voz con menor amplitud residual
  * Priorizar mantener textura "cascade" sin gaps
* **Master limiter:**
  * Limiter suave en salida master
  * Prevenir clipping bajo carga alta (200 hits/s)
* **Rate limiting (en oF):**
  * Token bucket global (ver sección 3.3.3)
  * Hard cap de eventos por segundo (200 hits/s máximo)

---

## 6. Calibración y parámetros expuestos

### 6.1 Parámetros en oF (UI simple)

* `N_particles` (500–8000)
* `k_home`
* `k_gesture`
* `sigma` (radio influencia)
* `k_drag`
* `restitution`
* `hit_threshold`
* `hit_cooldown_ms`

### 6.2 Parámetros en JUCE (UI simple)

* `Voices`
* `Metalness` (inharmonic spread)
* `Brightness`
* `Damping`
* `Drive`
* `ReverbMix` (opcional)
* `Limiter on/off`

---

## 7. Modos de demo

### 7.1 Demo A (interacción básica)

* usuario quieto → partículas en home, casi sin hits
* gesto de manos → desplazamiento + hits en bordes → sonido metálico

### 7.2 Demo B (instrumento)

* jugar con velocidad del gesto:

  * lento: pocos hits, sonido espacioso
  * rápido: muchos hits, textura granular metálica

### 7.3 Demo C (sin webcam)

* fallback: mouse como efector (para presentar si falla MediaPipe)

  * mouse pos y vel emulan gesto

*Esto te salva la nota si la webcam rompe.*

---

## 8. Testing mínimo

### 8.1 oF

* Test de estabilidad FPS con N=2000/5000
* Contador de hits/seg
* Validación de cooldown (no > X hits por partícula por segundo)

### 8.2 JUCE

* Test de estrés: recibir 200 hits/s sin glitch
* Test de voice stealing
* Test de limiter

### 8.3 Integración

* Script (opcional) que emita OSC `/hit` falso para probar JUCE sin oF.

---

## 9. Entregables (lo que se presenta)

### 9.1 ISTR

* Código oF + demo video
* explicación: tracking → fuerzas → partículas → eventos
* diagrama de pipeline

### 9.2 JUCE

* standalone o plugin + preset “metal particles”
* explicación: OSC → voices → resonador → master limiter

### 9.3 Bonus (si da el tiempo)

* captura integrada (pantalla + audio) como performance de 60–90s

---

## 10. Roadmap (orden recomendado)

1. oF: partículas + retorno + gesto (sin OSC)
2. oF: colisiones + eventos (log)
3. oF: OSC `/hit`
4. JUCE: synth simple con MIDI interno (para probar timbre)
5. JUCE: receptor OSC y mapping
6. calibración conjunta
7. demo + fallback mouse

---
