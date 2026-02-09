# Plan de Implementación — Sistema Modular Audiovisual

Plan detallado de desarrollo basado en la especificación técnica (`spec.md` en este mismo directorio) y el roadmap recomendado.

**Estrategia de implementación:** Priorizar el loop modular completo (partículas → colisiones → OSC → JUCE) antes de integrar MediaPipe. MediaPipe se trata como "swap de input" (mouse primero, MediaPipe después).

---

## Estructura del proyecto

```
Sistema Modular Audiovisual/
├── app-of/              # App A: openFrameworks
│   ├── src/            # Código fuente
│   ├── addons.make     # Configuración de addons
│   └── config/         # Archivos de configuración
├── app-juce/           # App B: JUCE
│   ├── Source/         # Código fuente
│   └── JuceLibraryCode/ # Librerías JUCE
├── docs/               # Documentación
├── Docs/               # Documentación completa
│   ├── specs/          # Especificaciones técnicas
│   │   ├── spec.md     # Especificación técnica completa
│   │   └── PLAN_IMPLEMENTACION.md  # Este archivo
│   └── ...
├── scripts/            # Scripts de utilidad
│   └── test-osc.py     # Script para probar OSC sin oF
└── tests/              # Tests de integración
```

---

## Fase 1: Setup inicial del proyecto

**Objetivo:** Establecer la estructura base y configuración inicial.

### Tareas

- [x] Crear estructura de carpetas del proyecto
- [ ] Configurar repositorio Git (si aplica)
- [ ] Crear archivos de configuración base:
  - [ ] `app-of/addons.make` — lista de addons necesarios
  - [ ] `app-of/config/settings.xml` — configuración de parámetros
  - [ ] `app-juce/` — proyecto base de JUCE Standalone (crear en Projucer)
- [ ] Documentar dependencias y versiones requeridas

### Entregables

- Estructura de carpetas completa
- Archivos de configuración base
- Lista de dependencias documentada

### Definition of Done (DoD)

- [ ] Proyecto compila sin errores
- [ ] Estructura de carpetas verificada
- [ ] Dependencias documentadas con versiones

---

## Fase 2: App A — Sistema de partículas básico (oF)

**Objetivo:** Implementar el sistema de partículas con física básica (sin tracking ni OSC).

**Referencia:** `spec.md` sección 3.2

### Tareas

#### 2.1 Setup de proyecto openFrameworks

- [ ] Crear proyecto base en `app-of/`
- [ ] Configurar addons necesarios en `addons.make`:
  - [ ] `ofxOsc` (para comunicación OSC, Fase 5)
  - [ ] `ofxMediaPipe` o alternativa (para tracking, Fase 3, opcional inicialmente)
- [ ] Configurar captura de video básica (`ofVideoGrabber`) — opcional para esta fase
- [ ] Verificar compilación y ejecución básica

#### 2.2 Implementar estructura de partícula

- [ ] Crear clase `Particle` con propiedades:
  - [ ] `home` (x, y) — posición de reposo
  - [ ] `pos` (x, y) — posición actual
  - [ ] `vel` (x, y) — velocidad
  - [ ] `mass` (float, default 1.0)
  - [ ] `lastHitTime` (float, para cooldown)
  - [ ] `id` (int, identificador único)
- [ ] Implementar inicialización con grid + jitter
  - [ ] Calcular grid según número de partículas
  - [ ] Aplicar jitter aleatorio a cada posición home
  - [ ] Asignar IDs únicos

#### 2.3 Implementar sistema de fuerzas

- [ ] Fuerza de retorno al origen (`F_home`):
  - [ ] `F_home = k_home * (home - pos)`
  - [ ] Parámetro `k_home` configurable (rango 0.5 - 6.0)
- [ ] Fuerza de drag (`F_drag`):
  - [ ] `F_drag = -k_drag * vel`
  - [ ] Parámetro `k_drag` configurable (rango 0.5 - 3.0)
- [ ] Integración semi-implícita Euler:
  - [ ] `vel += (F/mass) * dt`
  - [ ] `pos += vel * dt`
  - [ ] Usar `ofGetLastFrameTime()` para `dt`
- [ ] Sistema de parámetros ajustables:
  - [ ] UI simple (ofxGui o similar) para ajustar:
    - [ ] `N_particles` (500-8000) — **ajustable en tiempo real**
    - [ ] `k_home`
    - [ ] `k_drag`
- [ ] **UI para cambiar N_particles en vivo:**
  - [ ] Slider o control para cambiar número de partículas en runtime
  - [ ] Reinicializar arrays de partículas de forma segura sin crashear
  - [ ] Preservar forma de distribución (grid+jitter) al redimensionar
  - [ ] Verificar que FPS se mantiene aceptable después del cambio

