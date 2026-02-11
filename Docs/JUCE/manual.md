# JUCE — Manual de Usuario

Manual de usuario del módulo de síntesis en JUCE. Guía completa de parámetros y uso.

**Última actualización:** v0.1 - Documentación inicial

**Ver también:**
- [`readme.md`](readme.md) - Descripción general del módulo
- [`spec.md`](spec.md) - Especificación técnica para desarrolladores

---

## Ejecución de la Aplicación

### Iniciar la Aplicación

1. **Compilar y ejecutar:**
   - Abrir proyecto en Xcode (o IDE correspondiente)
   - Compilar (Cmd+B)
   - Ejecutar (Cmd+R)

2. **Verificar conexión OSC:**
   - El indicador "OSC Status" debe mostrar "OSC: Connected (port 9000)" en verde
   - Si muestra "Disconnected" en rojo, verificar que el puerto 9000 no esté en uso

3. **Iniciar App A (Particles):**
   - Ejecutar la aplicación Particles en openFrameworks
   - Las partículas comenzarán a enviar mensajes OSC automáticamente

### Configuración OSC

**Puerto por defecto**: 9000

**Mensajes esperados**:
- `/hit` — Eventos de colisión de partículas (principal)
- `/state` — Estado global del sistema (opcional)
- `/plate` — Control de vibración de placa (paralelo)

**Verificación**:
- El contador "OSC Messages" debe incrementarse cuando App A está activa
- Si no hay mensajes, verificar:
  - Que App A esté ejecutándose
  - Que App A esté enviando al puerto 9000
  - Que no haya firewall bloqueando UDP

---

## Controles de la Interfaz

### Voices

**Slider:** `Voices`  
**Tipo:** `int`  
**Rango:** 4 - 12  
**Valor por defecto:** 8  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Número máximo de voces activas simultáneamente. Controla la polyphony del sintetizador.

**Efectos:**
- **Valores bajos (4-6):** Menos voces, menos CPU, textura más simple
- **Valores medios (7-9):** Balance entre polyphony y rendimiento (recomendado)
- **Valores altos (10-12):** Más voces, más CPU, textura más densa

**Nota:** El sistema usa voice stealing cuando todas las voces están activas. Las voces con menor amplitud residual o más antiguas son robadas para nuevas notas.

---

### Pitch / Metalness

**Slider:** `Pitch`  
**Tipo:** `float`  
**Rango:** 0.0 - 1.0  
**Valor por defecto:** 0.5  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Control de timbre metálico. Afecta los factores inarmónicos y la característica tímbrica del sonido.

**Efectos:**
- **Valores bajos (0.0-0.3):** Timbre más suave, menos metálico
- **Valores medios (0.4-0.6):** Timbre balanceado (valor por defecto)
- **Valores altos (0.7-1.0):** Timbre más metálico, más brillante

**Uso:** Ajustar según el carácter tímbrico deseado. Valores altos = más "coin cascade" metálico, valores bajos = más suave.

**Nota:** Actualmente este control tiene efecto limitado. El timbre metálico depende principalmente de los factores inarmónicos fijos en el código.

---

### Waveform

**ComboBox:** `Waveform`  
**Tipo:** `enum`  
**Opciones:** Noise, Sine, Square, Saw, Triangle, Click, Pulse  
**Valor por defecto:** Noise  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Forma de onda de excitación para los resonadores modales. Cada forma de onda produce un carácter tímbrico diferente.

**Opciones:**
- **Noise:** Ruido diferenciado (default). Timbre percusivo y metálico.
- **Sine:** Sinusoidal suave. Timbre más puro y menos percusivo.
- **Square:** Cuadrada agresiva. Timbre más brillante y rico en armónicos.
- **Saw:** Diente de sierra brillante. Timbre muy brillante y agresivo.
- **Triangle:** Triangular suave. Timbre intermedio entre sine y square.
- **Click:** Impulso delta percusivo. Timbre muy percusivo y seco.
- **Pulse:** Pulso estrecho muy agudo. Timbre extremadamente percusivo.

**Uso:** Experimentar con diferentes formas de onda para encontrar el carácter tímbrico deseado. Noise es el default y funciona bien para "coin cascade".

---

### SubOsc Mix

