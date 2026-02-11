# MAAD-2-CALIB — Módulo de Calibración y Validación

**SMA v0.4 – Calibration & Validation Layer**

Módulo de calibración y validación para el Sistema Modular Audiovisual, diseñado como trabajo final integrador de **Matemática Aplicada al Arte Digital II (MAAD-2)**.

**Última actualización:** v0.4 (2026-02-XX)

---

## Descripción

MAAD-2-CALIB es una capa de calibración y validación que se integra con el Sistema Modular Audiovisual (SMA) para proporcionar herramientas de análisis y validación reproducibles. El módulo tiene tres responsabilidades principales:

1. **CONTROL:** Orquestación de transporte OSC para sesiones de calibración reproducibles
2. **REGISTRATION:** Captura y registro de datos (eventos OSC, audio, metadata) en formato estructurado
3. **ANALYSIS:** Análisis offline basado en Python/Jupyter con técnicas de procesamiento digital de señales

---

## Arquitectura

### Diagrama de Flujo

```
┌─────────────┐
│  oF (Particles) │
│  Simulación     │
└──────┬──────────┘
       │ OSC (/hit, /state, /plate)
       │
       ▼
┌─────────────┐
│ JUCE (Synthesis) │
│  Sintetizador    │
└──────┬──────────┘
       │ Audio + OSC
       │
       ▼
┌─────────────────────┐
│  CALIB (Control +   │
│   Registration)     │
│  - Captura OSC      │
│  - Captura Audio    │
│  - Registro NDJSON  │
└──────┬──────────────┘
       │ Datos estructurados
       │ (runs/YYYYMMDD_HHMMSS/)
       ▼
┌─────────────────────┐
│  Notebook (Analysis)│
│  - Análisis espectral│
│  - STFT, DFT        │
│  - Métricas         │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Report (HTML)      │
│  - Visualizaciones  │
│  - Métricas         │
│  - Conclusiones      │
└─────────────────────┘
```

### Responsabilidades

#### CONTROL

Orquestación de sesiones de calibración mediante comandos OSC (`/test/*`):

- `/test/start` — Iniciar sesión de calibración
- `/test/stop` — Detener sesión
- `/test/seek` — Control de posición temporal
- `/test/rate` — Control de velocidad de reproducción
- `/test/seed` — Establecer semilla para reproducibilidad
- `/test/beep` — Señal de prueba/calibración

#### REGISTRATION

Captura y registro de datos en formato estructurado:

- **Eventos OSC:** Registro línea por línea en formato NDJSON (`run.ndjson`)
- **Audio:** Captura de audio completo de la sesión (`audio.wav`)
- **Metadata:** Configuración, parámetros, timestamps (`meta.json`)

Cada sesión genera un directorio con timestamp: `runs/YYYYMMDD_HHMMSS/`

#### ANALYSIS

Análisis offline con técnicas de procesamiento digital de señales:

- **Análisis espectral:** DFT, STFT con diferentes ventanas
- **Análisis de sistemas:** Respuesta en frecuencia, función de transferencia
- **Convolución y filtrado:** Filtros FIR/IIR
- **Métricas temporales:** Energía, promedio, envolventes
- **Validación estadística:** Reproducibilidad, distribuciones
- **Generación de reportes:** HTML con visualizaciones

---

## Estructura del Módulo

```
maad-2-calib/
├── README.md                    # Este archivo
├── specs/
│   ├── CALIB_SPEC.md           # Especificación técnica completa
│   └── ACADEMIC_ALIGNMENT.md   # Alineación con contenidos académicos
├── src/
│   └── main.cpp                # Código fuente (placeholder)
├── runs/                       # Directorio de sesiones de calibración
│   └── YYYYMMDD_HHMMSS/        # Cada sesión genera un directorio
│       ├── run.ndjson          # Eventos OSC registrados
│       ├── audio.wav           # Audio capturado
│       ├── meta.json           # Metadatos de sesión
│       ├── metrics.json        # Métricas calculadas (generado por notebook)
│       └── report.html         # Reporte de análisis (generado por notebook)
└── notebooks/
    └── analysis_template.ipynb # Template de análisis Jupyter
```