#### 2.4 Rendering básico

- [ ] Render de partículas como puntos
- [ ] Estilo "metálico":
  - [ ] Puntos blancos/azulados
  - [ ] Halo leve (additive) opcional
- [ ] Fondo oscuro o gradiente suave
- [ ] Debug overlay:
  - [ ] FPS/dt
  - [ ] Número de partículas

#### 2.5 Testing básico

- [ ] Verificar estabilidad con N=500 partículas
- [ ] Verificar estabilidad con N=2000 partículas
- [ ] Verificar estabilidad con N=5000 partículas
- [ ] Medir FPS y rendimiento
- [ ] Verificar que partículas retornen a home correctamente

### Entregables

- Sistema de partículas funcionando
- Partículas retornan a posición home
- UI básica para ajustar parámetros
- Rendering visual funcional

### Definition of Done (DoD)

- [ ] FPS ≥ 45 fps con N=2000 partículas en máquina de desarrollo
- [ ] FPS aceptable (≥ 30 fps) con al menos un valor N más alto (p.ej. N=5000)
- [ ] UI permite ajustar `N_particles` en tiempo real sin crashear
- [ ] Partículas retornan a home visiblemente (sin drift)
- [ ] UI permite ajustar parámetros en tiempo real
- [ ] Sin crashes durante 5 minutos de ejecución continua

---

## Fase 3: App A — Input básico (Mouse como efector)

**Objetivo:** Implementar input básico con mouse para validar el loop modular completo antes de MediaPipe.

**Nota estratégica:** Esta fase permite validar la arquitectura completa (input → fuerzas → partículas → colisiones → OSC) sin depender de MediaPipe. MediaPipe se integra después (Fase 3b) cuando el sistema ya funciona end-to-end.

**Referencia:** `spec.md` sección 3.1 (modo fallback)

### Tareas

#### 3.1 Implementar input con mouse

- [ ] Detectar posición del mouse normalizada (0..1):
  - [ ] `mouse.pos` (x, y) desde `ofGetMouseX()`, `ofGetMouseY()`
  - [ ] Normalizar según tamaño de ventana
- [ ] Calcular velocidad del mouse:
  - [ ] `mouse.vel = (mouse.pos - mouse.pos_prev) / dt`
  - [ ] Suavizado opcional con lerp (α = 0.1-0.25)
- [ ] Implementar mouse como efector formal:
  - [ ] Tratar mouse como "efector único"
  - [ ] Misma interfaz que tendrá MediaPipe después

#### 3.2 Integrar mouse con sistema de partículas

- [ ] Fuerza de gesto (`F_gesture`) con influencia radial gaussiana:
  - [ ] Para el efector mouse:
    - [ ] Calcular dirección: `dir = normalize(vel_mouse)` si `|vel_mouse| > ε`
    - [ ] Calcular velocidad normalizada: `speed = clamp(|vel_mouse|/speed_ref, 0..1)`
    - [ ] Calcular influencia por distancia: `w = exp(-(r^2)/(2*sigma^2))`
    - [ ] Aplicar fuerza: `F_gesture += k_gesture * w * speed * dir`
- [ ] Parámetros configurables:
  - [ ] `k_gesture` (fuerza del gesto)
  - [ ] `sigma` (radio de influencia)
  - [ ] `speed_ref` (velocidad de referencia)

#### 3.3 Testing

- [ ] Verificar respuesta a movimiento del mouse
- [ ] Ajustar parámetros de influencia (`sigma`, `k_gesture`)
- [ ] Verificar que partículas responden suavemente
- [ ] Medir impacto en FPS

### Entregables

- Input con mouse funcionando
- Mouse afecta partículas correctamente
- Sistema listo para integrar colisiones y OSC

### Definition of Done (DoD)

- [ ] Movimiento del mouse afecta partículas visiblemente
- [ ] Respuesta suave sin jitter
- [ ] FPS se mantiene ≥ 45 fps con input activo

---

## Fase 4: App A — Colisiones y eventos

**Objetivo:** Implementar detección de colisiones con bordes, generación de eventos y rate limiting global.

**Referencia:** `spec.md` sección 3.3

### Tareas

#### 4.1 Implementar detección de colisiones

- [ ] Bordes rectangulares (MVP):
  - [ ] Definir límites: `xmin`, `xmax`, `ymin`, `ymax`
  - [ ] Detectar colisión: `pos.x < xmin` o `> xmax` o `pos.y < ymin` o `> ymax`
  - [ ] Identificar superficie impactada (0=L, 1=R, 2=T, 3=B)

#### 4.2 Implementar respuesta física

- [ ] Rebotar con restitución:
  - [ ] `vel.x *= -restitution` (según borde horizontal)
  - [ ] `vel.y *= -restitution` (según borde vertical)
  - [ ] Parámetro `restitution` configurable (rango 0.2 - 0.85)