**Slider:** `SubOsc Mix`  
**Tipo:** `float`  
**Rango:** 0.0 - 1.0  
**Valor por defecto:** 0.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Mezcla de sub-oscilador. Agrega un oscilador adicional una octava más baja para timbre más rico y grave.

**Efectos:**
- **Valor 0.0:** Sin sub-oscilador (solo resonadores modales)
- **Valores bajos (0.1-0.3):** Mezcla sutil, timbre ligeramente más grave
- **Valores medios (0.4-0.6):** Mezcla moderada, timbre más rico
- **Valores altos (0.7-1.0):** Mezcla fuerte, timbre muy grave y rico

**Uso:** Ajustar para agregar cuerpo y profundidad al sonido. Valores altos pueden hacer el sonido más "pesado".

**Nota:** El sub-oscilador usa forma de onda Square por defecto.

---

### Pitch Range

**Slider:** `Pitch Range`  
**Tipo:** `float`  
**Rango:** 0.0 - 1.0  
**Valor por defecto:** 0.5  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Rango de variación aleatoria de pitch. Agrega variación sutil en las frecuencias para timbre más rico y menos repetitivo.

**Efectos:**
- **Valor 0.0:** Sin variación, todas las voces suenan idénticas
- **Valores bajos (0.1-0.3):** Variación sutil, timbre más consistente
- **Valores medios (0.4-0.6):** Variación moderada (valor por defecto)
- **Valores altos (0.7-1.0):** Variación grande, timbre más variado pero menos consistente

**Uso:** Ajustar para controlar la consistencia tímbrica. Valores altos = más variación, valores bajos = más consistente.

---

### Plate Volume

**Slider:** `Plate Volume`  
**Tipo:** `float`  
**Rango:** 0.0 - 1.0  
**Valor por defecto:** 1.0  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Volumen del sintetizador de placa metálica (PlateSynth). Controla el nivel de mezcla del audio de placa con el audio de partículas.

**Mapeo exponencial:**
- **Slider 0.0:** Volumen 0.0 (silencio completo)
- **Slider 0.1:** Volumen 0.5 (mitad del rango de amplitud)
- **Slider 1.0:** Volumen 1.0 (volumen completo)

**Efectos:**
- **Valor 0.0:** PlateSynth silenciado, solo se escuchan partículas
- **Valores bajos (0.1-0.3):** PlateSynth sutil, mezcla balanceada
- **Valores medios (0.4-0.7):** Mezcla moderada (recomendado)
- **Valores altos (0.8-1.0):** PlateSynth prominente, puede dominar el sonido

**Uso:** Ajustar según el balance deseado entre partículas y placa. El mapeo exponencial permite control preciso en volúmenes bajos.

**Nota:** PlateSynth solo produce sonido si recibe mensajes `/plate` desde App A. Si no hay mensajes, el volumen no tiene efecto.

---

### Limiter

**Toggle:** `Limiter`  
**Tipo:** `bool`  
**Valor por defecto:** `true` (habilitado)  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Habilita o deshabilita el limiter master. El limiter previene saturación y clipping cuando hay muchos eventos simultáneos.

**Efectos:**
- **Habilitado (ON):** Limiter activo, previene saturación, sonido más controlado
- **Deshabilitado (OFF):** Sin limiter, puede haber saturación con muchos eventos, sonido más dinámico pero puede distorsionar

**Uso:** Mantener habilitado para uso normal. Deshabilitar solo para testing o si se quiere saturación intencional.

**Nota:** El limiter tiene threshold 0.95 y ratio 10.0. Es un limiter suave que no introduce distorsión audible en uso normal.

---

## Indicadores

### OSC Status

**Label:** `OSC Status`  
**Valores:** "OSC: Connected (port 9000)" (verde) o "OSC: Disconnected" (rojo)

**Descripción:**  
Indica el estado de la conexión OSC. Debe mostrar "Connected" en verde cuando el receptor OSC está activo.

**Troubleshooting:**
- Si muestra "Disconnected": Verificar que el puerto 9000 no esté en uso
- Si muestra "Connection failed": Verificar permisos de red o firewall

---

### OSC Messages

**Label:** `OSC Messages`  
**Formato:** "OSC Messages: X/s"  
**Unidad:** mensajes por segundo

**Descripción:**  
Contador de mensajes OSC recibidos por segundo. Se actualiza cada 50ms.

