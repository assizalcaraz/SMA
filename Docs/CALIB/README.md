# CALIB — Descripción General y Arquitectura

Este documento describe la arquitectura y el funcionamiento general del módulo MAAD-2-CALIB (App C) del Sistema Modular Audiovisual.

---

## 1. Propósito del Módulo

MAAD-2-CALIB es una capa de calibración y validación que se integra con el Sistema Modular Audiovisual (SMA) para proporcionar herramientas de análisis y validación reproducibles. El módulo está diseñado como trabajo final integrador de **Matemática Aplicada al Arte Digital II (MAAD-2)**, modalidad 2 (Herramienta propia).

Tiene tres responsabilidades principales:

1. **CONTROL:** Orquestación de transporte OSC para sesiones de calibración reproducibles
2. **REGISTRATION:** Captura y registro de datos (eventos OSC, audio, metadata) en formato estructurado
3. **ANALYSIS:** Análisis offline basado en Python/Jupyter con técnicas de procesamiento digital de señales

---

## 2. Arquitectura del Sistema

### 2.1. Diagrama de Flujo

```
┌─────────────────────┐
│  oF (Particles)      │
│  Simulación          │
└──────────┬────────────┘
           │ OSC (/hit, /state, /plate)
           │
           ▼
┌─────────────────────┐
│ JUCE (Synthesis)    │
│  Sintetizador       │
└──────────┬────────────┘
           │ Audio + OSC
           │
           ▼
┌─────────────────────────────┐
│  CALIB (Control +            │
│   Registration)              │
│  - Captura OSC               │
│  - Captura Audio             │
│  - Registro NDJSON           │
└──────────┬────────────────────┘
           │ Datos estructurados
           │ (runs/YYYYMMDD_HHMMSS/)
           ▼
┌─────────────────────────────┐
│  Notebook (Analysis)         │
│  - Análisis espectral        │
│  - STFT, DFT                 │
│  - Métricas                  │
└──────────┬────────────────────┘
           │
           ▼
┌─────────────────────────────┐
│  Report (HTML)               │
│  - Visualizaciones           │
│  - Métricas                  │
│  - Conclusiones              │
└─────────────────────────────┘
```

### 2.2. Responsabilidades

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

## 3. Estructura del Módulo

```
maad-2-calib/
├── README.md                    # Descripción general (raíz del módulo)
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

## 4. Flujo de Datos

### 4.1. Sesión de Calibración

1. **Inicio**: CALIB recibe `/test/start` y crea directorio de sesión
2. **Captura**: CALIB escucha eventos OSC (`/hit`, `/state`, `/plate`) y captura audio
3. **Registro**: Eventos se escriben línea por línea en `run.ndjson`, audio se captura en `audio.wav`
4. **Fin**: CALIB recibe `/test/stop` y finaliza archivos, genera `meta.json`
5. **Análisis**: Notebook procesa datos en `runs/YYYYMMDD_HHMMSS/` y genera `metrics.json` y `report.html`

### 4.2. Integración con Sistema Existente

CALIB se integra de forma **no invasiva** con el sistema:

- **Mismo puerto OSC (9000)**: Escucha los mismos mensajes que JUCE
- **No interfiere**: La comunicación oF ↔ JUCE continúa funcionando normalmente
- **Modos de operación**:
  - **Modo pasivo (escucha)**: CALIB solo escucha eventos sin enviar comandos
  - **Modo activo (orquestación)**: CALIB envía comandos `/test/*` para controlar sesiones
  - **Modo análisis**: Solo procesa datos existentes sin captura

---

## 5. Relación con la Arquitectura General del Sistema

Este módulo CALIB es una **capa adicional** que no modifica la arquitectura core del Sistema Modular Audiovisual. Su diseño permite:

- **Calibración reproducible**: Sesiones de prueba con semillas y parámetros controlados
- **Análisis científico**: Validación de comportamiento del sistema mediante técnicas de procesamiento de señales
- **Documentación técnica**: Registro estructurado de sesiones para análisis posterior

Para una visión completa de la arquitectura del sistema, incluyendo la interacción entre todos los módulos, consulte la **[Especificación Técnica General](../specs/spec.md)**.

---

## 6. Alineación Académica

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

Para detalles completos de alineación académica, ver [`../../maad-2-calib/specs/ACADEMIC_ALIGNMENT.md`](../../maad-2-calib/specs/ACADEMIC_ALIGNMENT.md).

---

## Referencias

- **[Contrato OSC](../api-osc.md)** — Contrato completo de mensajes OSC
- **[Especificación técnica completa](../specs/spec.md)** — Arquitectura general del sistema
- **[Plan de implementación](../specs/PLAN_IMPLEMENTACION.md)** — Fases de desarrollo
- **[Estado actual](../overview/ESTADO_ACTUAL.md)** — Estado del proyecto

---

**Última actualización**: 2026-02-11