- [ ] Clampear posición al borde:
  - [ ] `pos.x = clamp(pos.x, xmin, xmax)`
  - [ ] `pos.y = clamp(pos.y, ymin, ymax)`

#### 4.3 Generación de eventos de hit

- [ ] **Cálculo de energía del impacto (mapeo continuo):**
  - [ ] Velocidad normalizada: `speed_norm = |v_pre| / vel_ref`
    - [ ] `v_pre` = velocidad PRE-colisión (antes de aplicar restitución)
    - [ ] `vel_ref` = velocidad de referencia configurable (valor inicial recomendado: 500 px/s)
  - [ ] Distancia normalizada: `dist_norm = distance_traveled / dist_ref`
    - [ ] `distance_traveled` = distancia recorrida desde último hit (o desde último frame si es primer hit)
    - [ ] `dist_ref` = distancia de referencia configurable (valor inicial recomendado: 50 px)
  - [ ] Energía combinada: `energy = clamp(a * speed_norm + b * dist_norm, 0..1)`
    - [ ] `a` y `b` son pesos configurables (valores iniciales recomendados: a=0.7, b=0.3) — **tuneables por escena/estética**
    - [ ] `vel_ref` y `dist_ref` también son tuneables por escena/estética
    - [ ] **Todas las colisiones producen sonido**, incluso micro-colisiones (energy muy baja → amplitud muy baja, no mute)
- [ ] **Anti-stuck rule (partículas pegadas al borde):**
  - [ ] Detectar si partícula permanece en contacto con borde por múltiples frames
  - [ ] Tratar como "scrape" (roce) y throttling por partícula:
    - [ ] Cooldown por partícula: 30-120ms (configurable)
    - [ ] Opcional: partícula debe re-entrar al interior por margen (hysteresis) antes de hits de fuerza completa
    - [ ] **Importante:** Scrapes ≠ hits fuertes; comparten el mismo mensaje `/hit` con energía muy baja (no hay mensaje OSC separado)
    - [ ] Scrapes producen eventos de energía muy baja pero audibles
- [ ] **Cooldown por partícula:**
  - [ ] Verificar `timeNow - lastHitTime > hit_cooldown` (30-120ms)
  - [ ] Actualizar `lastHitTime` después de hit válido
  - [ ] **NO usar threshold duro** excepto para ruido numérico extremo (p.ej. energy < 0.001)
- [ ] **Rate limiting global (token bucket preciso):**
  - [ ] Algoritmo token bucket:
    - [ ] `tokens += rate * dt` (donde `rate = max_hits_per_second`)
    - [ ] `tokens = min(tokens, burst)` (limitar burst)
    - [ ] Cada hit emitido consume 1 token
    - [ ] Si `tokens < 1` → descartar hit (evitar saturar audio)
  - [ ] Parámetros configurables:
    - [ ] `max_hits_per_second` (rate) = 200 (default)
    - [ ] `burst` = 300 (default, permite ráfagas cortas)
    - [ ] `max_hits_per_frame` = 10 (límite adicional por frame)
- [ ] Logging de eventos (para debugging):
  - [ ] Contador de hits por segundo
  - [ ] Contador de hits descartados por rate limiting
  - [ ] Contador de hits descartados por cooldown
  - [ ] Log de eventos en consola (opcional, desactivable)

#### 4.4 Especificar formato final de `/hit` (CONTRATO CONGELADO)

- [ ] Documentar contrato definitivo (congelado en esta fase):
  - [ ] **Address:** `/hit`
  - [ ] **Args:**
    1. `int32 id` — ID único de partícula
    2. `float x` (0..1) — **Posición X normalizada de IMPACTO** (no posición del efector)
    3. `float y` (0..1) — **Posición Y normalizada de IMPACTO** (no posición del efector)
    4. `float energy` (0..1) — Energía del impacto (mapeo continuo: `a*speed_norm + b*dist_norm`)
    5. `int32 surface` — Enum superficie: 0=L, 1=R, 2=T, 3=B, -1=N/A
  - [ ] **Especificar explícitamente:**
    - [ ] `x, y` son posición de impacto (donde ocurre la colisión), NO posición del efector
    - [ ] `energy` se basa en velocidad PRE-colisión y distancia recorrida
    - [ ] Todos los valores deben estar en rango especificado
- [ ] Validar que formato sea consistente para JUCE
- [ ] Documentar en `docs/api-osc.md` con este contrato exacto

#### 4.5 Testing

- [ ] Verificar cooldown funciona correctamente
- [ ] Verificar rate limiting global funciona
- [ ] Contador de hits/segundo en debug overlay
- [ ] Validar que no haya spam de eventos
- [ ] Verificar que partículas reboten correctamente
- [ ] Probar con diferentes valores de `restitution`