**Uso:** Verificar que App A esté enviando mensajes. Si el contador está en 0, no hay comunicación OSC activa.

---

### Active Voices

**Label:** `Active Voices`  
**Formato:** "Active Voices: X"  
**Unidad:** número de voces

**Descripción:**  
Número de voces activas actualmente. Indica cuántas voces están resonando simultáneamente.

**Uso:** Monitorear la carga de polyphony. Si siempre está en el máximo (según slider Voices), puede ser necesario aumentar el número de voces.

---

### Output Level

**Label:** `Output Level`  
**Formato:** "Output: X.XX dB"  
**Unidad:** decibelios (RMS)

**Descripción:**  
Nivel de salida de audio en decibelios. Indica el nivel RMS de la señal de salida.

**Uso:** Monitorear el nivel de salida. Valores altos (> 0 dB) pueden indicar saturación. Valores muy bajos (< -40 dB) pueden indicar que no hay sonido.

---

## Botón Test Trigger

**Botón:** `Test Trigger`  
**Tipo:** `TextButton`  
**Ajustable en tiempo real:** Sí

**Descripción:**  
Botón para probar el sintetizador sin necesidad de mensajes OSC. Genera un evento de prueba con parámetros fijos.

**Uso:** 
- Probar que el sintetizador funciona correctamente
- Verificar que el audio está configurado
- Ajustar parámetros sin depender de App A

**Parámetros de prueba:**
- Frecuencia: 220 Hz
- Amplitud: 0.7
- Damping: 0.5
- Brightness: 0.5
- Metalness: valor actual del slider

---

## Cómo Afectan los Parámetros al Sonido

### Textura "Coin Cascade"

El objetivo sonoro es crear una textura metálica característica donde múltiples hits percusivos breves se acumulan formando una cascada granular. Cada colisión de partícula produce un sonido, desde micro-scrapes (casi imperceptibles) hasta golpes fuertes.

**Parámetros clave:**
- **Voices:** Controla la densidad de la cascada (más voces = más densidad)
- **Pitch/Metalness:** Controla el carácter metálico (valores altos = más metálico)
- **Waveform:** Controla el carácter percusivo (Noise, Click, Pulse = más percusivo)
- **Plate Volume:** Agrega textura de fondo de placa metálica

### Interacciones entre Parámetros

**Voices + Pitch Range:**
- Voices alto + Pitch Range alto = cascada muy densa y variada
- Voices bajo + Pitch Range bajo = cascada más simple y consistente

**Waveform + SubOsc Mix:**
- Waveform percusivo (Click, Pulse) + SubOsc Mix bajo = sonido más seco y percusivo
- Waveform suave (Sine, Triangle) + SubOsc Mix alto = sonido más rico y grave

**Plate Volume + Limiter:**
- Plate Volume alto + Limiter ON = mezcla controlada sin saturación
- Plate Volume alto + Limiter OFF = puede saturar si hay muchos eventos

---

## Ejemplos de Mensajes OSC

### Mensaje `/hit`

**Formato:**
```
/hit <id> <x> <y> <energy> <surface>
```

**Ejemplo:**
```
/hit 42 0.75 0.3 0.65 1
```

**Interpretación:**
- Partícula ID 42
- Posición: (0.75, 0.3) — derecha, parte superior
- Energía: 0.65 — impacto moderado-alto
- Superficie: 1 — borde derecho

**Mapeo en JUCE:**
- `energy 0.65` → `amplitude = 0.65^1.5 ≈ 0.52`, `brightness = 0.54`
- `y 0.3` → `damping = 0.74`, `baseFreq = 320 Hz`
- `x 0.75` → `pan = 0.5` (futuro, no implementado)

Ver [Contrato OSC completo](../api-osc.md) para más detalles.

---

### Mensaje `/state`

**Formato:**
```
/state <activity> <gesture> <presence>
```

**Ejemplo:**
```
/state 0.45 0.32 0.89
```

**Interpretación:**
- Actividad: 45% del máximo
- Gesto: 32% de energía
- Presencia: 89% de confianza (usuario bien detectado)

**Mapeo en JUCE:**
- Actualmente parcialmente implementado
- `presence` afecta el nivel master (futuro)

Ver [Contrato OSC completo](../api-osc.md) para más detalles.

---

### Mensaje `/plate`

