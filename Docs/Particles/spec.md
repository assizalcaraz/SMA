# Particles — Especificación Técnica

Especificación técnica detallada del módulo de partículas. Documentación para desarrolladores.

**Última actualización:** v0.3 Chladni State (2026-02-XX)

---

## Estructura del Código

### Mapeo Archivos → Funcionalidad

| Archivo | Responsabilidad |
|---------|----------------|
| `main.cpp` | Punto de entrada, creación de ventana |
| `ofApp.h` | Declaración de clase principal, estructuras de datos |
| `ofApp.cpp` | Lógica principal: setup, update, draw, input, física |
| `Particle.h` | Declaración de clase partícula |
| `Particle.cpp` | Implementación de física de partículas |

---

## Clase Particle

### Estructura de Datos

```cpp
class Particle {
    ofVec2f home;            // Posición de reposo
    ofVec2f pos;             // Posición actual
    ofVec2f vel;             // Velocidad
    float mass;              // Masa (default 1.0)
    int id;                 // Identificador único
    float lastHitTime;       // Tiempo del último hit (para cooldown)
    ofVec2f vel_pre;         // Velocidad PRE-colisión (para cálculo de energía)
    float last_hit_distance; // Distancia recorrida desde último hit
    int last_surface;        // Última superficie impactada (0=L, 1=R, 2=T, 3=B, -1=N/A)
};
```

### Métodos

#### `Particle::update(float dt, float k_home, float k_drag)`

Actualiza la física de la partícula usando integración semi-implícita Euler.

**Fuerzas aplicadas:**
1. **F_home:** `F_home = k_home * (home - pos)`
2. **F_drag:** `F_drag = -k_drag * vel`

**Integración:**
```cpp
vel += (F / mass) * dt;
pos += vel * dt;
```

**Parámetros:**
- `dt`: Delta time en segundos (de `ofGetLastFrameTime()`)
- `k_home`: Constante de retorno (0.5-6.0)
- `k_drag`: Constante de drag (0.5-3.0)

#### `Particle::bounce(int surface, float restitution, float width, float height)`

Aplica rebote físico cuando la partícula colisiona con un borde.

**Algoritmo:**
1. Si superficie es horizontal (0=L, 1=R):
   - Invierte y amortigua velocidad X: `vel.x *= -restitution`
   - Clampea posición X dentro de bordes: `pos.x = clamp(pos.x, 0, width)`
2. Si superficie es vertical (2=T, 3=B):
   - Invierte y amortigua velocidad Y: `vel.y *= -restitution`
   - Clampea posición Y dentro de bordes: `pos.y = clamp(pos.y, 0, height)`

**Parámetros:**
- `surface`: Superficie impactada (0=L, 1=R, 2=T, 3=B)
- `restitution`: Coeficiente de restitución (0.2-0.85)
- `width`: Ancho de la ventana
- `height`: Alto de la ventana

**Nota:** El rebote se aplica solo en el eje perpendicular a la superficie impactada.

#### `Particle::reset()`

Resetea la partícula a su estado inicial.

**Efectos:**
- `pos = home`
- `vel = (0, 0)`
- `lastHitTime = 0`
- `vel_pre = (0, 0)`
- `last_hit_distance = 0`
- `last_surface = -1`

---

## Clase ofApp

### Estructuras de Datos

#### MouseEfector

```cpp
struct MouseEfector {
    ofVec2f pos;          // Posición normalizada (0..1)
    ofVec2f pos_prev;     // Posición anterior
    ofVec2f pos_smooth;   // Posición suavizada
    ofVec2f vel;          // Velocidad (pixels/s)
    bool active;          // Si el mouse está activo
};
```

#### HitEvent

```cpp
struct HitEvent {
    int id;              // ID de partícula
    float x;             // Posición X normalizada (0..1)
    float y;             // Posición Y normalizada (0..1)
    float energy;        // Energía del impacto (0..1)
    int surface;         // Superficie impactada (0=L, 1=R, 2=T, 3=B, -1=N/A)
};
```