### Entregables

- Colisiones detectadas correctamente
- Partículas rebotan en bordes
- Eventos de hit generados con validación
- Sistema de cooldown funcionando
- Rate limiting global implementado
- Formato de `/hit` especificado y documentado
- Debug overlay con información de hits

### Definition of Done (DoD)

- [ ] Hits por segundo clamp < 200 (rate limiting activo)
- [ ] Cooldown por partícula funciona (no > 1 hit por partícula cada 30ms)
- [ ] Partículas rebotan correctamente en todos los bordes
- [ ] Debug overlay muestra hits/seg y hits descartados

---

## Fase 5: App A — Comunicación OSC (ADELANTADA)

**Objetivo:** Implementar envío de mensajes OSC a App B. **Esta fase se adelanta para validar el loop modular completo antes de MediaPipe.**

**Referencia:** `spec.md` sección 3.5 y 4

### Tareas

#### 5.1 Integrar ofxOsc

- [ ] Configurar cliente OSC:
  - [ ] Inicializar `ofxOscSender`
  - [ ] Configurar host destino (`127.0.0.1` por defecto)
  - [ ] Configurar puerto (`9000` por defecto)
  - [ ] Hacer host y puerto configurables

#### 5.2 Implementar envío de mensajes

- [ ] Mensaje `/hit` con todos los parámetros requeridos (formato especificado en Fase 4):
  - [ ] `int32 id` — ID de partícula
  - [ ] `float x` — Posición X normalizada (0..1)
  - [ ] `float y` — Posición Y normalizada (0..1)
  - [ ] `float energy` — Energía (0..1)
  - [ ] `int32 surface` — Superficie (0=L, 1=R, 2=T, 3=B, -1=N/A)
- [ ] Mensaje `/state` opcional (actividad global):
  - [ ] Calcular `activity` (hits por segundo normalizado)
  - [ ] Calcular `gesture` (energía de gesto agregada) — usar 0.5 si es mouse
  - [ ] Calcular `presence` (confianza tracking) — usar 1.0 si es mouse
  - [ ] Enviar periódicamente (10 Hz durante actividad)
- [ ] Validación de formato de mensajes:
  - [ ] Verificar que todos los valores estén en rango
  - [ ] Manejo de errores de envío

#### 5.3 Testing de comunicación

- [ ] Verificar mensajes OSC con herramienta de debugging:
  - [ ] OSCulator (macOS) o equivalente
  - [ ] Verificar formato correcto de mensajes
  - [ ] Verificar que lleguen al puerto correcto
- [ ] Usar script de prueba (`scripts/test-osc.py`) para validar formato
- [ ] Medir latencia de envío

### Entregables

- Comunicación OSC funcionando
- Mensajes `/hit` enviados correctamente
- Mensajes `/state` opcionales funcionando
- Configuración de host/puerto
- **Loop modular completo validado: mouse → partículas → colisiones → OSC**

### Definition of Done (DoD)

- [ ] Mensajes `/hit` se envían correctamente (verificado con OSCulator o test-osc.py)
- [ ] Todos los valores están en rango especificado (0..1 para floats)
- [ ] Latencia de envío < 2ms (medido localmente)
- [ ] Script `test-osc.py` puede recibir mensajes de oF

---

## Fase 6: App B — Sintetizador básico (JUCE Standalone)

**Objetivo:** Implementar sintetizador con resonadores modales (sin OSC aún).

**Decisión de diseño:** **Standalone = entrega principal. Plugin (VST3/AU) = bonus opcional.**

**Referencia:** `spec.md` sección 5.3

### Tareas

#### 6.1 Setup de proyecto JUCE

- [x] Crear proyecto **Standalone** en Projucer
- [x] Configurar audio I/O básico:
  - [x] Seleccionar dispositivo de audio
  - [x] Configurar sample rate (44.1 kHz o 48 kHz)
  - [x] Configurar buffer size (256 o 512 samples)
- [x] Setup de procesamiento de audio:
  - [x] Implementar `AudioAppComponent` con callbacks de audio
  - [x] Configurar callbacks de audio

#### 6.2 Implementar resonador modal simple

- [x] **Diseño sonoro objetivo: "Coin Cascade" (cascada de monedas)**
  - [x] Textura de múltiples hits percusivos metálicos breves
  - [x] Capas de eventos pequeños que se acumulan en textura granular
  - [x] Timbre metálico percusivo (como monedas cayendo)
- [x] Estructura de voz:
  - [x] Clase `ModalVoice` implementada
  - [x] Excitación: impulso corto (noise burst ~5ms, 128 samples)
  - [x] Banco de 2 resonadores bandpass (optimizado para RT)
  - [x] Suma de resonadores → saturación suave opcional → salida