**Formato:**
```
/plate <freq> <amp> <mode>
```

**Ejemplo:**
```
/plate 440.0 0.75 3
```

**Interpretación:**
- Frecuencia: 440.0 Hz (nota A4)
- Amplitud: 0.75 (75% de excitación)
- Modo: 3 (configuración intermedia)

**Mapeo en JUCE:**
- `freq 440.0` → `PlateSynth::currentFreq = 440.0`
- `amp 0.75` → `PlateSynth::currentAmp = 0.75`
- `mode 3` → `PlateSynth::currentMode = 3`

Ver [Contrato OSC completo](../api-osc.md) para más detalles.

---

## Valores Recomendados por Escenario

### Escenario 1: Cascada Sutil y Controlada
```
Voices: 6
Pitch/Metalness: 0.4
Waveform: Noise
SubOsc Mix: 0.0
Pitch Range: 0.3
Plate Volume: 0.3
Limiter: ON
```

### Escenario 2: Cascada Densa y Metálica (Por defecto)
```
Voices: 8
Pitch/Metalness: 0.5
Waveform: Noise
SubOsc Mix: 0.0
Pitch Range: 0.5
Plate Volume: 0.5
Limiter: ON
```

### Escenario 3: Cascada Agresiva y Percusiva
```
Voices: 12
Pitch/Metalness: 0.7
Waveform: Click
SubOsc Mix: 0.2
Pitch Range: 0.7
Plate Volume: 0.6
Limiter: ON
```

### Escenario 4: Cascada Rica y Grave
```
Voices: 8
Pitch/Metalness: 0.5
Waveform: Square
SubOsc Mix: 0.5
Pitch Range: 0.4
Plate Volume: 0.4
Limiter: ON
```

---

## Troubleshooting

### No se escucha sonido

**Síntomas:** La app ejecuta pero no hay audio.

**Soluciones:**
1. Verificar que la interfaz de audio esté configurada correctamente
2. Verificar permisos de audio en macOS (System Preferences → Security)
3. Verificar que el output level no esté en 0
4. Probar con el botón "Test Trigger" para verificar síntesis
5. Verificar que App A esté enviando mensajes OSC (contador "OSC Messages" > 0)

### El contador OSC no incrementa

**Síntomas:** El contador "OSC Messages" está en 0.

**Soluciones:**
1. Verificar que App A (Particles) esté ejecutándose
2. Verificar que App A esté enviando al puerto 9000
3. Verificar que el indicador "OSC Status" muestre "Connected" en verde
4. Verificar firewall no bloquea UDP puerto 9000
5. Usar herramienta de debug OSC (OSCulator, etc.) para verificar mensajes

### Alto uso de CPU

**Síntomas:** CPU usage > 50% con pocas voces.

**Soluciones:**
1. Reducir número de voces (slider Voices a 4-6)
2. Verificar que estés usando build Release (no Debug)
3. Aumentar tamaño de buffer de audio (mayor latencia pero menor CPU)
4. Cerrar otras aplicaciones que consuman CPU
5. Deshabilitar PlateSynth (Plate Volume a 0.0) si no se usa

### El timbre no suena metálico

**Síntomas:** El sonido suena más a "pluc" de madera que metálico.

**Soluciones:**
1. Ajustar slider Pitch/Metalness hacia valores más altos (0.7-1.0)
2. Probar diferentes waveforms (Noise, Click, Pulse)
3. Verificar que los factores inarmónicos estén correctos (requiere modificar código)
4. Ajustar parámetros de síntesis (requiere modificar código)

**Nota:** Este es un problema conocido. El timbre metálico depende principalmente de los factores inarmónicos fijos en el código, que pueden necesitar calibración.

---

## Notas de Implementación

- Todos los parámetros son ajustables en tiempo real sin reiniciar la aplicación
- Los cambios se aplican inmediatamente en el siguiente bloque de audio
- El sistema está optimizado para mantener CPU usage < 30% con 8 voces activas
- Los valores por defecto están calibrados para una experiencia balanceada

---

## Referencias

- **[Contrato OSC](../api-osc.md)** — Contrato completo de mensajes OSC
- **[Descripción general](readme.md)** — Arquitectura del módulo
- **[Especificación técnica](spec.md)** — Detalles de implementación

---

**Última actualización**: 2026-02-11
