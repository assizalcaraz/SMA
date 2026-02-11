# CALIB — Especificación Técnica

Especificación técnica detallada del módulo de calibración y validación MAAD-2-CALIB. Documentación para desarrolladores.

**Última actualización:** v0.4 (2026-02-11)

---

## Estructura del Código

### Mapeo Archivos → Funcionalidad

| Archivo | Responsabilidad |
|---------|----------------|
| `maad-2-calib/src/main.cpp` | Punto de entrada de la aplicación (placeholder, implementación pendiente) |
| `maad-2-calib/notebooks/analysis_template.ipynb` | Template de análisis Jupyter con técnicas de procesamiento de señales |

---

## Objetivo

MAAD-2-CALIB es un módulo de calibración y validación diseñado para servir como trabajo final integrador de MAAD-2. Proporciona tres responsabilidades principales:

1. **CONTROL:** Orquestación de transporte OSC para sesiones de calibración reproducibles
2. **REGISTRATION:** Captura y registro de datos (NDJSON + WAV + metadata)
3. **ANALYSIS:** Análisis offline basado en Python/Jupyter con técnicas de procesamiento de señales

---

## Arquitectura

### Flujo de datos

```
oF (Particles) → JUCE (Synthesis) → CALIB (Control + Registration) → Notebook (Analysis) → Report
```

### Responsabilidades

- **CONTROL:** Envía comandos OSC (`/test/*`) para orquestar sesiones de prueba
- **REGISTRATION:** Captura eventos OSC (`/hit`, `/state`, `/plate`) y audio, los registra en formato estructurado
- **ANALYSIS:** Procesamiento offline con herramientas de análisis espectral y validación estadística

---

## Contrato OSC de Control

MAAD-2-CALIB actúa como **cliente OSC** que envía comandos de control a las aplicaciones existentes (oF y JUCE) para orquestar sesiones de calibración reproducibles.

### Configuración de transporte

- **Protocolo:** UDP (User Datagram Protocol)
- **Formato:** OSC (Open Sound Control) 1.0
- **Host por defecto:** `127.0.0.1` (localhost)
- **Puerto por defecto:** `9000` (compatible con sistema existente)
- **Dirección completa:** `osc.udp://127.0.0.1:9000`

### Mensajes de control

#### `/test/start` — Iniciar sesión de calibración

**Dirección:** `/test/start`

**Descripción:** Inicia una nueva sesión de calibración. CALIB comienza a registrar eventos y audio desde este momento.

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `string` | `run_id`  | -          | Identificador único de la sesión (opcional, si no se proporciona se genera automáticamente) |

**Ejemplo:**
```
/test/start "run_001"
```

**Comportamiento:**
- CALIB crea un directorio `runs/YYYYMMDD_HHMMSS/` (o usa `run_id` si se proporciona)
- Inicia captura de audio (si está disponible)
- Inicia registro de eventos OSC en `run.ndjson`
- Genera `meta.json` con timestamp de inicio y configuración

---

#### `/test/stop` — Detener sesión de calibración

**Dirección:** `/test/stop`

**Descripción:** Detiene la sesión de calibración actual. CALIB finaliza la captura y cierra los archivos.

**Parámetros:** Ninguno

**Ejemplo:**
```
/test/stop
```

**Comportamiento:**
- Detiene captura de audio
- Cierra `run.ndjson`
- Finaliza `meta.json` con timestamp de fin y duración total
- Guarda `audio.wav` si se capturó audio

---

#### `/test/seek` — Control de posición temporal

**Dirección:** `/test/seek`

**Descripción:** Controla la posición temporal en una sesión de calibración (útil para reproducibilidad y análisis de segmentos específicos).

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `float` | `time`    | >= 0.0     | Tiempo en segundos desde el inicio de la sesión |

**Ejemplo:**
```
/test/seek 15.5
```

**Comportamiento:**
- Si la sesión está en curso, marca un punto de referencia temporal
- Si se usa en modo reproducción, salta a la posición temporal especificada
- Se registra en `meta.json` como evento de control

