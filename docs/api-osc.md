# API OSC — Contrato de Comunicación

Documentación detallada del protocolo de comunicación OSC entre App A (openFrameworks) y App B (JUCE).

---

## Configuración de transporte

### Protocolo y red

- **Protocolo:** UDP (User Datagram Protocol)
- **Formato:** OSC (Open Sound Control) 1.0
- **Host por defecto:** `127.0.0.1` (localhost)
- **Puerto por defecto:** `9000`
- **Dirección completa:** `osc.udp://127.0.0.1:9000`

### Configuración

Ambas aplicaciones deben usar la misma configuración de red:
- App A (oF) actúa como **cliente** (envía mensajes)
- App B (JUCE) actúa como **servidor** (recibe mensajes)

Los valores de host y puerto deben ser configurables en ambas aplicaciones.

---

## Mensajes OSC

### `/hit` — Evento de impacto

**Dirección:** `/hit`

**Descripción:** Mensaje principal del sistema. Se envía cuando una partícula colisiona con un borde y genera un evento sonoro. Este mensaje contiene toda la información necesaria para generar el sonido correspondiente.

**Contrato congelado (definitivo):**

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `int32` | `id`      | 0 - N      | Identificador único de la partícula            |
| 2     | `float` | `x`       | 0.0 - 1.0  | **Posición X normalizada de IMPACTO** (donde ocurre la colisión, NO posición del efector) |
| 3     | `float` | `y`       | 0.0 - 1.0  | **Posición Y normalizada de IMPACTO** (donde ocurre la colisión, NO posición del efector) |
| 4     | `float` | `energy`  | 0.0 - 1.0  | Energía del impacto (mapeo continuo, ver cálculo abajo) |
| 5     | `int32` | `surface` | 0-3, -1    | Superficie impactada (ver tabla de superficies)|

**Cálculo de energía (mapeo continuo):**

La energía se calcula como combinación de velocidad y distancia:

```
speed_norm = |v_pre| / vel_ref
dist_norm = distance_traveled / dist_ref
energy = clamp(a * speed_norm + b * dist_norm, 0.0, 1.0)
```

Donde:
- `v_pre` = velocidad PRE-colisión (antes de aplicar restitución)
- `vel_ref` = velocidad de referencia (valor inicial recomendado: 500 px/s) — **tuneable por escena/estética**
- `distance_traveled` = distancia recorrida desde último hit (o desde último frame si es primer hit)
- `dist_ref` = distancia de referencia (valor inicial recomendado: 50 px) — **tuneable por escena/estética**
- `a`, `b` = pesos configurables (valores iniciales recomendados: a=0.7, b=0.3) — **tuneables por escena/estética**

**Importante:** Todas las colisiones producen sonido, incluso micro-colisiones ("scrapes"). La energía muy baja se mapea a amplitud muy baja pero audible (no mute), creando textura granular "coin cascade".

**Nota sobre scrapes:** Los roces (scrapes) de partículas pegadas al borde **no son hits fuertes**; comparten el mismo mensaje `/hit` con energía muy baja. No hay mensaje OSC separado para scrapes.

**Superficies (`surface`):**

| Valor | Nombre           | Descripción              |
|-------|------------------|--------------------------|
| `-1`  | N/A              | No aplica o desconocido  |
| `0`   | Left (L)         | Borde izquierdo          |
| `1`   | Right (R)        | Borde derecho            |
| `2`   | Top (T)          | Borde superior           |
| `3`   | Bottom (B)       | Borde inferior           |

**Ejemplo de mensaje:**

```
/hit 42 0.75 0.3 0.65 1
```

**Interpretación:**
- Partícula ID 42
- Posición: (0.75, 0.3) — derecha, parte superior
- Energía: 0.65 — impacto moderado-alto
- Superficie: 1 — borde derecho

**Frecuencia esperada:**
- Máximo: ~200 hits/segundo (con cooldown activo)
- Típico: 10-50 hits/segundo durante interacción normal
- Mínimo: 0 hits/segundo (sin interacción)

**Validación:**
- App B debe validar que todos los parámetros estén en rango
- Valores fuera de rango deben ser clampeados o ignorados
- Mensajes malformados deben ser descartados sin crashear

---

### `/state` — Estado global

**Dirección:** `/state`

