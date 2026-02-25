# Documentación del Módulo Particles

**Módulo**: App A - openFrameworks  
**Estado**: Fase 4 completada (colisiones y eventos funcionando)

Este módulo implementa el sistema de partículas físicas que responde a gestos del usuario (mouse o MediaPipe) y genera eventos sonoros mediante colisiones.

---

## Documentación Disponible

- **[readme.md](readme.md)** — Descripción general y arquitectura del módulo
- **[manual.md](manual.md)** — Manual de usuario (parámetros, uso, configuración)
- **[spec.md](spec.md)** — Especificación técnica detallada (implementación, física, código)

---

## Mapeo Código → Documentación

| Archivo | Documentación | Descripción |
|---------|---------------|-------------|
| `Particles/src/main.cpp` | [spec.md](spec.md) | Punto de entrada de la aplicación |
| `Particles/src/ofApp.h` | [spec.md](spec.md) | Declaración de la clase principal |
| `Particles/src/ofApp.cpp` | [spec.md](spec.md) | Implementación: setup, update, draw, input, física |
| `Particles/src/Particle.h` | [spec.md](spec.md) | Declaración de la clase partícula |
| `Particles/src/Particle.cpp` | [spec.md](spec.md) | Implementación de física de partículas |

---

## Estado de Implementación

### ✅ Completado

- **Fase 2**: Sistema de partículas básico
  - Física de partículas (F_home, F_drag)
  - Integración semi-implícita Euler
  - UI con sliders ajustables
  - Rendering de puntos
  - Debug overlay

- **Fase 3**: Input básico (Mouse)
  - Captura de posición y velocidad del mouse
  - Suavizado de input
  - Fuerza de gesto con influencia gaussiana
  - Parámetros ajustables

- **Fase 4**: Colisiones y eventos
  - Detección de colisiones con bordes
  - **Detección de colisiones partícula-partícula** (nuevo)
  - Sistema de rebote con coeficiente de restitución
  - Cálculo de energía de impacto
  - Generación de eventos de hit
  - Rate limiting (token bucket)
  - Cooldown por partícula
  - Estadísticas de debug

### ✅ Completado (Continuación)

- **Fase 5**: Comunicación OSC ✅ **COMPLETADA**
  - ✅ Integración de ofxOsc
  - ✅ Envío de mensajes /hit
  - ✅ Envío de mensajes /state (opcional)
  - ✅ Configuración de host y puerto (127.0.0.1:9000)
  - ✅ Validación de mensajes antes de envío

- **Mejoras ISTR (Audit Pack)**
  - Puerta de reposo: no se generan hits cuando la velocidad normal de colisión es inferior a un umbral (evita “rest hits”).
  - Energía: la contribución de distancia está ponderada por velocidad, evitando energía por acumulación de distancia en reposo.
  - Calidad: suelo de energía perceptible (ENERGY_FLOOR), cooldown tras filtro de calidad; contadores de auditoría (candidatos, pending, validated, sent, descartes).
  - Fairness: priorización por energía (nth_element/partial_sort) antes del rate limiting para reducir sesgo FIFO.
  - Rate limiting: dos token buckets (borde y partícula-partícula) con presupuesto borde más estricto.
  - Debug: fondo semi-opaco detrás del overlay para legibilidad.

### Verificación (contadores y token buckets)

- **Tiempo base del overlay**: Los contadores con etiqueta `(per_sec)` se resetean cada segundo; el resto son por frame o acumulados en el frame actual.
- **this_frame**: Muestra `validated`, `sent`, `dropped` del frame actual. Con OSC ON, `validated` y `sent` deben coincidir; si no, revisar que `hits_sent_osc++` se llame solo en el bucle que invoca `sendHitEvent(event)`.
- **Selección por presupuesto vs token bucket**: Antes de `processPendingHits()` se aplica un **presupuesto por frame** (`budget_frame = min(max_per_frame, ceil(target_hits_per_second / fps))`). Se eligen los `budget_frame` eventos de mayor energía (nth_element + resize); el resto se cuenta como `discarded_by_budget`. El **token bucket** sigue actuando como red de seguridad sobre ese subconjunto ya reducido, por lo que `osc_msgs_dropped_by_rate_limiter` debería ser bajo (objetivo &lt; 5% de sent_osc). `discarded_by_budget` (per_sec) será alto cuando hay muchos candidatos; es la selección por calidad, no un fallo.
- **Tokens**: `Tokens border` y `pp` son los tokens disponibles tras el refill (cada frame: `tokens = min(burst, tokens + rate*dt_sec)`). Con presupuesto activo, el volumen que llega al limiter es menor y los tokens no deberían agotarse de forma constante.
- **Criterios de aceptación**:
  - Partículas en reposo: `candidate_p2p` ~0 y `sent_osc` ~0.
  - Alta densidad: `dropped_by_rate_limiter` (per_sec) bajo respecto a `sent_osc`; `discarded_by_budget` alto (esperado); `sent_osc` cercano a `target_hits_per_second` (acotado por max_per_frame y candidatos).
  - PAS recibe menos o similar OSC/s, con mejor claridad perceptual.