#### RateLimiter

```cpp
struct RateLimiter {
    float tokens;         // Tokens disponibles
    float rate;          // Tokens por segundo (max_hits_per_second)
    float burst;         // Máximo de tokens acumulados
    int max_per_frame;   // Límite por frame
    int hits_this_frame; // Contador temporal
};
```

### Parámetros Físicos

| Variable | Tipo | Rango | Default | Descripción |
|----------|------|-------|---------|-------------|
| `k_home` | `float` | 0.5-6.0 | 2.0 | Fuerza de retorno |
| `k_drag` | `float` | 0.5-3.0 | 1.0 | Amortiguación |
| `k_gesture` | `float` | 0-200 | 50.0 | Fuerza del gesto |
| `sigma` | `float` | 50-500 | 150.0 | Radio de influencia (px) |
| `speed_ref` | `float` | 100-2000 | 500.0 | Velocidad de referencia (px/s) |
| `smooth_alpha` | `float` | 0.1-0.25 | 0.15 | Factor de suavizado |
| `restitution` | `float` | 0.2-0.85 | 0.6 | Coeficiente de rebote |
| `hit_cooldown_ms` | `float` | 30-120 | 60.0 | Cooldown por partícula (ms) |
| `vel_ref` | `float` | 300-1000 | 500.0 | Velocidad de referencia para energía (px/s) |
| `dist_ref` | `float` | 20-100 | 50.0 | Distancia de referencia para energía (px) |
| `energy_a` | `float` | 0.5-0.9 | 0.7 | Peso de velocidad en energía |
| `energy_b` | `float` | 0.1-0.5 | 0.3 | Peso de distancia en energía |
| `max_hits_per_second` | `float` | 50-500 | 200.0 | Máximo de hits por segundo |
| `burst` | `float` | 100-500 | 300.0 | Burst máximo de tokens |
| `max_hits_per_frame` | `int` | 5-20 | 10 | Máximo de hits por frame |

**v0.3 - Parámetros Chladni State:**

| Variable | Tipo | Default | Descripción |
|----------|------|---------|-------------|
| `chladniState` | `bool` | `false` | Estado ON/OFF del modo Chladni (toggle con SPACE) |
| `k_home_previous` | `float` | `k_home` | Valor guardado de k_home antes de activar Chladni |
| `plateShakerStrength` | `float` | 30.0 | Intensidad del Plate Shaker (constante) |

### Métodos Principales

#### `ofApp::setup()`

Inicializa:
- Parámetros físicos con valores por defecto
- Estructura `MouseEfector`
- Rate limiter con valores iniciales
- Contadores de debug (hits/seg, descartados)
- GUI con sliders (incluyendo nuevos de Fase 4)
- Sistema de partículas (grid + jitter)

#### `ofApp::update()`

Loop principal de actualización (llamado cada frame):

1. **v0.3**: Lógica de Chladni State para `k_home`:
   - Si `chladniState == true`: `k_home = 0.01f` (ignora slider)
   - Si `chladniState == false`: `k_home = kHomeSlider` (comportamiento v0.2)
2. Actualiza parámetros desde sliders (excepto `k_home` si Chladni está activo)
3. Detecta cambios en N_particles y redimensiona si es necesario
4. `updateMouseInput()` - Actualiza posición y velocidad del mouse
5. `applyGestureForce()` - Aplica fuerza de gesto a partículas
6. `applyPlateForce()` - Aplica fuerza de placa (incluye Plate Shaker v0.3 si está activo)
7. Actualiza física de todas las partículas
8. `checkCollisions()` - Detecta colisiones con bordes y genera eventos
9. `updateRateLimiter(dt)` - Actualiza tokens del rate limiter
10. `processPendingHits()` - Procesa y valida eventos de hit

#### `ofApp::updateMouseInput()`

Captura y procesa input del mouse:

1. Obtiene posición en pixels: `ofGetMouseX()`, `ofGetMouseY()`
2. Normaliza a (0..1): `pos = (mouseX/width, mouseY/height)`
3. Suavizado con lerp: `pos_smooth = lerp(pos_smooth, pos, α)`
4. Calcula velocidad: `vel = (pos_smooth - pos_prev) / dt`
5. Actualiza `pos_prev` para siguiente frame

**Fórmula de suavizado:**
```
pos_smooth = pos_smooth * (1 - α) + pos_raw * α
```

Donde `α = smooth_alpha = 0.15`

#### `ofApp::applyGestureForce()`

Aplica fuerza de gesto a todas las partículas basada en el movimiento del mouse.

**Algoritmo:**
1. Verifica que mouse esté activo y tenga velocidad > 1.0 px/s
2. Convierte posición suavizada a pixels
3. Normaliza velocidad del mouse: `dir = normalize(vel)`
4. Calcula velocidad normalizada: `speed = clamp(|vel|/speed_ref, 0..1)`
5. Para cada partícula:
   - Calcula distancia: `r = distance(particle_pos, mouse_pos)`
   - Calcula influencia gaussiana: `w = exp(-(r²)/(2*sigma²))`
   - Calcula fuerza: `F_gesture = k_gesture * w * speed * dir`
   - Aplica como impulso: `particle.vel += (F_gesture / mass) * dt`

**Fórmula de influencia gaussiana:**
```
w = exp(-(r²) / (2 * sigma²))
```

Donde:
- `r` = distancia en pixels desde partícula al mouse
- `sigma` = radio de influencia (50-500 pixels)

#### `ofApp::getModeCoefficients(int mode, int& m, int& n, float& a, float& b)`

Mapea `plate_mode` a parámetros de modo de Chladni y coeficientes de mezcla para modos degenerados.

**Parámetros:**
- `mode`: Modo de placa (0-7)
- `m`, `n`: Números de modo (salida)
- `a`, `b`: Coeficientes de mezcla (salida), donde `a² + b² = 1`

**Comportamiento:**
- Para modos simétricos (m == n): `a = 1.0`, `b = 0.0` (comportamiento original)
- Para modos degenerados (m != n): `a = b = 0.707106781f` (√2/2, mezcla 50/50)

**Mapeo de modos:**
- `0`: m=1, n=1 (fundamental, simétrico)
- `1`: m=1, n=2 (degenerado)
- `2`: m=2, n=1 (degenerado)
- `3`: m=2, n=2 (simétrico)
- `4`: m=3, n=1 (degenerado)
- `5`: m=1, n=3 (degenerado)
- `6`: m=3, n=2 (degenerado)
- `7`: m=2, n=3 (degenerado)

#### `ofApp::applyPlateForce()`

Aplica fuerza de placa de Chladni a todas las partículas basada en modos de vibración estacionarios.

**Algoritmo:**

1. **Sistema de coordenadas fijo:**
   - Centro de placa: siempre el centro de la ventana (inmutable)
   - Transformación: `pos_world → pos_plate → (x̂, ŷ) normalizado [0,1]`

2. **Mapeo de modo:**
   - Obtiene (m, n) y coeficientes (a, b) mediante `getModeCoefficients()`

3. **Campo estacionario con mezcla de modos degenerados:**
   - Calcula ambos términos: `U1 = sin(mπx̂) * sin(nπŷ)`, `U2 = sin(nπx̂) * sin(mπŷ)`
   - Combina: `U = a * U1 + b * U2`
   - Calcula y combina gradientes: `∇U = a * ∇U1 + b * ∇U2`

4. **Normalización por modo:**
   - Factor: `norm_factor = 1.0 / (m + n)`
   - Normaliza: `U_norm = U * norm_factor`, `∇U_norm = ∇U * norm_factor`
   - Evita que modos altos dominen por gradientes más pronunciados

5. **Energía y gradiente de energía:**
   - `E = U_norm²`
   - `∇E = 2U_norm * ∇U_norm`
   - Fuerza: `F = -∇E * forceIntensity * spatial_weight`

