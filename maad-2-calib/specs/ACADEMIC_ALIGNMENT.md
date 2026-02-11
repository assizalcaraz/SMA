# MAAD-2-CALIB — Alineación Académica

Documento que establece la alineación explícita del módulo MAAD-2-CALIB con los contenidos del curso **Matemática Aplicada al Arte Digital II (MAAD-2)** y los criterios de evaluación del Trabajo Final Integrador (TFI).

**Última actualización:** v0.4 (2026-02-XX)

---

## Objetivo Académico

MAAD-2-CALIB está diseñado como **herramienta propia para procesar y analizar señales** (Modalidad 2 del TFI), con aplicación práctica en el contexto del Sistema Modular Audiovisual (SMA). El módulo demuestra la aplicación fundamentada de conceptos matemáticos y técnicas de procesamiento digital de señales trabajados en el curso.

---

## Temas del Curso Cubiertos

### 1. Transformada Discreta de Fourier (DFT)

**Aplicación en CALIB:**
- Análisis espectral del audio capturado (`audio.wav`)
- Cálculo de espectro de frecuencia para identificar componentes espectrales
- Análisis de contenido frecuencial de eventos sonoros generados por el sistema

**Implementación:**
- Uso de `numpy.fft` o `scipy.fft` para cálculo de DFT
- Visualización de espectro de magnitud y fase
- Análisis de propiedades de la DFT (simetría, periodicidad)

**Evidencia en notebook:**
- Celdas que calculan y visualizan DFT de segmentos de audio
- Comparación de espectros entre diferentes tipos de eventos (`/hit` con diferentes energías)
- Análisis de contenido espectral de sonidos metálicos generados

---

### 2. Transformada de Fourier de Tiempo Corto (STFT)

**Aplicación en CALIB:**
- Análisis tiempo-frecuencia del audio completo de la sesión
- Identificación de evolución temporal de componentes espectrales
- Análisis de transitorios y eventos sonoros en el tiempo

**Implementación:**
- Uso de `librosa.stft` o `scipy.signal.stft`
- Experimentación con diferentes ventanas temporales (Hamming, Hanning, Blackman)
- Visualización mediante espectrogramas (time-frequency representations)

**Evidencia en notebook:**
- Celdas que calculan STFT con diferentes ventanas
- Comparación de resoluciones tiempo-frecuencia (trade-off)
- Visualización de espectrogramas con diferentes parámetros de ventana
- Análisis de cómo diferentes modos de placa (Chladni) se reflejan en el espectrograma

---

### 3. Transformada Z

**Aplicación en CALIB:**
- Análisis de sistemas LTI del sintetizador (resonadores modales)
- Identificación de polos y ceros en sistemas de síntesis
- Análisis de estabilidad y respuesta en frecuencia de filtros

**Implementación:**
- Uso de `scipy.signal` para análisis de sistemas discretos
- Cálculo de función de transferencia a partir de coeficientes
- Visualización de diagrama de polos y ceros

**Evidencia en notebook:**
- Análisis de respuesta en frecuencia de resonadores modales
- Identificación de frecuencias resonantes a partir de polos
- Comparación teórica vs. empírica de respuesta del sistema

---

### 4. Convolución Discreta

**Aplicación en CALIB:**
- Análisis de respuesta al impulso del sistema completo
- Simulación de filtrado mediante convolución
- Análisis de efectos de procesamiento de señal

**Implementación:**
- Uso de `numpy.convolve` o `scipy.signal.convolve`
- Aplicación de convolución para filtrado
- Análisis de propiedades (conmutatividad, asociatividad)

**Evidencia en notebook:**
- Celdas que aplican convolución para filtrado de señales
- Comparación de señales originales vs. filtradas
- Análisis de respuesta al impulso del sistema de síntesis

---

### 5. Sistemas LTI (Lineales e Invariantes en el Tiempo)

**Aplicación en CALIB:**
- Modelado del sintetizador modal como sistema LTI
- Análisis de respuesta al impulso y en frecuencia
- Validación de propiedades de linealidad e invariancia temporal

**Implementación:**
- Análisis de respuesta del sistema a diferentes entradas
- Verificación de superposición (linealidad)
- Verificación de invariancia temporal

**Evidencia en notebook:**
- Análisis de respuesta del sintetizador a diferentes eventos
- Comparación de respuestas para validar propiedades LTI
- Visualización de función de transferencia del sistema