**Nota:** Este mensaje es principalmente para análisis posterior y marcado de segmentos. No modifica el estado de la simulación en tiempo real.

---

#### `/test/rate` — Control de velocidad de reproducción

**Dirección:** `/test/rate`

**Descripción:** Controla la velocidad de reproducción de una sesión (útil para análisis a diferentes velocidades o validación de comportamiento).

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `float` | `rate`    | 0.1 - 2.0  | Factor de velocidad (1.0 = normal, 0.5 = mitad, 2.0 = doble) |

**Ejemplo:**
```
/test/rate 0.5
```

**Comportamiento:**
- Se registra en `meta.json` como parámetro de control
- En modo reproducción, ajusta la velocidad de procesamiento de eventos
- No afecta la simulación en tiempo real (solo análisis offline)

---

#### `/test/seed` — Establecer semilla para reproducibilidad

**Dirección:** `/test/seed`

**Descripción:** Establece la semilla de números aleatorios para garantizar reproducibilidad de sesiones de calibración.

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `int32` | `seed`    | Cualquier entero | Semilla para generador de números aleatorios |

**Ejemplo:**
```
/test/seed 42
```

**Comportamiento:**
- Se registra en `meta.json` como parámetro de configuración
- Si la simulación usa números aleatorios (ruido, jitter inicial, etc.), esta semilla garantiza reproducibilidad
- **Importante:** Este mensaje debe enviarse **antes** de `/test/start` para que tenga efecto

---

#### `/test/beep` — Señal de prueba/calibración

**Dirección:** `/test/beep`

**Descripción:** Genera una señal de prueba (beep) para calibración de niveles de audio y sincronización temporal.

**Parámetros:**

| Orden | Tipo    | Nombre    | Rango      | Descripción                                    |
|-------|---------|-----------|------------|------------------------------------------------|
| 1     | `float` | `freq`    | 20.0 - 20000.0 | Frecuencia en Hz (por defecto: 1000.0) |
| 2     | `float` | `duration` | 0.01 - 5.0 | Duración en segundos (por defecto: 0.5) |
| 3     | `float` | `amplitude` | 0.0 - 1.0 | Amplitud normalizada (por defecto: 0.5) |

**Ejemplo:**
```
/test/beep 1000.0 0.5 0.5
```

**Comportamiento:**
- CALIB puede generar un beep de prueba directamente, o
- CALIB puede enviar un mensaje OSC a JUCE para generar el beep (si JUCE implementa este handler)
- Se registra en `run.ndjson` como evento de calibración
- Útil para sincronización temporal entre eventos y audio

---

## Estructura de Artefactos de Salida

Cada sesión de calibración genera un directorio con timestamp y los siguientes archivos:

### Directorio de sesión

**Formato:** `runs/YYYYMMDD_HHMMSS/`

**Ejemplo:** `runs/20260215_143022/`

El directorio se crea automáticamente cuando se recibe `/test/start`.

---

### `run.ndjson` — Eventos de simulación

**Formato:** NDJSON (Newline Delimited JSON) — un objeto JSON por línea

**Descripción:** Registro línea por línea de todos los eventos OSC recibidos durante la sesión, con timestamps precisos.

**Estructura de cada línea (objeto JSON):**

```json
{
  "timestamp": 0.123456,
  "osc_address": "/hit",
  "osc_args": [42, 0.75, 0.3, 0.65, 1],
  "wall_clock": "2026-02-15T14:30:22.123456Z"
}
```

**Campos:**
- `timestamp`: Tiempo relativo desde inicio de sesión en segundos (float de alta precisión)
- `osc_address`: Dirección del mensaje OSC (string)
- `osc_args`: Array de argumentos del mensaje OSC (tipos preservados)
- `wall_clock`: Timestamp absoluto ISO 8601 (string)

**Ventajas de NDJSON:**
- Streaming: se puede escribir línea por línea sin cargar todo en memoria
- Procesamiento incremental: fácil de leer y procesar con herramientas estándar
- Compatible con herramientas de análisis (jq, pandas, etc.)

