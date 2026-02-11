# CALIB — Manual de Usuario

Este manual describe cómo usar el módulo MAAD-2-CALIB para realizar sesiones de calibración y análisis del Sistema Modular Audiovisual.

---

## 1. Configuración Inicial

### 1.1. Requisitos

- **Python** 3.x instalado
- **Jupyter Notebook** instalado
- **Librerías Python**: NumPy, SciPy, Librosa, Matplotlib
- **Herramientas OSC**: `oscsend` (liblo) o librerías OSC en Python

### 1.2. Instalación de Dependencias

```bash
# Instalar dependencias Python
pip install numpy scipy librosa matplotlib jupyter

# Instalar herramientas OSC (macOS con Homebrew)
brew install liblo

# O usar librerías OSC en Python
pip install python-osc
```

---

## 2. Uso Básico

### 2.1. Iniciar Sesión de Calibración

Para iniciar una sesión de calibración, envía el comando OSC `/test/start`:

```bash
# Usando oscsend (liblo)
oscsend localhost 9000 /test/start "run_001"

# O usando Python
python -c "from pythonosc import osc_message_builder, udp_client; client = udp_client.SimpleUDPClient('127.0.0.1', 9000); client.send_message('/test/start', ['run_001'])"
```

**Comportamiento:**
- CALIB crea un directorio `runs/YYYYMMDD_HHMMSS/` (o usa `run_id` si se proporciona)
- Inicia captura de audio (si está disponible)
- Inicia registro de eventos OSC en `run.ndjson`
- Genera `meta.json` con timestamp de inicio y configuración

### 2.2. Ejecutar Simulación

Ejecuta oF (Particles) y JUCE (Synthesis) normalmente. CALIB capturará todos los eventos OSC y el audio automáticamente.

**Nota:** CALIB debe estar ejecutándose y escuchando en el puerto 9000 antes de iniciar la sesión.

### 2.3. Detener Sesión

Para detener la sesión de calibración:

```bash
oscsend localhost 9000 /test/stop
```

**Comportamiento:**
- Detiene captura de audio
- Cierra `run.ndjson`
- Finaliza `meta.json` con timestamp de fin y duración total
- Guarda `audio.wav` si se capturó audio

### 2.4. Analizar Datos

Después de detener la sesión, analiza los datos con el notebook:

```bash
# Navegar al directorio del módulo
cd maad-2-calib

# Abrir notebook de análisis
jupyter notebook notebooks/analysis_template.ipynb
```

El notebook procesará los datos en `runs/YYYYMMDD_HHMMSS/` y generará:
- `metrics.json` — Métricas calculadas
- `report.html` — Reporte de análisis con visualizaciones

---

## 3. Comandos OSC de Control

### 3.1. `/test/start` — Iniciar Sesión

**Formato:**
```
/test/start "run_id"
```

**Parámetros:**
- `run_id` (string, opcional): Identificador único de la sesión. Si no se proporciona, se genera automáticamente con timestamp.

**Ejemplo:**
```bash
oscsend localhost 9000 /test/start "run_001"
oscsend localhost 9000 /test/start "test_collisions_high_energy"
```

### 3.2. `/test/stop` — Detener Sesión

**Formato:**
```
/test/stop
```

**Parámetros:** Ninguno

**Ejemplo:**
```bash
oscsend localhost 9000 /test/stop
```

### 3.3. `/test/seek` — Control de Posición Temporal

**Formato:**
```
/test/seek time
```

**Parámetros:**
- `time` (float, >= 0.0): Tiempo en segundos desde el inicio de la sesión

**Ejemplo:**
```bash
oscsend localhost 9000 /test/seek 15.5
```

**Uso:** Marca un punto de referencia temporal para análisis posterior. No modifica el estado de la simulación en tiempo real.

### 3.4. `/test/rate` — Control de Velocidad

**Formato:**
```
/test/rate rate
```

**Parámetros:**
- `rate` (float, 0.1 - 2.0): Factor de velocidad (1.0 = normal, 0.5 = mitad, 2.0 = doble)

**Ejemplo:**
```bash
oscsend localhost 9000 /test/rate 0.5  # Mitad de velocidad
oscsend localhost 9000 /test/rate 2.0  # Doble velocidad
```

**Uso:** Controla la velocidad de reproducción en modo análisis offline. No afecta la simulación en tiempo real.

### 3.5. `/test/seed` — Establecer Semilla

**Formato:**
```
/test/seed seed
```

**Parámetros:**
- `seed` (int32): Semilla para generador de números aleatorios

**Ejemplo:**
```bash
oscsend localhost 9000 /test/seed 42
```

**Importante:** Este mensaje debe enviarse **antes** de `/test/start` para que tenga efecto en la reproducibilidad de la sesión.

### 3.6. `/test/beep` — Señal de Prueba

**Formato:**
```
/test/beep freq duration amplitude
```

**Parámetros:**
- `freq` (float, 20.0 - 20000.0): Frecuencia en Hz (por defecto: 1000.0)
- `duration` (float, 0.01 - 5.0): Duración en segundos (por defecto: 0.5)
- `amplitude` (float, 0.0 - 1.0): Amplitud normalizada (por defecto: 0.5)

**Ejemplo:**
```bash
oscsend localhost 9000 /test/beep 1000.0 0.5 0.5
```

**Uso:** Genera una señal de prueba para calibración de niveles de audio y sincronización temporal.

---

## 4. Estructura de Archivos Generados

Cada sesión de calibración genera un directorio con timestamp: `runs/YYYYMMDD_HHMMSS/`

