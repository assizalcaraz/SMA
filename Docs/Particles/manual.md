# Particles — Manual de Usuario

Manual de usuario del módulo de partículas. Guía completa de parámetros y uso.

**Última actualización:** v0.2 - Controles visuales y cámara agregados

**Ver también:**
- [`readme.md`](readme.md) - Descripción general del módulo
- [`spec.md`](spec.md) - Especificación técnica para desarrolladores

---

## Parámetros de Sistema

### N Particles

**Slider:** `N Particles`  
**Tipo:** `int`  
**Rango:** 500 - 8000  
**Valor por defecto:** 2000  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Número total de partículas en el sistema. Controla la densidad visual y el rendimiento.

**Efectos:**
- **Valores bajos (500-1000):** Menos partículas, mejor rendimiento, visual más espaciado
- **Valores medios (2000-3000):** Balance entre densidad y rendimiento (recomendado)
- **Valores altos (5000-8000):** Alta densidad visual, puede impactar FPS

**Nota:** El cambio es instantáneo y seguro. Las partículas se redistribuyen automáticamente en un grid con jitter.

---

## Parámetros Visuales

### particle_size

**Slider:** `particle_size`  
**Tipo:** `float`  
**Rango:** 1.0 - 10.0  
**Valor por defecto:** 2.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Tamaño de las partículas en píxeles. Controla el tamaño visual de cada partícula renderizada.

**Mapeo conceptual:**
- **Partículas grandes (6.0-10.0):** Representan frecuencias graves, sonido más profundo
- **Partículas pequeñas (1.0-3.0):** Representan frecuencias agudas, sonido más brillante
- **Partículas medianas (3.0-6.0):** Representan frecuencias medias, balance

**Efectos:**
- **Valores bajos (1.0-2.0):** Partículas pequeñas, mayor densidad visual, mejor rendimiento
- **Valores medios (2.0-5.0):** Balance entre visibilidad y densidad
- **Valores altos (6.0-10.0):** Partículas grandes, menor densidad visual, puede impactar rendimiento

**Uso:** Ajustar según el efecto visual deseado. Partículas grandes = timbre más grave, partículas pequeñas = timbre más agudo.

---

### Controles de Cámara

El sistema incluye controles de cámara para cambiar la vista del sistema de partículas.

#### camera_zoom

**Slider:** `camera_zoom`  
**Tipo:** `float`  
**Rango:** 0.1 - 5.0  
**Valor por defecto:** 1.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Controla el nivel de zoom de la cámara. Valores mayores a 1.0 acercan la vista, valores menores alejan.

**Efectos:**
- **Valores bajos (0.1-0.5):** Vista amplia, se ven más partículas pero más pequeñas
- **Valor 1.0:** Vista normal, sin zoom
- **Valores altos (2.0-5.0):** Zoom in, se ven menos partículas pero más grandes

#### camera_rotation

**Slider:** `camera_rotation`  
**Tipo:** `float`  
**Rango:** -180.0 - 180.0  
**Valor por defecto:** 0.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Rotación de la cámara en grados. Permite rotar la vista del sistema de partículas.

**Efectos:**
- **0°:** Vista normal, sin rotación
- **Valores positivos:** Rotación en sentido horario
- **Valores negativos:** Rotación en sentido antihorario
- **±180°:** Vista invertida

#### Presets de Cámara (Teclas 1-4)

El sistema incluye 4 presets de cámara que se pueden activar con las teclas numéricas:

- **Tecla 1:** Vista normal (zoom=1.0, rotación=0°)
- **Tecla 2:** Zoom in centro (zoom=2.0, rotación=0°)
- **Tecla 3:** Rotación 45° (zoom=1.0, rotación=45°)
- **Tecla 4:** Vista amplia (zoom=0.5, rotación=0°)

Los presets actualizan automáticamente los sliders de cámara en la GUI.

---

## Parámetros de Física

### k_home

**Slider:** `k_home`  
**Tipo:** `float`  
**Rango:** 0.5 - 6.0  
**Valor por defecto:** 2.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Constante de fuerza de retorno al origen. Controla qué tan rápido las partículas regresan a su posición de reposo (`home`).

**Fórmula:** `F_home = k_home * (home - pos)`

**Efectos:**
- **Valores bajos (0.5-1.5):** Retorno lento y suave, partículas se mueven más libremente
- **Valores medios (2.0-3.5):** Retorno moderado, balance entre libertad y estabilidad
- **Valores altos (4.0-6.0):** Retorno rápido y rígido, partículas vuelven rápidamente a home