**Ejemplo de contenido:**
```json
{"timestamp": 0.000000, "osc_address": "/test/start", "osc_args": ["run_001"], "wall_clock": "2026-02-15T14:30:22.000000Z"}
{"timestamp": 0.123456, "osc_address": "/hit", "osc_args": [42, 0.75, 0.3, 0.65, 1], "wall_clock": "2026-02-15T14:30:22.123456Z"}
{"timestamp": 0.234567, "osc_address": "/hit", "osc_args": [43, 0.25, 0.8, 0.45, 0], "wall_clock": "2026-02-15T14:30:22.234567Z"}
{"timestamp": 5.000000, "osc_address": "/test/stop", "osc_args": [], "wall_clock": "2026-02-15T14:30:27.000000Z"}
```

---

### `audio.wav` — Audio capturado

**Formato:** WAV (PCM, sin compresión)

**Descripción:** Audio capturado durante la sesión de calibración, sincronizado con los eventos en `run.ndjson`.

**Especificaciones técnicas:**
- **Sample rate:** 44100 Hz (o configurable, mínimo 44100)
- **Bit depth:** 16 bits (o 24 bits si está disponible)
- **Channels:** Estéreo (2 canales) o mono (1 canal), configurable
- **Duración:** Coincide con la duración de la sesión

**Sincronización:**
- El primer sample de audio corresponde a `timestamp: 0.0` en `run.ndjson`
- Los eventos OSC están sincronizados con el audio mediante timestamps precisos

**Nota:** Si no se captura audio (por ejemplo, en modo solo-registro-de-eventos), este archivo puede no existir o estar vacío.

---

### `meta.json` — Metadatos de sesión

**Formato:** JSON (objeto único)

**Descripción:** Metadatos completos de la sesión de calibración: configuración, parámetros, timestamps, y referencias a otros archivos.

**Estructura:**

```json
{
  "session_id": "20260215_143022",
  "start_time": "2026-02-15T14:30:22.000000Z",
  "end_time": "2026-02-15T14:30:27.000000Z",
  "duration_seconds": 5.0,
  "config": {
    "osc_port": 9000,
    "osc_host": "127.0.0.1",
    "audio_sample_rate": 44100,
    "audio_channels": 2,
    "audio_bit_depth": 16
  },
  "control_events": [
    {"time": 0.0, "command": "/test/start", "args": ["run_001"]},
    {"time": 2.5, "command": "/test/seek", "args": [2.5]},
    {"time": 5.0, "command": "/test/stop", "args": []}
  ],
  "parameters": {
    "seed": 42,
    "rate": 1.0
  },
  "statistics": {
    "total_events": 150,
    "hit_events": 145,
    "state_events": 5,
    "plate_events": 0
  },
  "files": {
    "events": "run.ndjson",
    "audio": "audio.wav",
    "metrics": "metrics.json",
    "report": "report.html"
  }
}
```

**Campos principales:**
- `session_id`: Identificador único de la sesión
- `start_time` / `end_time`: Timestamps ISO 8601 de inicio y fin
- `duration_seconds`: Duración total en segundos
- `config`: Configuración de hardware/software usada
- `control_events`: Lista de comandos de control enviados durante la sesión
- `parameters`: Parámetros de configuración (seed, rate, etc.)
- `statistics`: Estadísticas básicas de eventos capturados
- `files`: Referencias a archivos generados (algunos pueden no existir hasta después del análisis)

---

### `metrics.json` — Métricas calculadas

**Formato:** JSON (objeto único)

**Descripción:** Métricas calculadas por el notebook de análisis. Este archivo se genera **después** de ejecutar el análisis, no durante la captura.

**Estructura (ejemplo):**

```json
{
  "spectral": {
    "mean_spectral_centroid": 2500.5,
    "mean_spectral_rolloff": 8000.2,
    "mean_spectral_bandwidth": 3000.1,
    "mean_zero_crossing_rate": 0.15
  },
  "temporal": {
    "mean_rms": 0.25,
    "mean_energy": 0.18,
    "peak_amplitude": 0.85,
    "dynamic_range_db": 45.2
  },
  "statistical": {
    "event_rate_per_second": 29.0,
    "hit_distribution_by_surface": {"0": 35, "1": 38, "2": 28, "3": 44},
    "energy_distribution": {
      "mean": 0.45,
      "std": 0.18,
      "min": 0.05,
      "max": 0.95
    }
  },
  "reproducibility": {
    "seed_used": 42,
    "deterministic": true,
    "validation_passed": true
  }
}
```