### Pendiente

- **Fase 3b**: Integración MediaPipe (opcional/tardía)

---

## Características Principales

### Física de Partículas

- Sistema de partículas con fuerzas dinámicas
- Retorno al origen (home)
- Influencia de gestos del usuario
- Integración numérica estable
- **Colisiones partícula-partícula** con rebote elástico

### Input

- **Mouse**: Implementado y funcionando
- **MediaPipe**: Pendiente (opcional)

### Parámetros Ajustables

- `N_particles` (500-8000) — Número de partículas
- `k_home` (0.5-6.0) — Fuerza de retorno al origen
- `k_drag` (0.5-3.0) — Fuerza de drag
- `k_gesture` (0-200) — Fuerza de gesto
- `sigma` (50-500) — Radio de influencia del gesto
- `speed_ref` (100-2000) — Velocidad de referencia
- `restitution` (0.2-0.85) — Coeficiente de rebote
- `hit_cooldown` (30-120 ms) — Cooldown por partícula
- `particle_radius` (2-20 px) — Radio de colisión entre partículas
- `enable_particle_collisions` (toggle) — Habilitar/deshabilitar colisiones partícula-partícula
- `vel_ref` (300-1000) — Velocidad de referencia para energía
- `dist_ref` (20-100) — Distancia de referencia para energía
- `energy_a` (0.5-0.9) — Peso de velocidad en energía
- `energy_b` (0.1-0.5) — Peso de distancia en energía
- `max_hits/s` (50-500) — Máximo de hits por segundo
- `burst` (100-500) — Burst máximo de tokens
- `max_hits/frame` (5-20) — Máximo de hits por frame

---

## Dependencias

- **openFrameworks** 0.12.1
- **ofxGui** — Para interfaz de parámetros
- **OpenGL** — Para rendering
- **ofxOsc** — Para comunicación OSC ✅ implementado

---

## Requisitos del Sistema

### Software

- **openFrameworks** 0.12.1 instalado
- **Xcode** (macOS) o **Visual Studio** (Windows) o **Code::Blocks** (Linux)
- **CMake** (opcional, para regenerar proyecto)

### Hardware

- **CPU**: Mínimo 2.0 GHz (recomendado 2.5+ GHz)
- **RAM**: Mínimo 4 GB (recomendado 8+ GB)
- **GPU**: OpenGL 2.1+ compatible
- **Resolución**: Ventana 1024x768 (configurable)

### Rendimiento Esperado

- **N=2000 partículas**: ≥ 45 FPS
- **N=5000 partículas**: ≥ 30 FPS
- **N=8000 partículas**: ≥ 20 FPS (puede variar según hardware)

---

## Compilación y Ejecución

### Compilar con Xcode (macOS)

1. **Abrir el proyecto:**
   ```bash
   cd Particles
   open Particles.xcodeproj
   ```

2. **Seleccionar esquema:**
   - `Particles Debug` (desarrollo)
   - `Particles Release` (optimizado)

3. **Compilar:**
   - `Cmd + B` o `Product > Build`

4. **Ejecutar:**
   - `Cmd + R` o `Product > Run`

### Compilar con Makefile (Linux/macOS)

1. **Configurar OF_ROOT** (si no está configurado):
   ```bash
   export OF_ROOT=/ruta/a/of_v0.12.1
   ```

2. **Compilar:**
   ```bash
   cd Particles
   make
   ```

3. **Ejecutar:**
   ```bash
   make run
   # o directamente:
   bin/ParticlesDebug
   ```

### Regenerar Proyecto

Si necesitas regenerar el proyecto Xcode (por ejemplo, después de agregar addons):