6. **Centrado suave de excitación:**
   - Distancia normalizada: `dist_norm = sqrt((xHat - 0.5)² + (yHat - 0.5)²)`
   - Peso gaussiano: `spatial_weight = exp(-dist_norm² / (2σ²))`
   - Reduce fuerza en bordes, centra excitación

7. **Estabilidad:**
   - Clamp de magnitud: `|F| ≤ F_MAX` (F_MAX = 100.0)
   - Amortiguación cerca de nodos: si `|U_norm| < THRESHOLD_NODE` (0.1):
     - `damping_factor = 1.0 / (1.0 + EXTRA_DAMPING * (1.0 - |U_norm|/THRESHOLD_NODE))`
     - Aplica: `p.vel *= damping_factor` (reduce velocidad, adhiere a líneas nodales)

**Fórmulas clave:**
```
U = a * sin(mπx̂) * sin(nπŷ) + b * sin(nπx̂) * sin(mπŷ)
E = U_norm²
∇E = 2U_norm * ∇U_norm
F = -∇E * forceIntensity * spatial_weight
```

**Parámetros de estabilidad:**
- `F_MAX = 100.0f`: Límite de magnitud de fuerza
- `THRESHOLD_NODE = 0.1f`: Umbral de |U| para activar amortiguación extra
- `EXTRA_DAMPING = 0.3f`: Fuerza de amortiguación adicional
- `SIGMA_SPATIAL = 0.4f`: Ancho gaussiano para centrado (coordenadas normalizadas)

**Características físicas:**
- Campo estacionario: no depende de tiempo ni frecuencia
- `plate_freq` solo afecta intensidad de excitación, no el patrón espacial
- Partículas se mueven hacia nodos (donde U ≈ 0)
- Modos degenerados se mezclan para restaurar simetría

#### `ofApp::applyPlateForce()` — v0.3: Plate Shaker

**v0.3 Extensión:** Sistema de inyección de energía coherente (Plate Shaker) que permite auto-organización sin mouse.

**Algoritmo Plate Shaker:**
1. **Condición de activación**: Solo si `chladniState == true` Y `plateAmp >= 0.01f`
2. **Energía normalizada**:
   - `E = U_norm²` (ya calculado)
   - `E_clamped = ofClamp(E, 0.0f, 1.0f)` (asegurar rango [0,1])
   - `E_shaped = pow(E_clamped, 2.0f)` (concentrar agitación en antinodos)
3. **Magnitud**: `shaker_magnitude = plateShakerStrength * plateAmp * E_shaped`
4. **Dirección coherente** (Opción A - RECOMENDADA):
   - `dir_x = ofSignedNoise(x * 0.01f, y * 0.01f, time * 0.5f)`
   - `dir_y = ofSignedNoise(x * 0.01f + 100.0f, y * 0.01f + 100.0f, time * 0.5f)`
   - `direction = normalize(vec2(dir_x, dir_y))`
   - Usa `ofSignedNoise()` para producir agitación espacialmente coherente que permite settling en nodos
5. **Fuerza**: `F_shaker = direction * shaker_magnitude`
6. **Clamp relativo**: `F_SHAKER_MAX = 0.5f * F_MAX` (proporción de constante existente)
7. **Aplicación**: `p.vel += (F_shaker / p.mass) * dt`

**Parámetros v0.3:**
- `plateShakerStrength = 30.0f`: Intensidad base del shaker
- `F_SHAKER_MAX = 0.5f * F_MAX`: Límite relativo (no magic number)
- `dt` clamp: `if (dt > 1.0f/30.0f) dt = 1.0f/30.0f` (consistencia FPS)

**Características:**
- Independiente de `k_gesture` y mouse input
- Fuerza proporcional a energía local (más fuerte en antinodos)
- Dirección coherente permite settling en líneas nodales
- No usa `ofRandom()` por partícula (evita jitter incoherente)

#### `ofApp::initializeParticles(int n)`

Inicializa el sistema de partículas con distribución grid + jitter.

