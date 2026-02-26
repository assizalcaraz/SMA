# Sistema Modular Audiovisual

Sistema audiovisual en tiempo real que integra tracking corporal con síntesis de audio mediante comunicación OSC. El proyecto está dividido en dos aplicaciones independientes que se comunican a través de un contrato de mensajes estable.

## Descripción del proyecto

Este sistema permite generar sonido en tiempo real a partir del movimiento corporal del usuario. Utiliza tracking de manos o pose corporal mediante MediaPipe para controlar un sistema de partículas físicas. Las colisiones de las partículas con los bordes generan eventos sonoros que son procesados por un sintetizador en JUCE, creando un timbre metálico característico.

**Documentación técnica completa:** Ver [`Docs/specs/spec.md`](Docs/specs/spec.md) para detalles de implementación, arquitectura y especificaciones técnicas.

**Documentación organizada:** Ver [`Docs/README.md`](Docs/README.md) para índice completo y rutas de aprendizaje.

**Entregas ISTR y PAS1:** Para empaquetar los módulos Particles (ISTR) y app-juce (PAS1) sin dependencias de calibración, usar la rama `entrega-istr-pas1`. Ver [Estado actual — Rama de entrega](Docs/overview/ESTADO_ACTUAL.md#contenido-de-la-rama-de-entrega) para parches aplicados y uso.

**Índice de documentación:** Ver [`Docs/index.md`](Docs/index.md) para mapeo completo de código a documentación.

**Documentación del módulo Particles:**
- [`Docs/Particles/README.md`](Docs/Particles/README.md) - Índice del módulo
- [`Docs/Particles/readme.md`](Docs/Particles/readme.md) - Descripción general
- [`Docs/Particles/manual.md`](Docs/Particles/manual.md) - Manual de usuario (parámetros)
- [`Docs/Particles/spec.md`](Docs/Particles/spec.md) - Especificación técnica

**Documentación del módulo JUCE:**
- [`Docs/JUCE/README.md`](Docs/JUCE/README.md) - Índice del módulo
- [`Docs/JUCE/readme.md`](Docs/JUCE/readme.md) - Descripción general y arquitectura
- [`Docs/JUCE/manual.md`](Docs/JUCE/manual.md) - Manual de usuario
- [`Docs/JUCE/spec.md`](Docs/JUCE/spec.md) - Especificación técnica

---

## Arquitectura del sistema

El sistema está compuesto por dos aplicaciones independientes:

### App A — openFrameworks (ISTR)
- **Captura:** Video en tiempo real mediante webcam (`ofVideoGrabber`)
- **Tracking:** MediaPipe para detección de manos o pose corporal
- **Simulación física:** Sistema de partículas con fuerzas dinámicas
  - Retorno al origen (home)
  - Influencia de gestos del usuario
  - Colisiones con bordes
- **Comunicación:** Envío de eventos OSC al sintetizador

### App B — JUCE (PAS1)
- **Formato:** Standalone (entrega principal). Plugin VST3/AU (bonus opcional)
- **Receptor OSC:** Escucha mensajes en puerto 9000
- **Motor sonoro:** Sintetizador con resonadores modales
  - Timbre metálico "coin cascade" mediante modos inarmónicos
  - Polyphony (16-64 voces) con voice stealing
  - Mapeo de parámetros desde eventos OSC
  - Master limiter para anti-saturación
- **Salida:** Audio en tiempo real

### Flujo de datos

```
Webcam → MediaPipe → (puntos y velocidades) → Fuerzas → Partículas → 
Colisiones → Eventos → OSC → JUCE → Audio
```

### App C — MAAD-2-CALIB (v0.4)

**SMA v0.4 – Calibration & Validation Layer**

- **Formato:** Módulo independiente (C++ para control + Python/Jupyter para análisis)
- **Responsabilidades:**
  1. **CONTROL:** Orquestación de transporte OSC para sesiones de calibración reproducibles
  2. **REGISTRATION:** Captura y registro de datos (NDJSON + WAV + metadata)
  3. **ANALYSIS:** Análisis offline con técnicas de procesamiento digital de señales
- **Integración:** Se conecta al sistema existente mediante OSC, no modifica arquitectura core
- **Salida:** Datos estructurados en `runs/YYYYMMDD_HHMMSS/` para análisis posterior

**Arquitectura v0.4:**

```
oF (Particles) → JUCE (Synthesis) → CALIB (Control + Registration) → 
Notebook (Analysis) → Report (HTML)
```

**Documentación del módulo CALIB:**
- [`Docs/CALIB/README.md`](Docs/CALIB/README.md) - Índice del módulo
- [`Docs/CALIB/readme.md`](Docs/CALIB/readme.md) - Descripción general y arquitectura
- [`Docs/CALIB/manual.md`](Docs/CALIB/manual.md) - Manual de usuario (comandos OSC, uso, análisis)
- [`Docs/CALIB/spec.md`](Docs/CALIB/spec.md) - Especificación técnica

---

## Contrato OSC

Las aplicaciones se comunican mediante mensajes OSC sobre UDP.

### Configuración de transporte

- **Protocolo:** UDP OSC
- **Host:** `127.0.0.1` (localhost, configurable)
- **Puerto:** `9000` (configurable)

### Mensajes principales

#### `/hit` — Evento de impacto (principal)

Este mensaje se envía cuando una partícula colisiona con un borde y genera un evento sonoro.

**Parámetros:**
1. `int32 id` — Identificador único de la partícula
2. `float x` — Posición X normalizada (0.0 - 1.0)
3. `float y` — Posición Y normalizada (0.0 - 1.0)
4. `float energy` — Energía del impacto (0.0 - 1.0)
5. `int32 surface` — Superficie impactada:
   - `0` = Borde izquierdo (L)
   - `1` = Borde derecho (R)
   - `2` = Borde superior (T)
   - `3` = Borde inferior (B)
   - `-1` = No aplica

**Ejemplo:**
```
/hit 42 0.75 0.3 0.65 1
```

Este mensaje es suficiente para generar sonido. Es el mensaje principal del sistema.

#### `/state` — Estado global (opcional)

Mensaje periódico con información agregada del sistema.

**Parámetros:**
1. `float activity` — Actividad normalizada (0.0 - 1.0), basada en hits por segundo
2. `float gesture` — Energía de gesto agregada (0.0 - 1.0)
3. `float presence` — Confianza del tracking (0.0 - 1.0)

**Ejemplo:**
```
/state 0.45 0.32 0.89
```

#### `/ctrl` — Control remoto (opcional, MVP no requerido)

Permite controlar parámetros de oF desde JUCE.

**Parámetros:**
- `string key` — Nombre del parámetro
- `float value` — Valor del parámetro

**Ejemplo:**
```
/ctrl k_home 2.5
```

> **Nota:** Este mensaje no es necesario para el MVP. Ver [`docs/api-osc.md`](docs/api-osc.md) para documentación completa del contrato OSC.

---

## Estructura del proyecto

```
Sistema Modular Audiovisual/
├── Particles/           # App A: openFrameworks (sistema de partículas)
│   ├── src/            # Código fuente
│   ├── addons.make     # Configuración de addons
│   └── config/         # Archivos de configuración
├── app-juce/           # App B: JUCE
│   ├── Source/         # Código fuente
│   └── JuceLibraryCode/ # Librerías JUCE
├── maad-2-calib/       # App C: MAAD-2-CALIB (v0.4)
│   ├── README.md       # Descripción del módulo
│   ├── specs/          # Especificaciones técnicas
│   │   ├── CALIB_SPEC.md           # Especificación técnica
│   │   └── ACADEMIC_ALIGNMENT.md   # Alineación académica
│   ├── src/            # Código fuente (C++)
│   ├── notebooks/      # Notebooks de análisis (Python/Jupyter)
│   └── runs/           # Sesiones de calibración (generadas)
├── Docs/               # Documentación completa
│   ├── README.md       # Índice de documentación
│   ├── specs/          # Especificaciones técnicas
│   │   ├── spec.md     # Especificación técnica completa
│   │   └── PLAN_IMPLEMENTACION.md  # Plan de desarrollo
│   └── ...
├── scripts/            # Scripts de utilidad
│   └── test-osc.py     # Script para probar OSC sin oF
└── tests/              # Tests de integración
```

---

## Requisitos e instalación

### Requisitos del sistema

- **Hardware:**
  - Webcam (para tracking)
  - Sistema operativo: macOS / Linux / Windows
  - CPU: Mínimo 4 cores recomendado para tiempo real

- **Software:**
  - openFrameworks (última versión estable)
  - MediaPipe (vía addon de oF o integración externa)
  - JUCE 7+ (para App B)
  - Python 3.x (para scripts de prueba)

### Instalación

#### App A (openFrameworks)

1. Instalar openFrameworks desde [openframeworks.cc](https://openframeworks.cc/)
2. Clonar o descargar el proyecto
3. Configurar addons necesarios:
   - `ofxOsc` — Comunicación OSC
   - `ofxMediaPipe` o alternativa para tracking
4. Abrir proyecto en el IDE correspondiente (Xcode, Visual Studio, etc.)

#### App B (JUCE)

1. Instalar JUCE desde [juce.com](https://juce.com/)
2. Abrir proyecto `.jucer` en Projucer
3. Configurar módulos necesarios:
   - `juce_osc` — Receptor OSC
   - `juce_audio_basics`
   - `juce_audio_processors`
4. Exportar proyecto para tu plataforma
5. Compilar y ejecutar

### Configuración inicial

1. **Configurar comunicación OSC:**
   - Verificar que ambas apps usen el mismo puerto (9000 por defecto)
   - En oF: configurar host destino (`127.0.0.1`)
   - En JUCE: verificar que el listener esté activo en puerto 9000

2. **Calibrar parámetros:**
   - Ajustar número de partículas según rendimiento
   - Configurar sensibilidad de tracking
   - Ajustar parámetros de síntesis en JUCE

---

## Uso y ejecución

### Ejecutar App A (openFrameworks)

1. Abrir proyecto en IDE
2. Compilar y ejecutar
3. La aplicación debería:
   - Inicializar captura de video
   - Detectar manos/pose con MediaPipe
   - Mostrar sistema de partículas en pantalla
   - Enviar eventos OSC cuando hay colisiones

### Ejecutar App B (JUCE)

1. Compilar y ejecutar aplicación Standalone
2. La aplicación debería:
   - Iniciar listener OSC en puerto 9000
   - Mostrar UI con controles de síntesis
   - Reproducir sonido cuando recibe eventos `/hit`

### Ejecutar sistema completo

1. **Orden recomendado:**
   - Primero iniciar App B (JUCE) — receptor
   - Luego iniciar App A (oF) — emisor

2. **Verificar comunicación:**
   - En JUCE debería haber indicadores de actividad OSC
   - Mover manos frente a la cámara
   - Las partículas deberían responder y generar sonido

### Modo fallback (sin webcam)

Si MediaPipe no funciona o no hay webcam disponible:

- App A incluye modo de control con mouse
- El movimiento del mouse emula gestos de manos
- Permite probar el sistema sin dependencia de tracking

### Scripts de prueba

Usar `scripts/test-osc.py` para probar JUCE sin necesidad de ejecutar oF:

```bash
python scripts/test-osc.py
```

Este script envía mensajes OSC simulados para verificar que JUCE responde correctamente.

---

## Decisiones de diseño

- **No se utiliza LLM ni interpretación semántica**
- El sistema prioriza:
  - Estabilidad en tiempo real
  - Latencia baja
  - Comportamiento emergente
- Se separan claramente:
  - **Estado continuo** (campo de partículas)
  - **Eventos discretos** (impactos)

---

## Modo de presentación

El proyecto puede presentarse de dos maneras:

1. **Aplicaciones independientes**
   - oF como sistema visual interactivo
   - JUCE como instrumento reactivo

2. **Sistema integrado**
   - oF genera eventos
   - JUCE los convierte en sonido en tiempo real

---

## Estado del proyecto

### Componentes implementados

🟢 **Completado:**
- Arquitectura del sistema definida
- Especificación técnica completa (`Docs/specs/spec.md`)
- Contrato OSC establecido
- Plan de implementación detallado

🔧 **En desarrollo:**
- Receptor OSC y mapeo (App B - Fase 7)

📋 **Pendiente:**
- Implementación completa de tracking MediaPipe
- Calibración y ajuste de parámetros
- Tests de integración
- Documentación de código

### Roadmap de desarrollo

Ver [`Docs/specs/PLAN_IMPLEMENTACION.md`](Docs/specs/PLAN_IMPLEMENTACION.md) para el plan detallado de desarrollo con fases y tareas específicas.

**Estrategia de implementación:** Priorizar el loop modular completo (partículas → colisiones → OSC → JUCE) antes de integrar MediaPipe. MediaPipe se trata como "swap de input" (mouse primero, MediaPipe después).

**Fases principales:**
1. ✅ Setup inicial del proyecto
2. ✅ Sistema de partículas básico (oF)
3. ✅ Input básico (Mouse) — validación rápida de arquitectura
4. ✅ Colisiones y eventos (+ rate limiting global)
5. ✅ Comunicación OSC (adelantada para validar loop modular)
6. ✅ Sintetizador básico (JUCE Standalone) — **COMPLETADA** — **Standalone = entrega principal**
7. ⏳ Receptor OSC y mapeo (textura "coin cascade")
8. ⏳ Calibración y ajuste conjunto
9. ⏳ Documentación y demo
10. ⏳ MediaPipe (opcional/tardía, solo si el sistema ya funciona)

---

## Autor

Proyecto desarrollado por **José Assiz Alcaraz Baxter**  
Posgrado de especialización en sonido para las artes digitales

**Materias:**
- Imagen y Sonido en Tiempo Real (ISTR)
- Programación aplicada al sonido 1 (PAS1)
- Matemática Aplicada al Arte Digital II (MAAD-2) — v0.4

**Fecha:** Febrero 2026

---

## Referencias Rápidas

- **Documentación completa**: [Docs/README.md](Docs/README.md) — Índice y rutas de aprendizaje
- **Estado del proyecto**: [Docs/overview/ESTADO_ACTUAL.md](Docs/overview/ESTADO_ACTUAL.md) — Qué está implementado
- **Especificación técnica**: [Docs/specs/spec.md](Docs/specs/spec.md) — Arquitectura y diseño completo
- **Plan de desarrollo**: [Docs/specs/PLAN_IMPLEMENTACION.md](Docs/specs/PLAN_IMPLEMENTACION.md) — Fases y tareas
- **Contrato OSC**: [Docs/api-osc.md](Docs/api-osc.md) — Contrato de mensajes
- **Documentación de Particles**: [Docs/Particles/](Docs/Particles/) — Módulo de partículas
- **Documentación de JUCE**: [Docs/JUCE/](Docs/JUCE/) — Módulo de síntesis
- **Documentación de CALIB (v0.4)**: [Docs/CALIB/README.md](Docs/CALIB/README.md) — Módulo de calibración y validación
- **Mapeo código → docs**: [Docs/index.md](Docs/index.md) — Índice de archivos

## Referencias Externas

- [openFrameworks](https://openframeworks.cc/)
- [JUCE](https://juce.com/)
- [MediaPipe](https://mediapipe.dev/)