- [x] Modos inarmónicos para timbre metálico:
  - [x] Frecuencias de resonancia no armónicas
  - [x] Parámetro `Metalness` (inharmonic spread)
- [x] Parámetros por voz:
  - [x] Frecuencia base
  - [x] Damping (decaimiento)
  - [x] Brightness
  - [x] Amplitud

#### 6.3 Sistema de polyphony

- [x] Gestión de voces:
  - [x] `maxVoices` configurable (4-12 para estabilidad RT)
  - [x] Pool de voces pre-allocadas (hasta 32 voces)
- [x] Voice stealing:
  - [x] Algoritmo para robar voz con menor amplitud residual
  - [x] O robar voz más antigua
- [x] Limiter master para anti-saturación:
  - [x] Limiter suave en salida master
  - [x] Prevenir clipping

#### 6.4 UI básica

- [x] Controles principales:
  - [x] `Voices` (número máximo de voces, rango 4-12)
  - [x] `Metalness` (inharmonic spread)
  - [x] `Brightness`
  - [x] `Damping`
  - [x] `Drive` (saturación)
  - [x] `ReverbMix` (opcional)
  - [x] `Limiter on/off`
- [x] Indicadores:
  - [x] Nivel de salida (RMS en dB)
  - [x] Número de voces activas
- [x] Botón "Test Trigger" para testing manual

#### 6.5 Testing básico

- [x] Probar timbre con trigger manual:
  - [x] Trigger manual de voces (botón Test Trigger)
  - [x] Verificar timbre metálico
- [x] Verificar estabilidad de audio:
  - [x] Optimizaciones RT-safe implementadas
  - [x] Cola lock-free para eventos
  - [x] Pre-allocation de voces
  - [x] Límite de eventos por bloque (MAX_HITS_PER_BLOCK = 16)
- [x] Optimizaciones de rendimiento:
  - [x] Reducción a 2 modos por voz
  - [x] Voces por defecto: 8 (rango 4-12)
  - [x] Verificaciones de seguridad RT

### Entregables

- ✅ Sintetizador básico funcionando (Standalone)
- ✅ Timbre metálico característico (2 modos inarmónicos)
- ✅ Polyphony funcionando (4-12 voces, voice stealing)
- ✅ UI básica con controles y indicadores
- ✅ Audio estable con optimizaciones RT-safe
- ✅ Cola lock-free preparada para OSC (Fase 7)

### Definition of Done (DoD)

- [x] Audio sin glitches durante 5 minutos de ejecución continua (con optimizaciones RT)
- [x] Timbre metálico claramente audible (2 modos inarmónicos)
- [x] Voice stealing funciona correctamente
- [x] Optimizaciones RT-safe implementadas (cola lock-free, pre-allocation, sin allocations en audio thread)
- [ ] CPU usage < 30% en máquina de desarrollo (con 8-12 voces, objetivo ajustado para estabilidad)

---

## Fase 7: App B — Receptor OSC y mapeo

**Objetivo:** Integrar receptor OSC y mapear eventos a parámetros de síntesis.

**Referencia:** `spec.md` sección 5.2 y 5.5

### Tareas

#### 7.1 Integrar receptor OSC

- [ ] Listener en puerto 9000:
  - [ ] Configurar `OSCReceiver`
  - [ ] Conectar a puerto (configurable)
  - [ ] Manejar errores de conexión
- [ ] Parser de mensajes:
  - [ ] Callback para `/hit`
  - [ ] Callback para `/state`
  - [ ] Validación de formato y número de argumentos

#### 7.2 Implementar mapeo de parámetros (textura "Coin Cascade")

- [ ] **Mapeo desde `/hit` para textura "coin cascade":**
  - [ ] `energy` → amplitud: `amp = energy^γ` (γ = 1.2-2.0)
    - [ ] Micro-colisiones (energy muy baja) → amplitud muy baja pero audible (no mute)
    - [ ] Golpes fuertes (energy alta) → amplitud alta
  - [ ] `energy` → brightness: `brightness = lerp(minBright, maxBright, energy)`
    - [ ] Colisiones más energéticas = más brillantes
  - [ ] `x` → pan: `pan = (x * 2) - 1` (-1 = izquierda, 1 = derecha)
    - [ ] Distribución espacial de la cascada
  - [ ] `y` → damping: `damping = lerp(short, long, 1 - y)`
    - [ ] Partes superiores = más secas, inferiores = más reverberantes
  - [ ] `y` → pitch band (opcional): `freq = baseFreq * (1 + y * deviation)`
    - [ ] Diferentes bandas de pitch según posición vertical
  - [ ] `surface` → modulación opcional (diferentes timbres por superficie)