**Algoritmo:**
1. Calcula grid: `cols = sqrt(n)`, `rows = ceil(n/cols)`
2. Calcula tamaño de celda: `cellWidth = width/cols`, `cellHeight = height/rows`
3. Para cada partícula:
   - Posición base: centro de celda del grid
   - Aplica jitter aleatorio: ±30% del tamaño de celda
   - Clampea dentro de márgenes (50px)
   - Crea partícula con `home = pos` (inicialmente en reposo)

**Distribución:**
- Grid uniforme para distribución espacial
- Jitter aleatorio para evitar patrones rígidos
- Márgenes de 50px desde bordes

#### `ofApp::resizeParticles(int newN)`

Redimensiona el sistema de partículas en tiempo real.

**Implementación:**
- Reinicializa todas las partículas con nuevo N
- Preserva distribución grid+jitter
- Seguro: no causa crashes

#### `ofApp::checkCollisions()`

Detecta colisiones de partículas con los bordes de la ventana y genera eventos de hit.

**Algoritmo:**
1. Para cada partícula:
   - Guarda velocidad PRE-colisión: `vel_pre = vel`
   - Verifica si está fuera de los bordes (margen de 0px)
   - Si colisiona:
     - Calcula superficie impactada (0=L, 1=R, 2=T, 3=B)
     - Verifica cooldown: `timeNow - lastHitTime > hit_cooldown_ms`
     - Si pasa cooldown:
       - Calcula energía: `calculateHitEnergy(particle, surface)`
       - Genera evento: `generateHitEvent(particle, surface)`
       - Aplica rebote: `particle.bounce(surface, restitution, width, height)`
       - Actualiza `lastHitTime` y `last_surface`

**Superficies:**
- `0` = Borde izquierdo (x < 0)
- `1` = Borde derecho (x > width)
- `2` = Borde superior (y < 0)
- `3` = Borde inferior (y > height)
- `-1` = Colisión partícula-partícula (no superficie)

#### `ofApp::checkParticleCollisions()`

Detecta colisiones entre partículas y genera eventos de hit.

**Algoritmo:**
1. Guarda velocidades PRE-colisión para todas las partículas
2. Para cada pareja de partículas (i, j) donde i < j:
   - Calcula distancia: `distance = |p1.pos - p2.pos|`
   - Si `distance < 2 * particle_radius`:
     - Calcula punto de colisión: `collisionPoint = (p1.pos + p2.pos) * 0.5`
     - Calcula velocidad relativa: `relVel = p1.vel_pre - p2.vel_pre`
     - Calcula normal: `normal = (p1.pos - p2.pos).normalized()`
     - Si partículas se acercan (`relVel.dot(normal) < 0`):
       - Aplica rebote elástico
       - Separa partículas para evitar penetración
       - Genera evento: `generateParticleHitEvent(p1, p2, collisionPoint)`

**Física de rebote:**
- Impulso: `impulse = velAlongNormal * (1.0 + restitution)`
- Aplicación: `p1.vel -= normal * impulse * 0.5`, `p2.vel += normal * impulse * 0.5`
- Separación: `overlap = 2*radius - distance`, separación proporcional a overlap

#### `ofApp::calculateParticleCollisionEnergy(Particle& p1, Particle& p2)`

Calcula la energía del impacto entre dos partículas basada en velocidad relativa.

**Fórmulas:**
```
relSpeed = |p1.vel_pre - p2.vel_pre|
speed_norm = clamp(relSpeed / vel_ref, 0..1)
avg_distance = (p1.last_hit_distance + p2.last_hit_distance) * 0.5
dist_norm = clamp(avg_distance / dist_ref, 0..1)
energy = clamp(energy_a * speed_norm + energy_b * dist_norm, 0..1)
```

**Parámetros:**
- `p1`, `p2`: Referencias a las partículas en colisión

**Retorna:** Energía normalizada (0..1)

#### `ofApp::generateParticleHitEvent(Particle& p1, Particle& p2, ofVec2f collisionPoint)`

Genera un evento de hit cuando dos partículas colisionan.

