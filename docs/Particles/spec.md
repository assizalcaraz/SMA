# Particles — Especificación Técnica

Especificación técnica detallada del módulo de partículas. Documentación para desarrolladores.

**Última actualización:** Fase 3 completada

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
    ofVec2f home;        // Posición de reposo
    ofVec2f pos;         // Posición actual
    ofVec2f vel;         // Velocidad
    float mass;          // Masa (default 1.0)
    int id;             // Identificador único
    float lastHitTime;  // Para cooldown (futuro)
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

### Parámetros Físicos

| Variable | Tipo | Rango | Default | Descripción |
|----------|------|-------|---------|-------------|
| `k_home` | `float` | 0.5-6.0 | 2.0 | Fuerza de retorno |
| `k_drag` | `float` | 0.5-3.0 | 1.0 | Amortiguación |
| `k_gesture` | `float` | 0-200 | 50.0 | Fuerza del gesto |
| `sigma` | `float` | 50-500 | 150.0 | Radio de influencia (px) |
| `speed_ref` | `float` | 100-2000 | 500.0 | Velocidad de referencia (px/s) |
| `smooth_alpha` | `float` | 0.1-0.25 | 0.15 | Factor de suavizado |

### Métodos Principales

#### `ofApp::setup()`

Inicializa:
- Parámetros físicos con valores por defecto
- Estructura `MouseEfector`
- GUI con sliders
- Sistema de partículas (grid + jitter)

#### `ofApp::update()`

Loop principal de actualización (llamado cada frame):

1. Actualiza parámetros desde sliders
2. Detecta cambios en N_particles y redimensiona si es necesario
3. `updateMouseInput()` - Actualiza posición y velocidad del mouse
4. `applyGestureForce()` - Aplica fuerza de gesto a partículas
5. Actualiza física de todas las partículas

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

#### `ofApp::draw()`

Renderiza el frame:

1. Fondo oscuro: `ofBackground(10, 10, 15)`
2. Render de partículas como puntos:
   ```cpp
   glPointSize(2.0f);
   glEnable(GL_POINT_SMOOTH);
   glBegin(GL_POINTS);
   for (particle : particles) {
       glVertex2f(particle.pos.x, particle.pos.y);
   }
   glEnd();
   glDisable(GL_POINT_SMOOTH);
   ```
3. Debug overlay (FPS, N, parámetros)
4. GUI (ofxGui panel)

---

## Física del Sistema

### Fuerzas Totales

Para cada partícula:
```
F_total = F_home + F_drag + F_gesture
```

Donde:
- `F_home = k_home * (home - pos)` - Retorno al origen
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
- **Tamaño:** 2.0 pixels (glPointSize)
- **Suavizado:** GL_POINT_SMOOTH habilitado
- **Fondo:** Oscuro (10, 10, 15)

### Optimización

- Renderizado en batch usando `glBegin(GL_POINTS)`
- Sin transformaciones individuales por partícula
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
- Número de partículas
- Valores de parámetros principales
- Velocidad del mouse (px/s)

**Ubicación:** Esquina superior izquierda (20, 20)

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

### Completado (Fase 2-3)

- ✅ Sistema de partículas básico
- ✅ Fuerzas F_home y F_drag
- ✅ Input mouse
- ✅ Fuerza de gesto F_gesture
- ✅ GUI con sliders
- ✅ Rendering de puntos
- ✅ Debug overlay

### Pendiente (Fases Futuras)

- ⏳ Colisiones con bordes (Fase 4)
- ⏳ Generación de eventos (Fase 4)
- ⏳ Rate limiting (Fase 4)
- ⏳ Comunicación OSC (Fase 5)
- ⏳ Integración MediaPipe (Fase 3b)

---

## Dependencias

- **openFrameworks** 0.12.1
- **ofxGui** - Interfaz de parámetros
- **OpenGL** - Rendering

---

**Última actualización:** Fase 3 completada