**Nota:** La estructura exacta depende de las métricas implementadas en el notebook de análisis. Este es un ejemplo de estructura esperada.

---

### `report.html` — Reporte de análisis

**Formato:** HTML (autocontenido, puede incluir gráficos embebidos)

**Descripción:** Reporte visual generado por el notebook de análisis, incluyendo gráficos, tablas, y conclusiones.

**Contenido esperado:**
- Visualizaciones de análisis espectral (DFT, STFT)
- Gráficos de métricas temporales
- Distribuciones estadísticas
- Validación de reproducibilidad
- Conclusiones y observaciones

**Nota:** Este archivo se genera **después** de ejecutar el notebook de análisis.

---

## Non-Goals

MAAD-2-CALIB está diseñado como una **capa de calibración y validación** que **no modifica** la arquitectura core del sistema. Específicamente:

- ❌ **No modifica** la simulación de partículas en oF
- ❌ **No modifica** el sintetizador en JUCE
- ❌ **No modifica** el contrato OSC existente (`/hit`, `/state`, `/plate`)
- ✅ **Solo agrega** comandos de control (`/test/*`) para orquestación
- ✅ **Solo captura** eventos y audio para análisis posterior
- ✅ **Solo analiza** datos offline sin afectar tiempo real

---

## Integración con Sistema Existente

### Compatibilidad

MAAD-2-CALIB es **completamente compatible** con el sistema existente:

- Usa el mismo puerto OSC (9000) y protocolo
- Escucha los mismos mensajes que JUCE (`/hit`, `/state`, `/plate`)
- No interfiere con la comunicación oF ↔ JUCE
- Puede ejecutarse en paralelo o como reemplazo temporal de JUCE para captura

### Modos de operación

1. **Modo pasivo (escucha):** CALIB escucha eventos OSC sin enviar comandos de control
2. **Modo activo (orquestación):** CALIB envía comandos `/test/*` para controlar sesiones
3. **Modo análisis:** Solo procesa datos existentes en `runs/` sin captura

---

## Técnicas de Análisis Implementadas

El notebook de análisis implementa las siguientes técnicas de procesamiento digital de señales:

### Transformada Discreta de Fourier (DFT)

- Análisis espectral del audio capturado
- Cálculo de espectro de magnitud y fase
- Identificación de componentes frecuenciales

### Transformada de Fourier de Tiempo Corto (STFT)

- Análisis tiempo-frecuencia del audio completo
- Experimentación con diferentes ventanas (Hamming, Hanning, Blackman)
- Visualización mediante espectrogramas

### Análisis de Sistemas

- Respuesta en frecuencia
- Función de transferencia
- Respuesta al impulso

### Convolución y Filtrado

- Filtros FIR e IIR
- Procesamiento de señales con diferentes tipos de filtros

### Métricas Temporales

- Cálculo de energía y promedio de señales
- Análisis de variaciones y envolventes
- Métricas de dinámica (RMS, peak, dynamic range)

### Validación Estadística

- Reproducibilidad de sesiones
- Distribuciones de eventos
- Validación de comportamiento determinístico

---

## Referencias

- **[Contrato OSC](../api-osc.md)** — Contrato completo de mensajes OSC
- **[Especificación Particles](../Particles/spec.md)** — Especificación del módulo de partículas
- **[Especificación JUCE](../JUCE/spec.md)** — Especificación del módulo de síntesis
- **[Alineación académica](../../maad-2-calib/specs/ACADEMIC_ALIGNMENT.md)** — Alineación con contenidos académicos MAAD-2
- **[Especificación técnica completa](../specs/spec.md)** — Arquitectura general del sistema

---

**Última actualización**: 2026-02-11