**Uso:** Ajustar según la sensación deseada. Valores altos = sistema más "elástico", valores bajos = más "fluido".

---

### k_drag

**Slider:** `k_drag`  
**Tipo:** `float`  
**Rango:** 0.5 - 3.0  
**Valor por defecto:** 1.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Constante de amortiguación (drag). Controla la resistencia al movimiento, simulando fricción o viscosidad.

**Fórmula:** `F_drag = -k_drag * vel`

**Efectos:**
- **Valores bajos (0.5-1.0):** Poca fricción, partículas mantienen movimiento más tiempo
- **Valores medios (1.0-2.0):** Amortiguación moderada, movimiento natural
- **Valores altos (2.0-3.0):** Alta fricción, partículas se detienen rápidamente

**Uso:** Controlar la "inercia" del sistema. Valores altos = movimiento más controlado, valores bajos = más dinámico.

---

## Parámetros de Gesto (Input)

### k_gesture

**Slider:** `k_gesture`  
**Tipo:** `float`  
**Rango:** 0.0 - 200.0  
**Valor por defecto:** 50.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Fuerza del gesto. Controla la intensidad con la que el movimiento del mouse (o gesto) afecta a las partículas.

**Fórmula:** `F_gesture = k_gesture * w * speed * dir`

Donde:
- `w` = influencia gaussiana por distancia
- `speed` = velocidad normalizada del gesto (0..1)
- `dir` = dirección normalizada del gesto

**Efectos:**
- **Valor 0.0:** Desactiva completamente la fuerza de gesto
- **Valores bajos (10-30):** Efecto sutil, partículas responden levemente
- **Valores medios (50-100):** Respuesta moderada y natural
- **Valores altos (150-200):** Efecto fuerte, partículas responden agresivamente al gesto

**Uso:** Ajustar según la sensibilidad deseada. Valores altos = gestos más dramáticos, valores bajos = más sutiles.

---

### sigma

**Slider:** `sigma`  
**Tipo:** `float`  
**Rango:** 50.0 - 500.0  
**Valor por defecto:** 150.0  
**Unidad:** pixels  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Radio de influencia del gesto. Controla qué tan lejos del mouse/gesto las partículas son afectadas. Usa una función gaussiana para suavizar la influencia.

**Fórmula:** `w = exp(-(r²)/(2*sigma²))`

Donde `r` es la distancia desde la partícula al efector (mouse/gesto).

**Efectos:**
- **Valores bajos (50-100):** Influencia muy localizada, solo partículas cercanas se mueven
- **Valores medios (150-250):** Zona de influencia moderada, efecto visible pero controlado
- **Valores altos (300-500):** Influencia amplia, muchas partículas responden al gesto

**Uso:** Controlar el "alcance" del gesto. Valores altos = efecto más global, valores bajos = más preciso.

**Nota:** El efecto es suave gracias a la función gaussiana. Partículas muy lejanas tienen influencia casi nula.

---

### speed_ref

**Slider:** `speed_ref`  
**Tipo:** `float`  
**Rango:** 100.0 - 2000.0  
**Valor por defecto:** 500.0  
**Unidad:** pixels/segundo  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Velocidad de referencia para normalizar la velocidad del gesto. Usado para calcular la intensidad del gesto independientemente de la velocidad absoluta.

**Fórmula:** `speed = clamp(|vel_mouse| / speed_ref, 0..1)`

**Efectos:**
- **Valores bajos (100-300):** Gesto se considera "rápido" con poca velocidad, respuesta más sensible
- **Valores medios (500-800):** Sensibilidad moderada, gestos normales producen efecto medio
- **Valores altos (1200-2000):** Requiere gestos muy rápidos para máxima intensidad, respuesta menos sensible

**Uso:** Ajustar según la velocidad típica de los gestos. Si los gestos son lentos, usar valores bajos. Si son rápidos, valores altos.

**Ejemplo:** Con `speed_ref = 500`, un gesto a 500 px/s produce `speed = 1.0` (máxima intensidad). Un gesto a 250 px/s produce `speed = 0.5` (intensidad media).

---

## Parámetros de Colisiones

### restitution

**Slider:** `restitution`  
**Tipo:** `float`  
**Rango:** 0.2 - 0.85  
**Valor por defecto:** 0.6  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Coeficiente de restitución (rebote). Controla qué tan "elástico" es el rebote de las partículas al chocar con los bordes.