---

### 6. Función de Transferencia, Respuesta al Impulso y en Frecuencia

**Aplicación en CALIB:**
- Análisis de función de transferencia del sintetizador modal
- Cálculo de respuesta en frecuencia a partir de datos empíricos
- Comparación con modelos teóricos

**Implementación:**
- Uso de `scipy.signal.freqz` para respuesta en frecuencia
- Cálculo de respuesta al impulso desde datos de audio
- Visualización de magnitud y fase

**Evidencia en notebook:**
- Gráficos de respuesta en frecuencia (Bode plots)
- Análisis de respuesta al impulso del sistema
- Comparación teórica vs. empírica

---

### 7. Filtros FIR (Respuesta Finita) e IIR (Respuesta Infinita)

**Aplicación en CALIB:**
- Análisis de filtros utilizados en el sintetizador (si aplica)
- Diseño y evaluación de filtros para procesamiento de señales capturadas
- Comparación de características FIR vs. IIR

**Implementación:**
- Uso de `scipy.signal` para diseño de filtros
- Aplicación de filtros FIR e IIR a señales de audio
- Análisis de características (fase lineal, estabilidad)

**Evidencia en notebook:**
- Diseño y aplicación de filtros FIR/IIR
- Comparación de resultados de filtrado
- Análisis de características espectrales después de filtrado

---

### 8. Cálculo de Energía y Promedio de Señales

**Aplicación en CALIB:**
- Cálculo de energía total de sesiones de audio
- Análisis de energía por segmentos temporales
- Cálculo de promedios y métricas estadísticas

**Implementación:**
- Cálculo de energía: `E = Σ |x[n]|²`
- Cálculo de promedio: `μ = (1/N) Σ x[n]`
- Análisis de energía por ventanas temporales

**Evidencia en notebook:**
- Métricas de energía calculadas y visualizadas
- Análisis de distribución de energía en el tiempo
- Comparación de energía entre diferentes tipos de eventos

---

### 9. Análisis de Variaciones en Señales y Envolventes

**Aplicación en CALIB:**
- Análisis de envolventes de amplitud del audio
- Detección de variaciones temporales en señales
- Análisis de dinámica y transitorios

**Implementación:**
- Cálculo de envolvente mediante valor absoluto + filtrado
- Análisis de derivada para detectar variaciones
- Visualización de envolventes temporales

**Evidencia en notebook:**
- Gráficos de envolventes de amplitud
- Análisis de variaciones temporales
- Correlación entre eventos OSC y variaciones en audio

---

### 10. Ecuaciones Diferenciales Lineales y Sistemas Resonantes

**Aplicación en CALIB:**
- Modelado de resonadores modales como sistemas de ecuaciones diferenciales
- Análisis de comportamiento resonante en frecuencia
- Validación de modelos teóricos vs. datos empíricos

**Implementación:**
- Análisis de sistemas de segundo orden (resonadores)
- Identificación de frecuencias resonantes y factores de calidad
- Comparación con modelos teóricos

**Evidencia en notebook:**
- Análisis de comportamiento resonante en datos de audio
- Comparación con modelos teóricos de resonadores
- Validación de parámetros de sistemas resonantes

---

## Herramientas Matemáticas Involucradas

### Análisis Matemático

- **Cálculo diferencial:** Análisis de variaciones y derivadas de señales
- **Integración numérica:** Cálculo de energía y promedios
- **Análisis de Fourier:** DFT, STFT, propiedades espectrales

### Álgebra Lineal

- **Operaciones vectoriales:** Procesamiento de señales discretas
- **Transformaciones lineales:** DFT como transformación lineal
- **Espacios vectoriales:** Señales como vectores en espacios de dimensión finita

### Teoría de Sistemas

- **Sistemas discretos:** Modelado de sistemas LTI
- **Análisis de estabilidad:** Polos y ceros, región de convergencia
- **Respuesta en frecuencia:** Función de transferencia, diagramas de Bode

### Estadística

- **Estadística descriptiva:** Promedios, desviaciones estándar, distribuciones
- **Validación estadística:** Reproducibilidad, determinismo
- **Análisis de distribuciones:** Distribución de eventos, energía, parámetros

---

## Técnicas de Procesamiento de Señales