- [ ] **Manejo de estrés (múltiples hits simultáneos):**
  - [ ] Voice stealing strategy:
    - [ ] Robar voz más antigua O voz con menor amplitud residual
    - [ ] Priorizar mantener textura "cascade" sin gaps
  - [ ] Master limiter:
    - [ ] Limiter suave en salida master
    - [ ] Prevenir clipping bajo carga alta (200 hits/s)
- [ ] Integrar parámetros globales desde `/state`:
  - [ ] `activity` → global wet (reverb)
  - [ ] `gesture` → global drive
  - [ ] `presence` → master level

#### 7.3 Trigger de voces

- [ ] Al recibir `/hit`:
  - [ ] Obtener o crear voz disponible
  - [ ] Aplicar mapeo de parámetros
  - [ ] Trigger excitación
  - [ ] Aplicar pan si es estéreo

#### 7.4 UI mejorada

- [ ] Indicadores de actividad OSC:
  - [ ] LED o indicador visual cuando se reciben mensajes
  - [ ] Contador de mensajes recibidos
  - [ ] Indicador de conexión
- [ ] Controles de mapeo (opcionales):
  - [ ] Ajustar rangos de mapeo
  - [ ] Enable/disable diferentes mapeos

#### 7.5 Testing de integración

- [ ] Probar con script de OSC simulado (`scripts/test-osc.py`):
  - [ ] Verificar que se reciben mensajes
  - [ ] Verificar mapeo de parámetros
  - [ ] Verificar que se generan sonidos
- [ ] Test de estrés:
  - [ ] Enviar 200 hits/s con `test-osc.py --mode stress`
  - [ ] Verificar que no hay glitches
  - [ ] Verificar voice stealing funciona
- [ ] Test de latencia:
  - [ ] Medir latencia end-to-end (desde oF hasta audio)
  - [ ] **Objetivo: < 50ms total** (realista para OSC + audio buffer)

### Entregables

- Receptor OSC funcionando
- Mapeo de parámetros implementado
- Voces se triggeran correctamente desde OSC
- UI con indicadores de actividad
- Tests de integración pasando
- **Sistema completo funcionando: oF (mouse) → OSC → JUCE → Audio**

### Definition of Done (DoD)

- [ ] Audio sin glitch con 200 hits/s (test de estrés)
- [ ] Voice stealing funciona correctamente bajo carga
- [ ] Latencia end-to-end < 50ms (medido desde oF hasta audio)
- [ ] Mapeo de parámetros funciona (energy, x, y, surface)

---

## Fase 3b: App A — Integración MediaPipe (OPCIONAL/TARDÍA)

**Objetivo:** Integrar MediaPipe para tracking de manos/pose, reemplazando el input de mouse.

**Nota estratégica:** Esta fase se hace **después** de tener el loop modular completo funcionando (Fases 2-7). Si MediaPipe se demora o tiene problemas, la entrega se salva con mouse. **Checkpoint:** Si a X fecha MediaPipe no está estable, se congela y se presenta con mouse.

**Referencia:** `spec.md` sección 3.1

### Tareas

#### 3b.1 Integrar MediaPipe

- [ ] Setup de MediaPipe para tracking de manos (modo recomendado):
  - [ ] Configurar addon o integración externa
  - [ ] Inicializar detector de manos
  - [ ] Extraer posiciones normalizadas `handL.pos`, `handR.pos`
  - [ ] Extraer confianza por mano
- [ ] Implementar modo alternativo con pose (torso):
  - [ ] Detector de pose
  - [ ] Calcular `torso.pos` (midpoint hombros o cadera)
  - [ ] Calcular `torso.size` (distancia hombros)
  - [ ] Extraer `torso.vel`
- [ ] Manejo de errores:
  - [ ] Fallback automático a mouse si MediaPipe falla
  - [ ] Manejo de baja confianza

#### 3b.2 Implementar suavizado

- [ ] Filtro de suavizado con lerp:
  - [ ] `pos_smooth = lerp(pos_smooth, pos_raw, α)`
  - [ ] α configurable (rango recomendado 0.1-0.25)
- [ ] Cálculo de velocidad:
  - [ ] Derivar velocidad de posición suavizada
  - [ ] `vel = (pos_smooth - pos_smooth_prev) / dt`
  - [ ] Evitar ruido en velocidad

#### 3b.3 Integrar gestos con sistema de partículas

- [ ] Reutilizar código de Fase 3 (mouse) pero con efectores MediaPipe:
  - [ ] Soporte para múltiples efectores (mano L/R o torso)
  - [ ] Misma lógica de fuerza de gesto
- [ ] Actualizar mensaje `/state`:
  - [ ] `presence` ahora viene de confianza MediaPipe
  - [ ] `gesture` calculado desde energía de gestos reales

#### 3b.4 Testing