**Fórmula:** `vel_after = -restitution * vel_before`

**Efectos:**
- **Valores bajos (0.2-0.4):** Rebote muy amortiguado, partículas pierden mucha energía al chocar
- **Valores medios (0.5-0.7):** Rebote moderado, balance entre energía y control
- **Valores altos (0.75-0.85):** Rebote muy elástico, partículas mantienen mucha energía

**Uso:** Controlar el "rebote" del sistema. Valores altos = partículas más "saltarinas", valores bajos = más controladas.

---

### hit_cooldown (ms)

**Slider:** `hit_cooldown (ms)`  
**Tipo:** `float`  
**Rango:** 30.0 - 120.0  
**Valor por defecto:** 60.0  
**Unidad:** milisegundos  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Tiempo mínimo entre hits de la misma partícula. Previene spam de eventos cuando una partícula está "pegada" al borde.

**Efectos:**
- **Valores bajos (30-50ms):** Permite hits más frecuentes, más eventos pero puede generar spam
- **Valores medios (60-80ms):** Balance entre frecuencia y control
- **Valores altos (100-120ms):** Hits más espaciados, menos eventos pero más controlados

**Uso:** Ajustar según la frecuencia deseada de eventos. Valores bajos = más eventos, valores altos = más control.

---

### particle_radius

**Slider:** `particle_radius`  
**Tipo:** `float`  
**Rango:** 2.0 - 20.0  
**Valor por defecto:** 5.0  
**Unidad:** pixels  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Radio de colisión entre partículas. Controla la distancia mínima a la que dos partículas se consideran en colisión. La distancia de colisión efectiva es `2 * particle_radius`.

**Efectos:**
- **Valores bajos (2-5px):** Colisiones más precisas, solo cuando partículas están muy cerca
- **Valores medios (5-10px):** Balance entre precisión y frecuencia de colisiones
- **Valores altos (15-20px):** Colisiones más frecuentes, partículas "grandes" que colisionan fácilmente

**Uso:** Ajustar según la densidad de partículas y frecuencia deseada de colisiones. Valores bajos = menos colisiones pero más precisas, valores altos = más colisiones pero menos precisas.

**Nota:** Este parámetro solo afecta colisiones partícula-partícula, no colisiones con bordes.

---

### enable_particle_collisions

**Toggle:** `enable_particle_collisions`  
**Tipo:** `bool`  
**Valor por defecto:** `true` (habilitado)  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Habilita o deshabilita la detección de colisiones entre partículas. Cuando está deshabilitado, solo se detectan colisiones con los bordes de la ventana.

**Efectos:**
- **Habilitado (true):** Las partículas pueden colisionar entre sí, generando eventos adicionales
- **Deshabilitado (false):** Solo se detectan colisiones con bordes

**Uso:** Deshabilitar si quieres reducir la cantidad de eventos o si las colisiones partícula-partícula generan demasiado ruido. Habilitar para un comportamiento más dinámico y rico en eventos.

**Nota:** Las colisiones partícula-partícula generan eventos con `surface = -1` para distinguirlos de colisiones con bordes.

---

## Parámetros de Energía

### vel_ref

**Slider:** `vel_ref`  
**Tipo:** `float`  
**Rango:** 300.0 - 1000.0  
**Valor por defecto:** 500.0  
**Unidad:** pixels/segundo  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Velocidad de referencia para calcular la energía del impacto. Usado para normalizar la velocidad de la partícula antes del choque.

**Fórmula:** `speed_norm = clamp(|vel_pre| / vel_ref, 0..1)`

**Efectos:**
- **Valores bajos (300-400):** Velocidades normales se consideran "altas", más energía en hits
- **Valores medios (500-700):** Sensibilidad moderada
- **Valores altos (800-1000):** Requiere velocidades muy altas para máxima energía

**Uso:** Ajustar según la velocidad típica de las partículas. Si las partículas se mueven rápido, aumentar. Si se mueven lento, disminuir.

---

### dist_ref

**Slider:** `dist_ref`  
**Tipo:** `float`  
**Rango:** 20.0 - 100.0  
**Valor por defecto:** 50.0  
**Unidad:** pixels  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Distancia de referencia para calcular la energía del impacto. Usado para normalizar la distancia recorrida desde el último hit.

**Fórmula:** `dist_norm = clamp(dist_traveled / dist_ref, 0..1)`