### Análisis Temporal

- Análisis de forma de onda
- Cálculo de RMS (Root Mean Square)
- Detección de picos y transitorios
- Análisis de envolventes

### Análisis Espectral

- DFT para análisis de frecuencia
- STFT para análisis tiempo-frecuencia
- Análisis de contenido espectral (centroide, rolloff, bandwidth)
- Identificación de componentes frecuenciales

### Análisis de Sistemas

- Respuesta al impulso
- Respuesta en frecuencia
- Función de transferencia
- Análisis de estabilidad

### Filtrado Digital

- Diseño de filtros FIR/IIR
- Aplicación de filtros para análisis
- Análisis de características de filtros (fase, estabilidad)

---

## Metodología de Validación

### Reproducibilidad Experimental

**Modelo:**
1. **Semilla determinística:** Uso de `/test/seed` para garantizar reproducibilidad
2. **Registro completo:** Todos los eventos y audio capturados en formato estructurado
3. **Metadatos completos:** Configuración y parámetros registrados en `meta.json`
4. **Validación cruzada:** Comparación de múltiples ejecuciones con misma semilla

**Implementación:**
- Semilla registrada en `meta.json`
- Validación de que misma semilla produce mismos resultados
- Análisis estadístico de variabilidad entre ejecuciones

### Validación Estadística

**Métricas:**
- Estadísticas descriptivas (media, desviación estándar, min, max)
- Distribuciones de eventos y parámetros
- Análisis de correlación entre eventos OSC y audio
- Validación de hipótesis sobre comportamiento del sistema

**Implementación:**
- Cálculo de métricas estadísticas en notebook
- Visualización de distribuciones
- Tests de hipótesis (si aplica)

### Validación Técnica

**Verificación:**
- Sincronización temporal entre eventos OSC y audio
- Precisión de timestamps
- Integridad de datos (validación de formato, rangos)
- Consistencia de metadatos

---

## Modelo de Reproducibilidad Experimental

### Componentes del Modelo

1. **Configuración determinística:**
   - Semilla de números aleatorios (`/test/seed`)
   - Parámetros de simulación registrados en `meta.json`
   - Configuración de hardware/software documentada

2. **Registro completo:**
   - Todos los eventos OSC capturados con timestamps precisos
   - Audio completo de la sesión
   - Metadatos de configuración

3. **Análisis reproducible:**
   - Notebook de análisis con código versionado
   - Resultados calculados de forma determinística
   - Métricas y reportes generados automáticamente

4. **Validación:**
   - Comparación de múltiples ejecuciones con misma configuración
   - Verificación de que resultados son idénticos (o dentro de tolerancia aceptable)

### Flujo de Reproducibilidad

```
Configuración (seed + params) → Ejecución → Registro → Análisis → Validación
         ↑                                                              ↓
         └─────────────────── Comparación ─────────────────────────────┘
```

---

## Criterios de Evaluación del TFI

### 1. Aplicación Fundamentada de Conceptos Trabajados en el Curso

**Evidencia requerida:**
- Uso explícito de DFT, STFT, Transformada Z, Convolución, Sistemas LTI
- Aplicación correcta de conceptos matemáticos
- Referencias teóricas cuando sea apropiado
- Código que demuestra comprensión de conceptos

**Cómo CALIB lo cumple:**
- Notebook implementa todas las técnicas mencionadas
- Código comentado con referencias a conceptos teóricos
- Visualizaciones que demuestran comprensión de conceptos

---

### 2. Claridad y Organización del Código

**Evidencia requerida:**
- Código bien estructurado y comentado
- Organización lógica de archivos y funciones
- Nombres de variables y funciones descriptivos
- Documentación clara

**Cómo CALIB lo cumple:**
- Estructura modular (CONTROL, REGISTRATION, ANALYSIS)
- Código Python en notebook con comentarios claros
- Especificaciones técnicas documentadas
- README con descripción clara del módulo

---

### 3. Calidad de la Visualización, Interacción o Síntesis Sonora

**Evidencia requerida:**
- Visualizaciones claras y informativas
- Gráficos que comunican información relevante
- Presentación profesional de resultados

**Cómo CALIB lo cumple:**
- Espectrogramas (STFT) con diferentes ventanas
- Gráficos de respuesta en frecuencia
- Visualizaciones de distribuciones estadísticas
- Reporte HTML con visualizaciones embebidas
- Comparaciones visuales (antes/después, teórico/empírico)

