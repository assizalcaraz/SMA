# Particles — Manual de Usuario

Manual de usuario del módulo de partículas. Guía completa de parámetros y uso.

**Última actualización:** Fase 3 completada (input mouse)

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

---

**Próximas fases:** Se agregarán parámetros de colisiones, rate limiting y OSC en fases futuras.