**Efectos:**
- **Valores bajos (20-40):** Distancias cortas se consideran "largas", más energía en hits
- **Valores medios (50-70):** Sensibilidad moderada
- **Valores altos (80-100):** Requiere distancias largas para máxima energía

**Uso:** Ajustar según la escala de movimiento. Si las partículas se mueven en distancias cortas, disminuir. Si se mueven en distancias largas, aumentar.

---

### energy_a

**Slider:** `energy_a`  
**Tipo:** `float`  
**Rango:** 0.5 - 0.9  
**Valor por defecto:** 0.7  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Peso de la velocidad en el cálculo de energía. Controla qué tan importante es la velocidad vs. la distancia.

**Fórmula:** `energy = clamp(energy_a * speed_norm + energy_b * dist_norm, 0..1)`

**Efectos:**
- **Valores bajos (0.5-0.6):** Menos peso a velocidad, más peso a distancia
- **Valores medios (0.7-0.8):** Balance entre velocidad y distancia
- **Valores altos (0.85-0.9):** Más peso a velocidad, menos a distancia

**Uso:** Si quieres que los hits rápidos tengan más energía, aumentar. Si quieres que los hits de partículas que viajaron lejos tengan más energía, disminuir.

---

### energy_b

**Slider:** `energy_b`  
**Tipo:** `float`  
**Rango:** 0.1 - 0.5  
**Valor por defecto:** 0.3  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Peso de la distancia en el cálculo de energía. Controla qué tan importante es la distancia vs. la velocidad.

**Fórmula:** `energy = clamp(energy_a * speed_norm + energy_b * dist_norm, 0..1)`

**Efectos:**
- **Valores bajos (0.1-0.2):** Menos peso a distancia, más peso a velocidad
- **Valores medios (0.3-0.4):** Balance entre distancia y velocidad
- **Valores altos (0.45-0.5):** Más peso a distancia, menos a velocidad

**Uso:** Complementario a `energy_a`. Generalmente `energy_a + energy_b ≈ 1.0` para balance.

**Nota:** `energy_a` y `energy_b` trabajan juntos. Aumentar uno generalmente implica disminuir el otro para mantener balance.

---

## Parámetros de Rate Limiting

### max_hits/s

**Slider:** `max_hits/s`  
**Tipo:** `float`  
**Rango:** 50.0 - 500.0  
**Valor por defecto:** 200.0  
**Unidad:** hits por segundo  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Máximo de hits permitidos por segundo (tasa de tokens). Controla la frecuencia global de eventos usando un sistema de token bucket.

**Efectos:**
- **Valores bajos (50-100):** Menos eventos por segundo, más controlado
- **Valores medios (150-250):** Balance entre frecuencia y control
- **Valores altos (300-500):** Muchos eventos por segundo, más dinámico

**Uso:** Ajustar según la capacidad del sistema receptor (OSC/JUCE). Valores altos = más eventos pero más carga.

---

### burst

**Slider:** `burst`  
**Tipo:** `float`  
**Rango:** 100.0 - 500.0  
**Valor por defecto:** 300.0  
**Unidad:** tokens  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Máximo de tokens acumulados (burst máximo). Permite picos temporales de eventos por encima de la tasa promedio.

**Efectos:**
- **Valores bajos (100-200):** Picos más limitados, más uniforme
- **Valores medios (250-350):** Permite picos moderados
- **Valores altos (400-500):** Permite picos grandes de eventos

**Uso:** Controlar la "capacidad de pico" del sistema. Valores altos = permite ráfagas de eventos, valores bajos = más uniforme.

**Nota:** El sistema acumula tokens a la tasa de `max_hits/s`, pero puede tener hasta `burst` tokens acumulados.

---

### max_hits/frame

**Slider:** `max_hits/frame`  
**Tipo:** `int`  
**Rango:** 5 - 20  
**Valor por defecto:** 10  
**Unidad:** hits por frame  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Máximo de hits procesados en un solo frame. Previene sobrecarga en frames con muchas colisiones simultáneas.

**Efectos:**
- **Valores bajos (5-8):** Menos hits por frame, más distribuidos
- **Valores medios (10-15):** Balance entre procesamiento y distribución
- **Valores altos (18-20):** Más hits por frame, puede causar picos de carga

**Uso:** Controlar la carga por frame. Valores altos = más hits simultáneos pero más carga, valores bajos = más distribuido.

---

## Parámetros Internos (No ajustables en GUI)

### smooth_alpha

**Tipo:** `float`  
**Valor:** 0.15 (fijo)  
**Rango recomendado:** 0.1 - 0.25