**Algoritmo:**
1. Verifica cooldown para ambas partículas (al menos una debe estar fuera de cooldown)
2. Calcula energía: `energy = calculateParticleCollisionEnergy(p1, p2)`
3. Selecciona partícula con mayor velocidad para ID del evento
4. Crea `HitEvent`:
   - `id = p.id` (partícula con mayor velocidad)
   - `x = collisionPoint.x / width` (normalizado)
   - `y = collisionPoint.y / height` (normalizado)
   - `energy = energy`
   - `surface = -1` (indica colisión partícula-partícula)
5. Agrega a `pending_hits`
6. Actualiza estado de ambas partículas (`lastHitTime`, `last_hit_distance`, `last_surface = -1`)

#### `ofApp::calculateHitEnergy(Particle& p, int surface)`

Calcula la energía del impacto basada en velocidad y distancia.

**Fórmulas:**
```
speed_norm = clamp(|vel_pre| / vel_ref, 0..1)
dist_norm = clamp(last_hit_distance / dist_ref, 0..1)
energy = clamp(energy_a * speed_norm + energy_b * dist_norm, 0..1)
```

**Parámetros:**
- `p`: Referencia a la partícula
- `surface`: Superficie impactada

**Retorna:** Energía normalizada (0..1)

#### `ofApp::generateHitEvent(Particle& p, int surface)`

Genera un evento de hit y lo agrega a `pending_hits`.

**Algoritmo:**
1. Calcula energía: `energy = calculateHitEnergy(p, surface)`
2. Crea `HitEvent`:
   - `id = p.id`
   - `x = p.pos.x / width` (normalizado)
   - `y = p.pos.y / height` (normalizado)
   - `energy = energy`
   - `surface = surface`
3. Agrega a `pending_hits`

#### `ofApp::updateRateLimiter(float dt)`

Actualiza el sistema de rate limiting (token bucket).

**Algoritmo:**
1. Acumula tokens: `tokens += rate * dt`
2. Limita a burst: `tokens = min(tokens, burst)`
3. Resetea contador de frame: `hits_this_frame = 0`

**Parámetros:**
- `dt`: Delta time en segundos

#### `ofApp::canEmitHit()`

Verifica si se puede emitir un hit (rate limiting).

**Retorna:** `true` si:
- `tokens >= 1.0`
- `hits_this_frame < max_per_frame`

#### `ofApp::consumeToken()`

Consume un token del rate limiter.

**Efectos:**
- `tokens -= 1.0`
- `hits_this_frame += 1`

#### `ofApp::processPendingHits()`

Procesa eventos pendientes y los valida con rate limiting.

**Algoritmo:**
1. Para cada evento en `pending_hits`:
   - Si `canEmitHit()`:
     - Consume token: `consumeToken()`
     - Agrega a `validated_hits`
     - Incrementa contadores de debug
   - Si no:
     - Incrementa `hits_discarded_rate`
2. Limpia `pending_hits`
3. Actualiza estadísticas (hits/seg, etc.)

**Nota:** Los eventos en `validated_hits` se envían automáticamente por OSC (Fase 5 completada).

#### `ofApp::draw()`

Renderiza el frame:

1. Fondo oscuro: `ofBackground(10, 10, 15)`
2. Aplicar transformaciones de cámara:
   ```cpp
   ofPushMatrix();
   ofTranslate(centerX, centerY);
   ofScale(cameraZoom, cameraZoom);
   ofRotateDeg(cameraRotation);
   ofTranslate(-centerX, -centerY);
   ```
3. Render de partículas como puntos:
   ```cpp
   glPointSize(particleSize);  // Tamaño variable desde slider
   glEnable(GL_POINT_SMOOTH);
   glBegin(GL_POINTS);
   int rendered_count = 0;
   for (particle : particles) {
       glVertex2f(particle.pos.x, particle.pos.y);
       rendered_count++;
   }
   glEnd();
   glDisable(GL_POINT_SMOOTH);
   particles_rendered_this_frame = rendered_count;
   ```