1. Abrir **Project Generator** de openFrameworks
2. **Project Path**: `/ruta/al/proyecto/Particles`
3. **Project Name**: `Particles`
4. **Addons**: Seleccionar `ofxGui` (y otros si es necesario)
5. Click en **Update** o **Generate**

Ver [`REGENERAR_PROYECTO.md`](../../Particles/REGENERAR_PROYECTO.md) para más detalles.

---

## Arquitectura del Sistema

### Componentes Principales

```
┌─────────────────────────────────────────┐
│           ofApp (Clase Principal)      │
├─────────────────────────────────────────┤
│  • Gestión del ciclo de vida           │
│  • Coordinación de componentes         │
│  • Input y procesamiento                │
│  • Rendering                            │
└─────────────────────────────────────────┘
           │
           ├───► ┌──────────────────────┐
           │     │  Sistema de           │
           │     │  Partículas           │
           │     │  (vector<Particle>)   │
           │     └──────────────────────┘
           │
           ├───► ┌──────────────────────┐
           │     │  MouseEfector        │
           │     │  (Input Handler)     │
           │     └──────────────────────┘
           │
           ├───► ┌──────────────────────┐
           │     │  GUI (ofxGui)        │
           │     │  (Parámetros)        │
           │     └──────────────────────┘
           │
           └───► ┌──────────────────────┐
                 │  OSC Sender          │
                 │  (Futuro)            │
                 └──────────────────────┘
```

### Flujo de Ejecución

1. **Setup** (`ofApp::setup()`)
   - Inicializa parámetros físicos
   - Crea sistema de partículas (grid + jitter)
   - Configura GUI con sliders
   - Inicializa estructuras de input

2. **Update Loop** (`ofApp::update()`)
   - Actualiza parámetros desde sliders
   - Procesa input del mouse (`updateMouseInput()`)
   - Aplica fuerza de gesto (`applyGestureForce()`)
   - Actualiza física de todas las partículas
   - (Futuro) Detecta colisiones y genera eventos

3. **Render Loop** (`ofApp::draw()`)
   - Limpia fondo
   - Renderiza partículas como puntos
   - Dibuja debug overlay
   - Dibuja GUI

### Física del Sistema

El sistema aplica tres fuerzas principales a cada partícula:

1. **F_home**: Retorno al origen
   ```
   F_home = k_home * (home - pos)
   ```

2. **F_drag**: Amortiguación
   ```
   F_drag = -k_drag * vel
   ```

3. **F_gesture**: Influencia del gesto (si aplica)
   ```
   F_gesture = k_gesture * w * speed * dir
   ```
   Donde `w` es la influencia gaussiana por distancia.

La integración temporal usa **semi-implícita Euler**:
```cpp
vel += (F_total / mass) * dt;
pos += vel * dt;
```

---

## Flujo de Datos

### Input → Física → Render

```
Input (Mouse)
    │
    ├─► Captura posición y velocidad
    ├─► Suavizado (lerp)
    └─► Cálculo de velocidad normalizada
         │
         └─► applyGestureForce()
              │
              ├─► Para cada partícula:
              │   ├─► Calcula distancia al mouse
              │   ├─► Calcula influencia gaussiana
              │   └─► Aplica fuerza de gesto
              │
              └─► Particle::update()
                   │
                   ├─► Aplica F_home
                   ├─► Aplica F_drag
                   └─► Integra posición y velocidad
                        │
                        └─► Render (draw)
```

### Flujo Futuro (con OSC)

```
Partículas → Colisiones → Eventos → OSC → JUCE
    │            │           │        │      │
    │            │           │        │      └─► Audio
    │            │           │        │
    │            │           │        └─► /hit messages
    │            │           │
    │            │           └─► Rate limiting
    │            │
    │            └─► Detección de bordes
    │
    └─► Física y movimiento
```

---

## Troubleshooting

### Problemas Comunes

#### El proyecto no compila

**Síntomas**: Errores de compilación relacionados con addons o rutas.

**Soluciones**:
1. Verificar que `OF_ROOT` esté configurado correctamente
2. Regenerar el proyecto con Project Generator
3. Limpiar build: `Product > Clean Build Folder` (Xcode)
4. Verificar que `addons.make` contenga solo `ofxGui`

#### Bajo rendimiento (FPS bajo)

**Síntomas**: FPS < 30 con N=2000 partículas.