- [ ] Verificar respuesta a gestos de manos
- [ ] Verificar respuesta a gestos de torso (modo alternativo)
- [ ] Ajustar parámetros de influencia (`sigma`, `k_gesture`)
- [ ] Verificar suavizado reduce jitter
- [ ] Medir impacto en FPS
- [ ] Verificar fallback a mouse funciona

### Entregables

- Tracking de manos funcionando (opcional)
- Gestos afectan partículas correctamente
- Suavizado implementado y funcionando
- Fallback a mouse si MediaPipe falla

### Definition of Done (DoD)

- [ ] Tracking funciona con confianza > 0.7 típicamente
- [ ] Fallback a mouse funciona automáticamente si MediaPipe falla
- [ ] FPS se mantiene ≥ 30 fps con MediaPipe activo
- [ ] **Checkpoint:** Si MediaPipe no está estable a X fecha, se congela y se presenta con mouse

---

## Fase 8: Calibración y ajuste conjunto

**Objetivo:** Ajustar parámetros de ambas apps para funcionamiento óptimo conjunto.

**Referencia:** `spec.md` sección 6

### Tareas

#### 8.1 Calibrar parámetros de oF

- [ ] Ajustar parámetros de física:
  - [ ] `k_home` — retorno al origen
  - [ ] `k_gesture` — fuerza de gesto
  - [ ] `sigma` — radio de influencia
  - [ ] `k_drag` — amortiguación
- [ ] Ajustar parámetros de colisión:
  - [ ] `restitution` — rebote
  - [ ] `hit_threshold` — umbral de energía
  - [ ] `hit_cooldown_ms` — cooldown
- [ ] Ajustar rate limiting:
  - [ ] `max_hits_per_second` — límite global
  - [ ] `max_hits_per_frame` — límite por frame
- [ ] UI para ajuste en tiempo real:
  - [ ] Sliders para todos los parámetros
  - [ ] Guardar/cargar presets

#### 8.2 Calibrar parámetros de JUCE

- [ ] Ajustar rangos de frecuencia:
  - [ ] Frecuencia base del resonador
  - [ ] Spread de modos inarmónicos
- [ ] Ajustar damping y brightness:
  - [ ] Rangos de mapeo desde `energy` y `y`
- [ ] Ajustar limiter:
  - [ ] Threshold
  - [ ] Release time

#### 8.3 Testing de integración completa

- [ ] Ejecutar ambas apps simultáneamente:
  - [ ] Iniciar App B primero (receptor)
  - [ ] Iniciar App A (emisor)
- [ ] Verificar latencia end-to-end:
  - [ ] Medir desde gesto hasta sonido
  - [ ] **Objetivo: < 50ms total** (consistente con Fase 7)
- [ ] Ajustar para evitar glitches:
  - [ ] Optimizar si hay problemas de CPU
  - [ ] Ajustar buffer sizes si es necesario
- [ ] Probar diferentes escenarios:
  - [ ] Gesto lento → pocos hits
  - [ ] Gesto rápido → muchos hits
  - [ ] Sin usuario → partículas en reposo

### Entregables

- Parámetros calibrados para funcionamiento óptimo
- Sistema completo funcionando sin glitches
- Latencia aceptable (< 50ms)
- Presets guardados para diferentes configuraciones

### Definition of Done (DoD)

- [ ] Latencia end-to-end < 50ms (medido desde gesto hasta audio)
- [ ] Sin glitches durante 10 minutos de ejecución continua
- [ ] Rate limiting previene saturación de audio
- [ ] Presets funcionan (guardar/cargar)

---

## Fase 9: Documentación y demo

**Objetivo:** Completar documentación y preparar materiales de presentación.

**Referencia:** `spec.md` sección 9 y `Docs/requisitos/Trabajo Final ISTR.md`

### Tareas

#### 9.1 Documentación técnica

- [ ] Comentar código crítico:
  - [ ] Funciones principales
  - [ ] Algoritmos de física
  - [ ] Mapeo de parámetros
- [ ] Documentar API de parámetros:
  - [ ] Lista completa de parámetros configurables
  - [ ] Rangos y valores por defecto
  - [ ] Efectos de cada parámetro
- [ ] Crear diagrama de pipeline:
  - [ ] Flujo de datos completo
  - [ ] Componentes principales
  - [ ] Comunicación OSC

#### 9.2 Preparar demo

- [ ] Video de composición visual:
  - [ ] Captura de pantalla de App A
  - [ ] Muestra sistema de partículas
  - [ ] Muestra respuesta a gestos (o mouse si MediaPipe no está)
  - [ ] Duración: según requisitos (ver `Docs/requisitos/Trabajo Final ISTR.md`)
- [ ] Explicación (máx. 1 minuto al inicio del video):
  - [ ] Qué es el sistema
  - [ ] Cómo funciona
  - [ ] Input → fuerzas → partículas → eventos → sonido