4. Restaurar transformaciones: `ofPopMatrix()`
5. Debug overlay (FPS, N, parámetros, partículas renderizadas)
6. GUI (ofxGui panel)

---

## Física del Sistema

### Fuerzas Totales

Para cada partícula:
```
F_total = F_home + F_drag + F_gesture + F_plate
```

Donde:
- `F_home = k_home * (home - pos)` - Retorno al origen
- `F_plate = -∇E * forceIntensity * spatial_weight` - Fuerza de placa de Chladni (ver `applyPlateForce()`)
- `F_drag = -k_drag * vel` - Amortiguación
- `F_gesture = k_gesture * w * speed * dir` - Fuerza de gesto (si aplica)

### Integración Temporal

**Método:** Semi-implícita Euler

```cpp
vel += (F_total / mass) * dt;
pos += vel * dt;
```

**Delta time:** `dt = ofGetLastFrameTime()` (en segundos)

---

## Rendering

### Estilo Visual

- **Partículas:** Puntos blancos (255, 255, 255)
- **Tamaño:** Variable (1.0-10.0 pixels) controlado por `particleSize` slider
- **Suavizado:** GL_POINT_SMOOTH habilitado
- **Fondo:** Oscuro (10, 10, 15)

### Transformaciones de Cámara

El sistema aplica transformaciones de cámara antes del renderizado:

1. **Zoom:** `ofScale(cameraZoom, cameraZoom)` - Escala uniforme
2. **Rotación:** `ofRotateDeg(cameraRotation)` - Rotación en grados
3. **Centro:** Transformaciones aplicadas desde el centro de la ventana

**Implementación:**
```cpp
ofPushMatrix();
float centerX = ofGetWidth() / 2.0f;
float centerY = ofGetHeight() / 2.0f;
ofTranslate(centerX, centerY);
ofScale(cameraZoom, cameraZoom);
ofRotateDeg(cameraRotation);
ofTranslate(-centerX, -centerY);
// Render partículas
ofPopMatrix();
```

### Controles de Cámara

#### Sliders

- **camera_zoom:** Rango 0.1-5.0, valor por defecto 1.0
- **camera_rotation:** Rango -180.0-180.0, valor por defecto 0.0

#### Presets (Teclas 1-4)

Implementados en `keyPressed(int key)`:

- **Tecla '1':** Vista normal (zoom=1.0, rotación=0°)
- **Tecla '2':** Zoom in (zoom=2.0, rotación=0°)
- **Tecla '3':** Rotación 45° (zoom=1.0, rotación=45°)
- **Tecla '4':** Vista amplia (zoom=0.5, rotación=0°)

Los presets actualizan automáticamente los sliders en la GUI.

### Diagnóstico de Renderizado

Se agregó contador de partículas renderizadas para análisis:

- **Variable:** `particles_rendered_this_frame` (int)
- **Actualización:** En cada frame durante el renderizado
- **Display:** Mostrado en debug overlay como "Particles (rendered): X / Y"

Ver [RENDERING_ANALYSIS.md](RENDERING_ANALYSIS.md) para análisis detallado del problema de renderizado.

### Optimización

- Renderizado en batch usando `glBegin(GL_POINTS)`
- Transformaciones de cámara aplicadas una vez por frame (eficiente)
- Eficiente para N alto (hasta 8000 partículas)

---

## Input

### Mouse (Fase 3)

**Captura:**
- Posición: `ofGetMouseX()`, `ofGetMouseY()`
- Normalización: `(0..1)` según tamaño de ventana
- Suavizado: Lerp con `α = 0.15`

**Velocidad:**
- Calculada como: `vel = (pos - pos_prev) / dt`
- En unidades de pixels/segundo
- Normalizada para fuerza de gesto: `speed = clamp(|vel|/speed_ref, 0..1)`

### MediaPipe (Fase 3b - Futuro)

Misma interfaz que mouse (`MouseEfector`), pero con datos de MediaPipe.

---

## GUI (ofxGui)

### Sliders Implementados