**Descripción:**  
Factor de suavizado para la posición del mouse. Controla qué tan suave es el seguimiento del input.

**Fórmula:** `pos_smooth = pos_smooth * (1 - α) + pos_raw * α`

**Nota:** Actualmente fijo en 0.15. Puede hacerse ajustable en el futuro si es necesario.

---

## Interacciones entre Parámetros

### Balance k_home / k_drag

- **k_home alto + k_drag bajo:** Sistema muy elástico, partículas rebotan mucho antes de volver a home
- **k_home bajo + k_drag alto:** Sistema más controlado, partículas se detienen rápido
- **k_home medio + k_drag medio:** Balance natural (valores por defecto)

### Balance k_gesture / sigma

- **k_gesture alto + sigma bajo:** Efecto muy localizado pero intenso
- **k_gesture bajo + sigma alto:** Efecto amplio pero sutil
- **k_gesture medio + sigma medio:** Respuesta balanceada (valores por defecto)

### speed_ref y sensibilidad

- Ajustar `speed_ref` según la velocidad típica de gestos:
  - Gestos lentos y precisos → `speed_ref` bajo (200-400)
  - Gestos normales → `speed_ref` medio (500-800)
  - Gestos rápidos y amplios → `speed_ref` alto (1000-2000)

---

## Valores Recomendados por Escenario

### Escenario 1: Sistema Suave y Fluido
```
N Particles: 2000
k_home: 1.5
k_drag: 0.8
k_gesture: 30
sigma: 200
speed_ref: 400
```

### Escenario 2: Sistema Responsivo y Dinámico (Por defecto)
```
N Particles: 2000
k_home: 2.0
k_drag: 1.0
k_gesture: 50
sigma: 150
speed_ref: 500
```

### Escenario 3: Sistema Elástico y Dramático
```
N Particles: 3000
k_home: 4.0
k_drag: 1.5
k_gesture: 100
sigma: 100
speed_ref: 600
```

### Escenario 4: Sistema de Alta Densidad
```
N Particles: 5000
k_home: 2.5
k_drag: 1.2
k_gesture: 60
sigma: 180
speed_ref: 500
```

### Escenario 5: Sistema con Colisiones Activas (Configuración Interesante)
```
N Particles: 8000
k_home: 0.5275
k_drag: 0.5875
k_gesture: 200
sigma: 131
speed_ref: 100
restitution: 0.45675
hit_cooldown: 60 ms
vel_ref: 702.5
dist_ref: 96.4
energy_a: 0.5
energy_b: 0.1
max_hits/s: 70.25
burst: 132
max_hits/frame: 9
```

**Nota:** Esta configuración está optimizada para generar colisiones frecuentes con gestos normales, usando valores bajos de `k_home` y `k_drag` para permitir más movimiento libre, y `k_gesture` alto para empujar partículas hacia los bordes.

---

## Notas de Implementación

- Todos los parámetros son ajustables en tiempo real sin reiniciar la aplicación
- Los cambios se aplican inmediatamente en el siguiente frame
- El sistema está optimizado para mantener FPS ≥ 45 con N=2000 partículas
- Los valores por defecto están calibrados para una experiencia balanceada

---

## Historial de Cambios

**Fase 2 (Partículas básicas):**
- Agregados: `N Particles`, `k_home`, `k_drag`

**Fase 3 (Input mouse):**
- Agregados: `k_gesture`, `sigma`, `speed_ref`
- Documentación inicial creada

**Fase 4 (Colisiones y eventos):**
- Agregados: `restitution`, `hit_cooldown`, `vel_ref`, `dist_ref`, `energy_a`, `energy_b`
- Agregados: `max_hits/s`, `burst`, `max_hits/frame`
- Sistema de detección de colisiones con bordes
- Sistema de rate limiting (token bucket)
- Cálculo de energía de impacto
- Generación de eventos de hit

**v0.2 (Mejoras visuales):**
- Agregado: `particle_size` - Control de tamaño de partículas con mapeo conceptual (grandes=graves, pequeñas=agudos)
- Agregado: `camera_zoom` - Control de zoom de cámara
- Agregado: `camera_rotation` - Control de rotación de cámara
- Agregados: Presets de cámara (teclas 1-4)
- Mejoras: Contadores de diagnóstico para análisis de renderizado

---

**Estado:** Comunicación OSC (Fase 5) ✅ completada - Los eventos se envían automáticamente al sintetizador JUCE en puerto 9000.