---

## Uso Rápido

### 1. Iniciar sesión de calibración

```bash
# Enviar comando OSC para iniciar sesión
oscsend localhost 9000 /test/start "run_001"
```

### 2. Ejecutar simulación

Ejecutar oF (Particles) y JUCE (Synthesis) normalmente. CALIB capturará todos los eventos OSC y el audio.

### 3. Detener sesión

```bash
# Enviar comando OSC para detener sesión
oscsend localhost 9000 /test/stop
```

### 4. Analizar datos

```bash
# Abrir notebook de análisis
jupyter notebook notebooks/analysis_template.ipynb
```

El notebook procesará los datos en `runs/YYYYMMDD_HHMMSS/` y generará `metrics.json` y `report.html`.

---

## Alineación Académica

Este módulo está diseñado para cumplir con los requisitos del **Trabajo Final Integrador de MAAD-2**, modalidad 2 (Herramienta propia).

### Temas del Curso Cubiertos

- Transformada Discreta de Fourier (DFT)
- Transformada de Fourier de Tiempo Corto (STFT)
- Transformada Z
- Convolución discreta
- Sistemas LTI (lineales e invariantes en el tiempo)
- Función de transferencia, respuesta al impulso y en frecuencia
- Filtros FIR e IIR
- Cálculo de energía y promedio de señales
- Análisis de variaciones en señales y envolventes
- Ecuaciones diferenciales lineales y sistemas resonantes

Ver [`specs/ACADEMIC_ALIGNMENT.md`](specs/ACADEMIC_ALIGNMENT.md) para detalles completos de alineación académica.

---

## Especificación Técnica

Para detalles completos de implementación, contrato OSC, y estructura de artefactos, ver:

- **[CALIB_SPEC.md](specs/CALIB_SPEC.md)** — Especificación técnica completa
- **[ACADEMIC_ALIGNMENT.md](specs/ACADEMIC_ALIGNMENT.md)** — Alineación académica

---

## Integración con SMA

MAAD-2-CALIB es **completamente compatible** con el sistema existente:

- Usa el mismo puerto OSC (9000) y protocolo
- Escucha los mismos mensajes que JUCE (`/hit`, `/state`, `/plate`)
- No interfiere con la comunicación oF ↔ JUCE
- Puede ejecutarse en paralelo o como reemplazo temporal de JUCE para captura

**Non-goals:** CALIB no modifica la arquitectura core del sistema (simulación de partículas, sintetizador). Solo agrega capacidad de calibración y análisis.

---

## Requisitos

### Software

- Python 3.x
- Jupyter Notebook
- NumPy, SciPy, Librosa, Matplotlib
- Herramientas OSC (oscsend, oscdump, o librerías OSC)

### Hardware

- Sistema capaz de ejecutar oF y JUCE simultáneamente
- Captura de audio (opcional, para análisis de audio)

---

## Estado del Proyecto

**Versión:** v0.4 (Scaffolding)

**Estado actual:**
- ✅ Estructura de directorios creada
- ✅ Especificación técnica completa
- ✅ Alineación académica documentada
- ⏳ Implementación de código (pendiente)
- ⏳ Notebook de análisis (template creado, implementación pendiente)

---

## Referencias

- **Sistema Modular Audiovisual:** [`../readme.md`](../readme.md)
- **Contrato OSC principal:** [`../Docs/api-osc.md`](../Docs/api-osc.md)
- **Especificación Particles:** [`../Docs/Particles/spec.md`](../Docs/Particles/spec.md)
- **Estado actual del proyecto:** [`../Docs/overview/ESTADO_ACTUAL.md`](../Docs/overview/ESTADO_ACTUAL.md)

---

## Autor

**José Assiz Alcaraz Baxter**  
Posgrado de especialización en sonido para las artes digitales  
**Materia:** Matemática Aplicada al Arte Digital II (MAAD-2)  
**Fecha:** Febrero 2026

---

**Última actualización:** 2026-02-XX