### 4.1. `run.ndjson` — Eventos OSC

Formato NDJSON (Newline Delimited JSON) con un objeto JSON por línea.

**Estructura de cada línea:**
```json
{
  "timestamp": 0.123456,
  "osc_address": "/hit",
  "osc_args": [42, 0.75, 0.3, 0.65, 1],
  "wall_clock": "2026-02-15T14:30:22.123456Z"
}
```

**Campos:**
- `timestamp`: Tiempo relativo desde inicio de sesión en segundos
- `osc_address`: Dirección del mensaje OSC
- `osc_args`: Array de argumentos del mensaje OSC
- `wall_clock`: Timestamp absoluto ISO 8601

### 4.2. `audio.wav` — Audio Capturado

Formato WAV (PCM, sin compresión) con las siguientes especificaciones:
- **Sample rate:** 44100 Hz (o configurable)
- **Bit depth:** 16 bits (o 24 bits si está disponible)
- **Channels:** Estéreo (2 canales) o mono (1 canal), configurable

**Sincronización:** El primer sample de audio corresponde a `timestamp: 0.0` en `run.ndjson`.

### 4.3. `meta.json` — Metadatos de Sesión

JSON con metadatos completos de la sesión:

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
  }
}
```

### 4.4. `metrics.json` — Métricas Calculadas

Generado por el notebook de análisis después de procesar los datos. Contiene métricas espectrales, temporales y estadísticas.

### 4.5. `report.html` — Reporte de Análisis

Generado por el notebook de análisis. Reporte visual con gráficos, tablas y conclusiones.

---

## 5. Análisis con Jupyter Notebook

### 5.1. Abrir Notebook

```bash
cd maad-2-calib
jupyter notebook notebooks/analysis_template.ipynb
```

### 5.2. Configurar Ruta de Sesión

En el notebook, configura la ruta a la sesión que quieres analizar:

```python
session_path = "runs/20260215_143022"
```

### 5.3. Ejecutar Análisis

El notebook incluye celdas para:
- Cargar datos (`run.ndjson`, `audio.wav`, `meta.json`)
- Análisis espectral (DFT, STFT)
- Cálculo de métricas temporales
- Validación estadística
- Generación de visualizaciones
- Exportación de `metrics.json` y `report.html`

### 5.4. Técnicas de Análisis Disponibles

- **Transformada Discreta de Fourier (DFT)**: Análisis espectral del audio
- **Transformada de Fourier de Tiempo Corto (STFT)**: Análisis tiempo-frecuencia
- **Análisis de sistemas**: Respuesta en frecuencia, función de transferencia
- **Convolución y filtrado**: Filtros FIR/IIR
- **Métricas temporales**: Energía, promedio, envolventes
- **Validación estadística**: Reproducibilidad, distribuciones

---

## 6. Ejemplos de Uso

### 6.1. Sesión Básica de Calibración

```bash
# 1. Iniciar sesión
oscsend localhost 9000 /test/start "basic_test"

# 2. Ejecutar oF (Particles) y JUCE (Synthesis) normalmente
# CALIB capturará eventos y audio automáticamente

# 3. Detener sesión después de unos segundos
oscsend localhost 9000 /test/stop

# 4. Analizar datos
cd maad-2-calib
jupyter notebook notebooks/analysis_template.ipynb
```

### 6.2. Sesión con Semilla para Reproducibilidad

```bash
# 1. Establecer semilla ANTES de iniciar
oscsend localhost 9000 /test/seed 42

# 2. Iniciar sesión
oscsend localhost 9000 /test/start "reproducible_test"

# 3. Ejecutar simulación...

# 4. Detener sesión
oscsend localhost 9000 /test/stop
```

### 6.3. Sesión con Marcadores Temporales

```bash
# 1. Iniciar sesión
oscsend localhost 9000 /test/start "marked_test"

# 2. Marcar puntos de interés durante la sesión
oscsend localhost 9000 /test/seek 5.0   # Marca a los 5 segundos
oscsend localhost 9000 /test/seek 10.0  # Marca a los 10 segundos

# 3. Detener sesión
oscsend localhost 9000 /test/stop
```

---

## 7. Troubleshooting

### 7.1. CALIB no recibe eventos OSC

**Problema:** Los eventos no se registran en `run.ndjson`.

**Soluciones:**
- Verificar que CALIB esté ejecutándose y escuchando en puerto 9000
- Verificar que oF (Particles) esté enviando mensajes al puerto correcto
- Verificar firewall no bloquea UDP puerto 9000
- Verificar que la sesión esté iniciada (`/test/start` enviado)

### 7.2. No se captura audio

**Problema:** El archivo `audio.wav` no se genera o está vacío.

**Soluciones:**
- Verificar que la captura de audio esté habilitada en CALIB
- Verificar permisos de audio en el sistema operativo
- Verificar que la interfaz de audio esté configurada correctamente

### 7.3. El notebook no encuentra los datos

**Problema:** El notebook no puede cargar `run.ndjson` o `audio.wav`.

**Soluciones:**
- Verificar que la ruta `session_path` en el notebook sea correcta
- Verificar que los archivos existan en el directorio de sesión
- Verificar permisos de lectura en el directorio `runs/`

---

## Referencias

- **[Descripción general](readme.md)** — Arquitectura del módulo
- **[Especificación técnica](spec.md)** — Detalles de implementación y contrato OSC
- **[Contrato OSC principal](../api-osc.md)** — Contrato completo de mensajes OSC

---

**Última actualización**: 2026-02-11