| Slider | Variable | Rango | Default |
|--------|----------|-------|---------|
| N Particles | `nParticlesSlider` | 500-8000 | 2000 |
| k_home | `kHomeSlider` | 0.5-6.0 | 2.0 |
| k_drag | `kDragSlider` | 0.5-3.0 | 1.0 |
| k_gesture | `kGestureSlider` | 0-200 | 50.0 |
| sigma | `sigmaSlider` | 50-500 | 150.0 |
| speed_ref | `speedRefSlider` | 100-2000 | 500.0 |
| restitution | `restitutionSlider` | 0.2-0.85 | 0.6 |
| hit_cooldown (ms) | `hitCooldownSlider` | 30-120 | 60.0 |
| vel_ref | `velRefSlider` | 300-1000 | 500.0 |
| dist_ref | `distRefSlider` | 20-100 | 50.0 |
| energy_a | `energyASlider` | 0.5-0.9 | 0.7 |
| energy_b | `energyBSlider` | 0.1-0.5 | 0.3 |
| max_hits/s | `maxHitsPerSecondSlider` | 50-500 | 200.0 |
| burst | `burstSlider` | 100-500 | 300.0 |
| max_hits/frame | `maxHitsPerFrameSlider` | 5-20 | 10 |
| particle_size | `particleSizeSlider` | 1.0-10.0 | 2.0 |
| camera_zoom | `cameraZoomSlider` | 0.1-5.0 | 1.0 |
| camera_rotation | `cameraRotationSlider` | -180.0-180.0 | 0.0 |

### Actualización

Todos los sliders se actualizan en `update()`:
```cpp
k_home = kHomeSlider;
k_drag = kDragSlider;
// etc.
```

Cambios se aplican inmediatamente (tiempo real).

---

## Debug Overlay

Muestra en pantalla:
- FPS actual
- Número de partículas (total y renderizadas)
- Valores de parámetros principales
- Velocidad del mouse (px/s)
- Hits por segundo (promedio móvil)
- Hits descartados (por rate limiting y cooldown)
- Tokens disponibles en rate limiter
- Eventos pendientes y validados

**Ubicación:** Esquina superior izquierda (20, 20)

**v0.2:** Agregado contador de partículas renderizadas para diagnóstico de rendimiento.

---

## Rendimiento

### Objetivos

- **FPS objetivo:** ≥ 45 fps con N=2000 partículas
- **FPS mínimo:** ≥ 30 fps con N=5000 partículas
- **Sin crashes:** 5+ minutos de ejecución continua

### Optimizaciones

- Vector pre-reservado: `particles.reserve(n)`
- Renderizado en batch
- Cálculos vectoriales eficientes (ofVec2f)
- Sin allocations en update loop

---

## Estado de Implementación

### Completado (Fase 2-4, v0.2)

- ✅ Sistema de partículas básico
- ✅ Fuerzas F_home y F_drag
- ✅ Input mouse
- ✅ Fuerza de gesto F_gesture
- ✅ GUI con sliders
- ✅ Rendering de puntos
- ✅ Debug overlay
- ✅ Detección de colisiones con bordes
- ✅ Sistema de rebote con coeficiente de restitución
- ✅ Cálculo de energía de impacto (velocidad + distancia)
- ✅ Generación de eventos de hit
- ✅ Cooldown por partícula
- ✅ Rate limiting (token bucket)
- ✅ Estadísticas de debug extendidas
- ✅ Control de tamaño de partículas (v0.2)
- ✅ Controles de cámara: zoom y rotación (v0.2)
- ✅ Presets de cámara con teclas 1-4 (v0.2)
- ✅ Contadores de diagnóstico de renderizado (v0.2)

### Pendiente (Fases Futuras)

- ✅ Comunicación OSC (Fase 5) - Completada
- ⏳ Integración MediaPipe (Fase 3b)

---

## Dependencias

- **openFrameworks** 0.12.1
- **ofxGui** - Interfaz de parámetros
- **OpenGL** - Rendering

---

**Última actualización:** Fase 4 completada + Mejoras Chladni (2026-02-10)