---

### 4. Coherencia entre el Enfoque Técnico y la Propuesta Creativa

**Evidencia requerida:**
- Integración coherente de aspectos técnicos y creativos
- Aplicación práctica que demuestra utilidad
- Conexión clara entre teoría y práctica

**Cómo CALIB lo cumple:**
- Herramienta práctica para calibración y validación del sistema SMA
- Análisis que informa mejoras al sistema creativo
- Integración con proyecto artístico existente (SMA)
- Aplicación de conceptos matemáticos a problema real

---

### 5. Presentación Clara y Sintética en la Memoria Técnica (1-3 páginas)

**Evidencia requerida:**
- Memoria técnica concisa (1-3 páginas)
- Descripción clara del proyecto
- Explicación de conceptos aplicados
- Resultados y conclusiones

**Cómo CALIB lo cumple:**
- Este documento (ACADEMIC_ALIGNMENT.md) puede servir como base para memoria técnica
- README del módulo con descripción clara
- Reporte HTML generado por notebook con resultados
- Documentación técnica completa en CALIB_SPEC.md

---

## Modalidad del Trabajo

### Modalidad 2: Herramienta Propia

**Descripción:** Desarrollo de una herramienta propia (función o módulo) para procesar o analizar señales. Aplicación de esa herramienta en un caso práctico que muestre su potencial.

**Cómo CALIB cumple esta modalidad:**

1. **Herramienta propia:**
   - Módulo completo de calibración y validación
   - Sistema de captura y registro de datos (NDJSON + WAV + metadata)
   - Notebook de análisis con técnicas de procesamiento de señales

2. **Aplicación práctica:**
   - Uso en contexto real del Sistema Modular Audiovisual
   - Análisis de señales generadas por el sistema creativo
   - Validación y calibración de parámetros del sistema

3. **Demostración de potencial:**
   - Análisis espectral que informa diseño de síntesis
   - Validación de reproducibilidad del sistema
   - Métricas que permiten optimización del sistema

---

## Bibliografía Relevante

### Referencias del Curso

- **Smith, Julius O.** (2007). *Introduction to Digital Filters with Audio Applications*. W3K Publishing.
- **Smith, Steven W.** (1997). *The Scientist and Engineer's Guide to Digital Signal Processing*. California Technical Publishing.
- **Moore, F. Richard** (1978). "An Introduction to the Mathematics of Digital Signal Processing: Part II: Sampling, Transforms, and Digital Filtering." *Computer Music Journal* 2 (4): 38–60.

### Referencias Adicionales para CALIB

- **Elliot, Douglas F.** (1987). *Handbook of Digital Signal Processing: Algorithms, Analysis, Applications*. Academic Press.
- **Loy, Gareth** (2006). *Musimathics: The Mathematical Foundations of Music*. Vol. 1. MIT Press.

### Herramientas de Software

- **NumPy:** Cálculos numéricos, FFT
- **SciPy:** Procesamiento de señales, filtros, análisis de sistemas
- **Librosa:** Análisis de audio, STFT, características espectrales
- **Matplotlib:** Visualizaciones y gráficos

---

## Estructura del Notebook de Análisis

El notebook `analysis_template.ipynb` está estructurado para cubrir todos los temas académicos:

1. **Carga de datos:** Lectura de NDJSON y WAV
2. **Análisis espectral (DFT):** Espectro de frecuencia
3. **Representaciones tiempo-frecuencia (STFT):** Espectrogramas con diferentes ventanas
4. **Análisis de sistemas LTI:** Respuesta en frecuencia, función de transferencia
5. **Convolución y filtrado:** Aplicación de filtros FIR/IIR
6. **Cálculo de energía y promedio:** Métricas temporales
7. **Análisis de envolventes:** Variaciones temporales
8. **Métricas y validación estadística:** Reproducibilidad, distribuciones
9. **Generación de reporte:** HTML con visualizaciones

---

## Conclusión

MAAD-2-CALIB está diseñado para cumplir explícitamente con los requisitos académicos del TFI de MAAD-2, demostrando aplicación fundamentada de conceptos matemáticos y técnicas de procesamiento digital de señales en un contexto práctico y creativo.

---

**Última actualización:** 2026-02-XX