**Descripción:** Mensaje opcional que proporciona información agregada del sistema. Puede enviarse periódicamente (p.ej. cada 100ms) o cuando cambian significativamente los valores.

**Parámetros:**

| Orden | Tipo    | Nombre     | Rango     | Descripción                                    |
|-------|---------|------------|-----------|------------------------------------------------|
| 1     | `float` | `activity` | 0.0 - 1.0 | Actividad normalizada (hits por segundo)      |
| 2     | `float` | `gesture`   | 0.0 - 1.0 | Energía de gesto agregada                      |
| 3     | `float` | `presence`  | 0.0 - 1.0 | Confianza del tracking (0 = sin usuario)      |

**Cálculo de parámetros:**

- **`activity`:** 
  - `activity = clamp(hits_per_second / max_hits_per_second, 0.0, 1.0)`
  - `max_hits_per_second` típicamente 100-200

- **`gesture`:**
  - Agregación de energía de gestos detectados
  - Normalizada según velocidad máxima esperada

- **`presence`:**
  - Confianza del tracking MediaPipe
  - 0.0 = no hay usuario detectado
  - 1.0 = tracking perfecto

**Ejemplo de mensaje:**

```
/state 0.45 0.32 0.89
```

**Interpretación:**
- Actividad: 45% del máximo
- Gesto: 32% de energía
- Presencia: 89% de confianza (usuario bien detectado)

**Frecuencia recomendada:**
- 10 Hz (cada 100ms) durante actividad
- 1 Hz (cada 1s) cuando está inactivo
- Opcional: solo cuando cambia significativamente (>10%)

**Uso en App B:**
- `activity` → wet global, densidad de reverb
- `gesture` → drive/saturación global
- `presence` → master level (bajar si presencia baja)

---

### `/ctrl` — Control remoto

**Dirección:** `/ctrl`

**Descripción:** Mensaje opcional que permite controlar parámetros de App A desde App B. **No es necesario para el MVP.**

**Parámetros:**

| Orden | Tipo     | Nombre  | Descripción                    |
|-------|----------|---------|--------------------------------|
| 1     | `string` | `key`   | Nombre del parámetro a cambiar |
| 2     | `float`  | `value` | Nuevo valor del parámetro     |

**Parámetros controlables:**

| Key          | Tipo    | Rango        | Descripción                    |
|--------------|---------|--------------|--------------------------------|
| `k_home`     | `float` | 0.5 - 6.0    | Constante de retorno al origen |
| `k_gesture`  | `float` | 0.0 - 10.0   | Fuerza de gesto                |
| `sigma`      | `float` | 0.01 - 0.5   | Radio de influencia            |
| `k_drag`     | `float` | 0.5 - 3.0    | Amortiguación                  |
| `restitution`| `float` | 0.2 - 0.85   | Coeficiente de rebote          |
| `hit_threshold` | `float` | 0.08 - 0.2  | Umbral de energía para hit     |
| `hit_cooldown_ms` | `float` | 30 - 120 | Cooldown en milisegundos   |

**Ejemplo de mensaje:**

```
/ctrl k_home 2.5
/ctrl hit_threshold 0.12
```

**Nota:** Este mensaje es bidireccional. App B puede enviarlo a App A si App A tiene un receptor OSC configurado. Para el MVP, este mensaje no es necesario.

---

## Mapeo de parámetros en App B (JUCE)

**Diseño sonoro objetivo: "Coin Cascade" (cascada de monedas)**

Textura de múltiples hits percusivos metálicos breves que se acumulan en una cascada granular. Cada colisión produce sonido, desde micro-scrapes (casi imperceptibles) hasta golpes fuertes.

### Mapeo desde `/hit`

| Parámetro OSC | Mapeo en JUCE                    | Fórmula                                    |
|---------------|----------------------------------|--------------------------------------------|
| `energy`      | Amplitud                         | `amp = energy^γ` (γ = 1.2-2.0)            |
|               |                                  | Micro-colisiones → amplitud muy baja pero audible (no mute) |
|               |                                  | Golpes fuertes → amplitud alta            |
| `energy`      | Brightness                       | `brightness = lerp(minBright, maxBright, energy)` |
|               |                                  | Colisiones más energéticas = más brillantes |
| `x`           | Pan                              | `pan = (x * 2) - 1` (-1 = izquierda, 1 = derecha) |
|               |                                  | Distribución espacial de la cascada        |
| `y`           | Damping                          | `damping = lerp(short, long, 1 - y)`      |
|               |                                  | Partes superiores = más secas, inferiores = más reverberantes |
| `y`           | Base Frequency (opcional)        | `freq = baseFreq * (1 + y * deviation)`   |
|               |                                  | Diferentes bandas de pitch según posición vertical |
| `surface`     | Modulación opcional               | Diferentes timbres según superficie       |