- [ ] Captura integrada (pantalla + audio):
  - [ ] Sincronizar video y audio
  - [ ] Performance de 60-90s (bonus)
- [ ] Modo fallback funcionando:
  - [ ] Demostrar control con mouse
  - [ ] **Útil si falla MediaPipe durante presentación**

#### 9.3 Memoria descriptiva

- [ ] Descripción del trabajo (máx. 2 páginas):
  - [ ] Objetivo y motivación
  - [ ] Arquitectura del sistema
  - [ ] Componentes principales
  - [ ] Resultados y conclusiones
- [ ] Experiencia en la clase:
  - [ ] Qué se aprendió
  - [ ] Desafíos encontrados
  - [ ] Soluciones implementadas
- [ ] Decisiones de diseño:
  - [ ] Por qué se eligió esta arquitectura
  - [ ] Trade-offs considerados
  - [ ] MediaPipe como input opcional vs. mouse como fallback
  - [ ] Mejoras futuras posibles

### Entregables

- Código documentado
- Video de demo con explicación
- Memoria descriptiva completa
- Diagrama de pipeline
- Documentación de API

### Definition of Done (DoD)

- [ ] Video de demo completo con explicación ≤ 1 minuto
- [ ] Memoria descriptiva ≤ 2 páginas
- [ ] Código tiene comentarios en funciones críticas
- [ ] Diagrama de pipeline creado

---

## Checklist general de entregables

### Para ISTR (App A)

- [ ] Código oF completo y funcional
- [ ] Demo video con explicación (máx. 1 minuto al inicio)
- [ ] Explicación: input → fuerzas → partículas → eventos
- [ ] Diagrama de pipeline
- [ ] Memoria descriptiva (máx. 2 páginas)

### Para PAS1 (App B)

- [ ] **Standalone JUCE** (entrega principal) + preset "metal particles"
- [ ] Explicación: OSC → voices → resonador → master limiter
- [ ] Documentación de parámetros
- [ ] Memoria descriptiva (máx. 2 páginas)

### Bonus (si da el tiempo)

- [ ] Captura integrada (pantalla + audio) como performance de 60-90s
- [ ] Plugin VST3/AU (además de Standalone)
- [ ] MediaPipe integrado (además de mouse)

---

## Notas de desarrollo

### Orden recomendado de implementación (REVISADO)

**Orden crítico para validar loop modular:**
1. Fase 2: Partículas básicas
2. Fase 3: Input mouse (rápido, valida arquitectura)
3. Fase 4: Colisiones + eventos + rate limiting
4. Fase 5: OSC (adelantada, valida comunicación)
5. Fase 6-7: JUCE completo (Standalone + OSC)
6. **Fase 3b: MediaPipe (opcional, cuando el sistema ya funciona)**

Este orden permite validar el sistema completo antes de invertir tiempo en MediaPipe.

### Testing continuo

- Probar cada componente individualmente antes de integrar
- Usar scripts de prueba para validar comunicación OSC
- Medir rendimiento en cada fase
- **Validar loop completo (mouse → OSC → JUCE) antes de MediaPipe**

### Versionado

- Usar control de versiones (Git)
- Hacer commits frecuentes
- Taggear versiones estables
- **Taggear versión "mouse-only" como checkpoint seguro**

### Recursos útiles

- Documentación openFrameworks: https://openframeworks.cc/documentation/
- Documentación JUCE: https://juce.com/learn/documentation
- Especificación OSC: https://opensoundcontrol.stanford.edu/
- MediaPipe: https://mediapipe.dev/

### Decisiones de diseño explícitas

- **Standalone JUCE = entrega principal.** Plugin es bonus.
- **Mouse = input principal para validación.** MediaPipe es mejora opcional.
- **Rate limiting global = crítico** para evitar saturar audio.
- **Latencia objetivo = < 50ms** (realista para OSC + audio buffer).

---

## Estado actual

- [x] Fase 1: Setup inicial — Estructura creada
- [x] Fase 2: Sistema de partículas básico
- [x] Fase 3: Input básico (Mouse)
- [x] Fase 4: Colisiones y eventos (+ rate limiting)
- [x] Fase 5: Comunicación OSC (adelantada)
- [x] Fase 6: Sintetizador básico (JUCE Standalone) — **COMPLETADA**
- [ ] Fase 7: Receptor OSC y mapeo
- [ ] Fase 3b: Integración MediaPipe (opcional/tardía)
- [ ] Fase 8: Calibración y ajuste conjunto
- [ ] Fase 9: Documentación y demo

---

**Última actualización:** Febrero 2025  
**Versión:** 2.1 (Fase 6 completada - Sintetizador JUCE básico)