**Soluciones**:
1. Reducir `N_particles` a valores más bajos (1000-1500)
2. Verificar que estés usando build Release (no Debug)
3. Cerrar otras aplicaciones que consuman GPU
4. Verificar drivers de gráficos actualizados

#### El mouse no afecta las partículas

**Síntomas**: Las partículas no responden al movimiento del mouse.

**Soluciones**:
1. Verificar que `k_gesture > 0` en los sliders
2. Verificar que `sigma` no sea demasiado bajo (< 50)
3. Mover el mouse más rápido (aumentar `speed_ref` si es necesario)
4. Verificar que el mouse esté dentro de la ventana

#### Las partículas desaparecen o se salen de la pantalla

**Síntomas**: Partículas fuera de los bordes visibles.

**Soluciones**:
1. Aumentar `k_home` para que regresen más rápido
2. Aumentar `k_drag` para reducir inercia
3. (Futuro) Las colisiones con bordes resolverán esto automáticamente

#### La GUI no aparece

**Síntomas**: No se ven los sliders en pantalla.

**Soluciones**:
1. Presionar `g` para mostrar/ocultar GUI
2. Verificar que `ofxGui` esté correctamente instalado
3. Verificar que el addon esté en `addons.make`

---

## Roadmap Detallado

### Fase 4: Colisiones y Eventos (✅ Completada)

**Objetivos**:
- ✅ Detección de colisiones con bordes de ventana
- ✅ Sistema de rebote físico con coeficiente de restitución
- ✅ Generación de eventos `/hit` con información de colisión
- ✅ Cooldown por partícula para evitar spam
- ✅ Rate limiting global (token bucket)
- ✅ Cálculo de energía de impacto (velocidad + distancia)

**Entregables**:
- ✅ Función `checkCollisions()` en `ofApp`
- ✅ Estructura de datos `HitEvent` para eventos
- ✅ Sistema de cooldown por partícula
- ✅ Rate limiter con token bucket
- ✅ Cálculo de energía basado en velocidad y distancia
- ✅ Estadísticas de debug (hits/seg, descartados, tokens)

### Fase 5: Comunicación OSC (Pendiente)

**Objetivos**:
- Integración de `ofxOsc`
- Envío de mensajes `/hit` al sintetizador
- Configuración de host y puerto (GUI o archivo)
- Validación de mensajes antes de envío
- (Opcional) Envío de mensajes `/state` periódicos

**Entregables**:
- Clase o estructura para manejo OSC
- Configuración de red
- Integración con sistema de eventos

### Fase 3b: Integración MediaPipe (Opcional/Tardía)

**Objetivos**:
- Integración de MediaPipe para tracking
- Detección de manos o pose corporal
- Mapeo de tracking a `MouseEfector` (interfaz común)
- Calibración y ajuste de parámetros

**Nota**: Esta fase se implementará solo después de completar el loop modular completo (partículas → OSC → JUCE).

---

## Documentación Relacionada

- **[Contrato OSC](../api-osc.md)** — Contrato de mensajes OSC (cuando se implemente)
- **[Especificación técnica completa](../specs/spec.md)** — Arquitectura general del sistema
- **[Plan de implementación](../specs/PLAN_IMPLEMENTACION.md)** — Fases de desarrollo

---

## Referencias Rápidas

- **Descripción general**: [readme.md](readme.md)
- **Cómo usar**: [manual.md](manual.md)
- **Detalles técnicos**: [spec.md](spec.md)
- **Estado del proyecto**: [../overview/ESTADO_ACTUAL.md](../overview/ESTADO_ACTUAL.md)

---

**Última actualización**: 2026-01-20

---

## Notas Adicionales

### Convenciones de Código

- **Nomenclatura**: Variables en `snake_case`, clases en `PascalCase`
- **Estructuras**: `MouseEfector` para input, `Particle` para partículas
- **Parámetros**: Todos ajustables en tiempo real vía GUI

### Extensiones Futuras

- **Múltiples efectores**: Soporte para varios puntos de input simultáneos
- **Diferentes tipos de partículas**: Partículas con propiedades físicas distintas
- **Visualización avanzada**: Líneas de conexión, trails, efectos visuales
- **Presets**: Guardar/cargar configuraciones de parámetros

### Contribuciones

Para contribuir o reportar problemas:
1. Revisar la documentación técnica en [`spec.md`](spec.md)
2. Seguir las convenciones de código existentes
3. Documentar cambios en código y documentación