### Mapeo desde `/state`

| Parámetro OSC | Mapeo en JUCE                    | Fórmula                                    |
|---------------|----------------------------------|--------------------------------------------|
| `activity`    | Global Wet (Reverb)               | `wet = activity`                           |
| `gesture`     | Global Drive/Saturación          | `drive = gesture`                           |
| `presence`    | Master Level                      | `master = presence` (bajar si < 0.5)      |

---

## Implementación de referencia

### App A (openFrameworks) — Envío

```cpp
// Configuración
ofxOscSender sender;
sender.setup("127.0.0.1", 9000);

// Enviar evento /hit
ofxOscMessage msg;
msg.setAddress("/hit");
msg.addIntArg(particleId);
msg.addFloatArg(normalizedX);
msg.addFloatArg(normalizedY);
msg.addFloatArg(energy);
msg.addIntArg(surfaceId);
sender.sendMessage(msg);

// Enviar estado /state (opcional)
ofxOscMessage stateMsg;
stateMsg.setAddress("/state");
stateMsg.addFloatArg(activity);
stateMsg.addFloatArg(gesture);
stateMsg.addFloatArg(presence);
sender.sendMessage(stateMsg);
```

### App B (JUCE) — Recepción

```cpp
// Configuración
juce::OSCReceiver receiver;
receiver.connect(9000);

// Callback para /hit
void OSCMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern().matches("/hit")) {
        if (message.size() == 5) {
            int id = message[0].getInt32();
            float x = message[1].getFloat32();
            float y = message[2].getFloat32();
            float energy = message[3].getFloat32();
            int surface = message[4].getInt32();
            
            // Procesar evento
            triggerSound(id, x, y, energy, surface);
        }
    }
    else if (message.getAddressPattern().matches("/state")) {
        if (message.size() == 3) {
            float activity = message[0].getFloat32();
            float gesture = message[1].getFloat32();
            float presence = message[2].getFloat32();
            
            // Actualizar parámetros globales
            updateGlobalParams(activity, gesture, presence);
        }
    }
}
```

---

## Testing y debugging

### Herramientas recomendadas

1. **OSCulator** (macOS) — Monitor y debug de mensajes OSC
2. **OSC Monitor** (JUCE) — Herramienta incluida en JUCE
3. **Python script** — Ver `scripts/test-osc.py` para script de prueba

### Script de prueba

Usar `scripts/test-osc.py` para enviar mensajes OSC simulados y probar App B sin necesidad de ejecutar App A.

### Validación

- Verificar que los mensajes lleguen correctamente
- Validar rangos de parámetros
- Medir latencia end-to-end (debe ser < 10ms típicamente)
- Verificar que no haya pérdida de mensajes bajo carga

---

## Consideraciones de rendimiento

### Densidad de eventos

- **Máximo sostenible:** ~200 hits/segundo
- **Típico:** 10-50 hits/segundo
- **Anti-spam:** Cooldown por partícula (30-120ms)

### Latencia

- **Objetivo:** < 10ms end-to-end
- **OSC overhead:** ~1-2ms típicamente
- **Procesamiento:** Depende de App B

### Manejo de saturación

Si App B recibe demasiados eventos:
- **Voice stealing strategy:**
  - Robar voz más antigua O voz con menor amplitud residual
  - Priorizar mantener textura "cascade" sin gaps
- **Master limiter:**
  - Limiter suave en salida master
  - Prevenir clipping bajo carga alta (200 hits/s)
- Rate limiting ya implementado en oF (token bucket)

---

## Versión

**Versión del protocolo:** 1.0  
**Última actualización:** Febrero 2026  
**Compatibilidad:** Backward compatible con versiones futuras (nuevos mensajes opcionales)

---

## Referencias

- [Especificación OSC 1.0](https://opensoundcontrol.stanford.edu/)
- [Especificación técnica del proyecto](../spec.md)
- [README principal](../readme.md)
