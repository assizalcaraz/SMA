# JUCE — Especificación Técnica

Especificación técnica detallada del módulo de síntesis en JUCE. Documentación para desarrolladores.

**Última actualización:** v0.1 - Documentación inicial

---

## Estructura del Código

### Mapeo Archivos → Funcionalidad

| Archivo | Responsabilidad |
|---------|----------------|
| `Main.cpp` | Punto de entrada, creación de ventana y aplicación |
| `MainComponent.h` | Declaración de clase principal (UI, OSC, audio) |
| `MainComponent.cpp` | Implementación: UI, OSC receiver, audio callbacks, mapeo |
| `SynthesisEngine.h` | Declaración del motor de síntesis |
| `SynthesisEngine.cpp` | Implementación: orquestación, cola lock-free, limiter, mezcla |
| `ModalVoice.h` | Declaración de voz modal |
| `ModalVoice.cpp` | Implementación: resonadores modales, excitación, envolvente |
| `VoiceManager.h` | Declaración del gestor de polyphony |
| `VoiceManager.cpp` | Implementación: voice stealing, gestión de voces |
| `PlateSynth.h` | Declaración del sintetizador de placa |
| `PlateSynth.cpp` | Implementación: síntesis de placa metálica |
| `SynthParameters.h` | Parámetros globales del sintetizador |

---

## Clase ModalVoice

### Estructura de Datos

```cpp
class ModalVoice {
    static constexpr int NUM_MODES = 6; // Número de modos resonantes
    
    ResonantFilter modes[NUM_MODES];    // Filtros resonantes
    FormantFilter formantFilter;        // Filtro formant opcional
    SubOscillator subOsc;               // Sub-oscilador
    
    float currentBaseFreq;              // Frecuencia base actual
    float currentAmplitude;             // Amplitud actual
    float currentDamping;               // Damping actual
    float currentBrightness;            // Brightness actual
    float currentMetalness;             // Metalness actual
    ExcitationWaveform currentWaveform; // Forma de onda de excitación
    float currentSubOscMix;             // Mezcla de sub-oscilador
    
    float frequencyVariation[NUM_MODES]; // Variación aleatoria de frecuencias
    float excitationBuffer[128];         // Buffer de excitación
    float envelope;                       // Envolvente de decaimiento
    float envelopeDecay;                  // Tasa de decaimiento
    float residualAmplitude;             // Amplitud residual (para voice stealing)
};
```

### Factores Inarmónicos

```cpp
static constexpr float INHARMONIC_FACTORS[NUM_MODES] = {
    1.0f,      // Modo 0: fundamental
    2.76f,     // Modo 1: segundo modo inarmónico
    5.40f,     // Modo 2: tercer modo inarmónico
    8.93f,     // Modo 3: cuarto modo inarmónico
    13.34f,    // Modo 4: quinto modo inarmónico
    18.65f     // Modo 5: sexto modo inarmónico
};
```

**Nota**: Estos factores están optimizados para timbre metálico "Coin Cascade". Pueden necesitar calibración para sonar más metálico.

### Ganancias por Modo

```cpp
static constexpr float MODE_GAINS[NUM_MODES] = {
    1.0f,      // Modo 0: fundamental más fuerte
    0.8f,      // Modo 1: segundo modo
    0.9f,      // Modo 2: tercer modo (medio-alto, más brillante)
    0.7f,      // Modo 3: cuarto modo (alto)
    0.6f,      // Modo 4: quinto modo
    0.5f       // Modo 5: sexto modo
};
```

### Métodos

#### `ModalVoice::prepare(double sampleRate)`

Prepara la voz con el sample rate.

**Algoritmo**:
1. Guarda `currentSampleRate`
2. Resetea todos los filtros
3. Llama a `updateFilterCoefficients()`
4. Prepara filtro formant si está habilitado

#### `ModalVoice::setParameters(...)`

Configura los parámetros de la voz.

**Parámetros**:
- `baseFreq`: Frecuencia base (100-800 Hz)
- `amplitude`: Amplitud (0-1)
- `damping`: Damping (0-1, invertido: 0=corto, 1=largo)
- `brightness`: Brightness (0-1)
- `metalness`: Metalness (0-1)
- `waveform`: Forma de onda de excitación
- `subOscMix`: Mezcla de sub-oscilador (0-1)

**Algoritmo**:
1. Actualiza parámetros actuales
2. Si cambió frecuencia, brightness o metalness, actualiza coeficientes
3. Calcula `envelopeDecay` basado en damping (curva cúbica invertida)

**Fórmula de damping**:
```cpp
dampingInverted = 1.0 - damping;
dampingCurve = dampingInverted³;
decayTime = map(dampingCurve, 0.01, 5.0); // 10ms a 5 segundos
envelopeDecay = exp(-1.0 / (decayTime * sampleRate));
```

#### `ModalVoice::trigger()`

Inicia la excitación de la voz.

**Algoritmo**:
1. Regenera variaciones aleatorias de frecuencia (±2% por modo)
2. Actualiza coeficientes con nuevas variaciones
3. Configura sub-oscillator (una octava abajo)
4. Genera excitación según `currentWaveform`
5. Resetea envolvente a 1.0
6. Marca como activa

#### `ModalVoice::renderNextSample()`

Renderiza el siguiente sample.

**Algoritmo**:
1. Verifica si está activa (early return si `envelope < 0.0001`)
2. Genera excitación si aún está activa (con envolvente lineal)
3. Procesa excitación a través de los 6 modos resonantes
4. Aplica filtro formant si está habilitado
5. Aplica envolvente de decaimiento global
6. Agrega sub-oscillator si está activo
7. Actualiza `envelope *= envelopeDecay`
8. Actualiza `residualAmplitude` para voice stealing

**Fórmula de salida**:
```cpp
excitation = excitationBuffer[position] * (1.0 - position/length);
output = Σ modes[i].process(excitation);
output = formantFilter.process(output);
output *= envelope * amplitude;
output += subOsc.render() * subOscMix * envelope * amplitude;
```

#### `ModalVoice::updateFilterCoefficients()`

Actualiza los coeficientes de los filtros resonantes.

**Algoritmo**:
1. Para cada modo:
   - Calcula frecuencia: `freq = baseFreq * INHARMONIC_FACTORS[i] * frequencyVariation[i]`
   - Calcula Q: `Q = calculateModeQ(i)`
   - Calcula ganancia: `gain = MODE_GAINS[i] * brightness`
   - Actualiza coeficientes del filtro

#### `ModalVoice::generateExcitation()`

Genera la excitación según la forma de onda actual.

**Formas de onda**:
- **Noise**: Ruido blanco diferenciado (high-pass)
- **Sine**: Sinusoidal suave
- **Square**: Cuadrada agresiva
- **Saw**: Diente de sierra brillante
- **Triangle**: Triangular suave
- **Click**: Impulso delta percusivo
- **Pulse**: Pulso estrecho muy agudo

**Longitud**: ~5ms (128 samples a 48kHz)

---

## Clase VoiceManager

### Estructura de Datos

```cpp
class VoiceManager {
    static constexpr int DEFAULT_MAX_VOICES = 8;
    static constexpr int MAX_VOICES_LIMIT = 32; // Pre-allocation
    static constexpr int MIN_VOICES_LIMIT = 4;
    
    juce::OwnedArray<ModalVoice> voices; // Pool de voces (pre-allocado)
    int maxVoices;                        // Límite activo
    int voiceTriggerTime[MAX_VOICES_LIMIT]; // Tiempo de trigger (para voice stealing)
    int currentTime;                      // Contador de tiempo
};
```

### Métodos

#### `VoiceManager::prepare(double sampleRate, int maxVoices)`

Prepara el gestor con sample rate y número máximo de voces.

**Algoritmo**:
1. Guarda `currentSampleRate` y `maxVoices`
2. Pre-alloca hasta `MAX_VOICES_LIMIT` voces (32)
3. Llama a `prepare()` en cada voz
4. Inicializa `voiceTriggerTime`

#### `VoiceManager::triggerVoice(...)`

Obtiene una voz disponible y la configura, o roba una si es necesario.

**Algoritmo**:
1. Busca voz disponible con `findAvailableVoice()`
2. Si no hay disponible, busca voz para robar con `findVoiceToSteal()`
3. Configura parámetros de la voz
4. Llama a `voice.trigger()`
5. Actualiza `voiceTriggerTime[voiceIndex] = currentTime++`

#### `VoiceManager::findVoiceToSteal()`

Encuentra la mejor voz para robar.

**Algoritmo**:
1. Busca voz con menor `residualAmplitude`
2. Si hay empate, roba la más antigua (menor `voiceTriggerTime`)
3. Retorna puntero a la voz

#### `VoiceManager::renderNextBlock(...)`

Renderiza todas las voces activas en el buffer.

**Algoritmo**:
1. Limpia buffer
2. Para cada voz activa (`isActive()`):
   - Renderiza sample por sample
   - Suma al buffer
3. Retorna buffer mezclado

---

## Clase SynthesisEngine

### Estructura de Datos

```cpp
class SynthesisEngine {
    static constexpr int MAX_HITS_PER_BLOCK = 32;
    static constexpr int EVENT_QUEUE_SIZE = 128;
    
    VoiceManager voiceManager;
    PlateSynth plateSynth;
    
    juce::AbstractFifo eventFifo{EVENT_QUEUE_SIZE};
    HitEvent eventQueue[EVENT_QUEUE_SIZE];
    
    std::atomic<int> maxVoices{8};
    std::atomic<float> metalness{0.5f};
    std::atomic<int> waveform{0};
    std::atomic<float> subOscMix{0.0f};
    std::atomic<float> pitchRange{0.5f};
    std::atomic<bool> limiterEnabled{true};
    std::atomic<float> plateVolume{1.0f};
    
    float limiterThreshold = 0.95f;
    float limiterRatio = 10.0f;
    
    juce::AudioBuffer<float> plateBuffer; // Pre-allocado, tamaño MAX_BLOCK_SIZE
};
```

### Estructura HitEvent

```cpp
struct HitEvent {
    float baseFreq;
    float amplitude;
    float damping;
    float brightness;
    float metalness;
    ModalVoice::ExcitationWaveform waveform;
    float subOscMix;
};
```

### Métodos

#### `SynthesisEngine::prepare(double sampleRate)`

Prepara el motor con el sample rate.

**Algoritmo**:
1. Guarda `currentSampleRate`
2. Prepara `voiceManager` con `maxVoices`
3. Prepara `plateSynth`
4. Pre-alloca `plateBuffer` (2 canales, `MAX_BLOCK_SIZE` samples)
5. Calcula `outputLevelDecay` basado en sample rate

#### `SynthesisEngine::renderNextBlock(...)`

Renderiza el siguiente bloque de audio.

**Algoritmo**:
1. Procesa eventos de cola lock-free (`processEventQueue()`)
2. Actualiza parámetros globales periódicamente (cada `PARAMETER_UPDATE_INTERVAL` bloques)
3. Renderiza voces (`voiceManager.renderNextBlock()`)
4. Renderiza plate en buffer temporal
5. Mezcla plate con voces (pre-limiter) con control de volumen
6. Aplica limiter si está habilitado
7. Actualiza nivel de salida

**RT-safe**: Todo el procesamiento es RT-safe, sin allocations.

#### `SynthesisEngine::processEventQueue()`

Procesa eventos de la cola lock-free.

**Algoritmo**:
1. Lee hasta `MAX_HITS_PER_BLOCK` eventos de `eventFifo`
2. Para cada evento:
   - Llama a `voiceManager.triggerVoice()` con parámetros del evento
3. Retorna número de eventos procesados

**RT-safe**: Solo lectura de cola lock-free, sin allocations.

#### `SynthesisEngine::triggerVoiceFromOSC(...)`

Escribe evento a cola lock-free desde OSC thread.

**Algoritmo**:
1. Crea `HitEvent` con parámetros
2. Escribe a `eventFifo` (lock-free)
3. Si la cola está llena, descarta el evento (no bloquea)

**RT-safe**: Escritura lock-free, sin allocations.

#### `SynthesisEngine::applyLimiter(float sample)`

Aplica limiter suave al sample.

**Fórmula**:
```cpp
if (sample > threshold) {
    float excess = sample - threshold;
    float compressed = threshold + (excess / ratio);
    sample = compressed;
}
```

**Parámetros**:
- `threshold = 0.95f`
- `ratio = 10.0f`

---

## Clase PlateSynth

### Estructura de Datos

```cpp
class PlateSynth {
    static constexpr int NUM_PLATE_MODES = 6;
    static constexpr int TIMEOUT_MS = 2000;
    
    ResonantFilter modes[NUM_PLATE_MODES];
    
    std::atomic<float> currentFreq{220.0f};
    std::atomic<float> currentAmp{0.0f};
    std::atomic<int> currentMode{0};
    std::atomic<juce::int64> lastUpdateTime{0};
    
    float fadeOutGain = 1.0f;
    float fadeOutDecay = 0.999f;
    
    juce::Random noiseGen;
};
```

### Métodos

#### `PlateSynth::prepare(double sampleRate)`

Prepara el sintetizador con el sample rate.

**Algoritmo**:
1. Guarda `currentSampleRate`
2. Calcula `fadeOutDecay` basado en sample rate
3. Actualiza coeficientes de filtros

#### `PlateSynth::triggerPlate(float freq, float amp, int mode)`

Actualiza parámetros de la placa (RT-safe usando atomic).

**Algoritmo**:
1. Clampea parámetros a rangos válidos
2. Actualiza `currentFreq`, `currentAmp`, `currentMode` (atomic)
3. Actualiza `lastUpdateTime` (atomic)

#### `PlateSynth::renderNextBlock(...)`

Renderiza el siguiente bloque de audio.

**Algoritmo**:
1. Verifica timeout y actualiza `fadeOutGain` si es necesario
2. Si `currentAmp > 0.0` y `fadeOutGain > 0.0`:
   - Genera ruido blanco
   - Procesa a través de los 6 modos resonantes
   - Aplica `fadeOutGain`
3. Actualiza coeficientes periódicamente si cambiaron parámetros

**RT-safe**: Solo lectura de atomic, sin allocations.

#### `PlateSynth::updateFailSafe()`

Verifica timeout y actualiza fade-out gain.

**Algoritmo**:
1. Lee `lastUpdateTime` (atomic)
2. Calcula tiempo desde último update
3. Si `timeSinceUpdate > TIMEOUT_MS`:
   - Reduce `fadeOutGain *= fadeOutDecay`
4. Si `timeSinceUpdate <= TIMEOUT_MS`:
   - Restaura `fadeOutGain = 1.0f`

#### `PlateSynth::getInharmonicFactor(int modeIndex, int plateMode)`

Calcula factores inarmónicos según modo de placa.

**Modos de placa (0-7)**:
- Cada modo tiene diferentes factores inarmónicos
- Modo 0: Placa delgada (timbre más brillante)
- Modo 7: Placa gruesa (timbre más oscuro)

---

## Clase MainComponent

### Estructura de Datos

```cpp
class MainComponent : public juce::AudioAppComponent,
                     public juce::Slider::Listener,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener,
                     public juce::Timer,
                     private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
    SynthesisEngine synthesisEngine;
    
    juce::OSCReceiver oscReceiver;
    std::atomic<int> oscMessageCount{0};
    std::atomic<int> oscMessagesPerSecond{0};
    juce::int64 lastOscActivityTimestamp = 0;
    
    // UI controls
    juce::Slider voicesSlider;
    juce::Slider metalnessSlider;
    juce::ComboBox waveformComboBox;
    juce::Slider subOscMixSlider;
    juce::Slider pitchRangeSlider;
    juce::Slider plateVolumeSlider;
    juce::ToggleButton limiterToggle;
    juce::TextButton testTriggerButton;
    
    // Indicators
    juce::Label outputLevelLabel;
    juce::Label activeVoicesLabel;
    juce::Label oscStatusLabel;
    juce::Label oscMessageCountLabel;
};
```

### Métodos

#### `MainComponent::MainComponent()`

Constructor: inicializa UI y OSC receiver.

**Algoritmo**:
1. Configura sliders y controles UI
2. Inicializa OSC receiver en puerto 9000
3. Registra listener para mensajes OSC
4. Inicia timer para indicadores (50ms)
5. Configura audio channels

#### `MainComponent::oscMessageReceived(const juce::OSCMessage& message)`

Callback para mensajes OSC recibidos.

**Algoritmo**:
1. Actualiza `lastOscActivityTimestamp` y contadores
2. Extrae dirección del mensaje
3. Rutea a handler apropiado:
   - `/hit` → `mapOSCHitToEvent()`
   - `/state` → `updateOSCState()`
   - `/plate` → `mapOSCPlateToEvent()`

**Thread-safe**: Se ejecuta en OSC thread (no audio thread).

#### `MainComponent::mapOSCHitToEvent(const juce::OSCMessage& message)`

Mapea mensaje `/hit` a evento de síntesis.

**Algoritmo**:
1. Valida formato del mensaje (5 parámetros)
2. Extrae y clampea valores:
   - `id`, `x`, `y`, `energy`, `surface`
3. Mapea parámetros:
   - `energy` → `amplitude = energy^1.5`
   - `energy` → `brightness = lerp(0.3, 1.0, energy)`
   - `y` → `damping = lerp(0.2, 0.8, 1 - y)`
   - `y` → `baseFreq = 200 + (y * 400)` (con variación aleatoria)
   - `x` → `pan = (x * 2) - 1` (futuro)
4. Selecciona waveform según energía (adaptativo)
5. Llama a `synthesisEngine.triggerVoiceFromOSC()`

**Fórmulas de mapeo**:
```cpp
amplitude = pow(energy, 1.5f);
brightness = 0.3f + (energy * 0.7f);
damping = 0.2f + ((1.0f - y) * 0.6f);
baseFreq = 300.0f + randomVariation * pitchRange * 200.0f;
```

#### `MainComponent::mapOSCPlateToEvent(const juce::OSCMessage& message)`

Mapea mensaje `/plate` a PlateSynth.

**Algoritmo**:
1. Valida formato del mensaje (3 parámetros)
2. Extrae y clampea valores:
   - `freq` (20-2000 Hz)
   - `amp` (0-1)
   - `mode` (0-7)
3. Llama a `synthesisEngine.triggerPlateFromOSC()`

#### `MainComponent::getNextAudioBlock(...)`

Audio callback: renderiza bloque de audio.

**Algoritmo**:
1. Llama a `synthesisEngine.renderNextBlock()`
2. Retorna buffer de audio

**RT-safe**: Solo llama a métodos RT-safe de `SynthesisEngine`.

---

## Mapeo OSC Técnico

### Mensaje `/hit`

**Formato**: `/hit <id:int32> <x:float> <y:float> <energy:float> <surface:int32>`

**Mapeo**:

| Parámetro | Tipo | Rango | Mapeo | Fórmula |
|-----------|------|-------|-------|---------|
| `id` | `int32` | 0-N | (no usado) | - |
| `x` | `float` | 0.0-1.0 | `pan` | `pan = (x * 2) - 1` (futuro) |
| `y` | `float` | 0.0-1.0 | `damping`, `baseFreq` | `damping = lerp(0.2, 0.8, 1-y)`, `freq = 200 + (y * 400)` |
| `energy` | `float` | 0.0-1.0 | `amplitude`, `brightness` | `amp = energy^1.5`, `brightness = lerp(0.3, 1.0, energy)` |
| `surface` | `int32` | 0-3, -1 | (futuro) | - |

**Waveform adaptativo**:
- `energy > 0.7`: Click o Pulse (más percusivo)
- `energy 0.3-0.7`: Noise (default)
- `energy < 0.3`: Sine o Triangle (más suave)

### Mensaje `/state`

**Formato**: `/state <activity:float> <gesture:float> <presence:float>`

**Mapeo** (parcialmente implementado):

| Parámetro | Tipo | Rango | Mapeo | Estado |
|-----------|------|-------|-------|--------|
| `activity` | `float` | 0.0-1.0 | Global wet (reverb) | No implementado |
| `gesture` | `float` | 0.0-1.0 | Global drive/saturación | No implementado |
| `presence` | `float` | 0.0-1.0 | Master level | Parcialmente implementado |

### Mensaje `/plate`

**Formato**: `/plate <freq:float> <amp:float> <mode:int32>`

**Mapeo**:

| Parámetro | Tipo | Rango | Mapeo | Fórmula |
|-----------|------|-------|-------|---------|
| `freq` | `float` | 20-2000 Hz | `PlateSynth::currentFreq` | Clamp a rango |
| `amp` | `float` | 0.0-1.0 | `PlateSynth::currentAmp` | Clamp a 0-1 |
| `mode` | `int32` | 0-7 | `PlateSynth::currentMode` | Clamp a 0-7 |

---

## Implementación RT-Safe

### Principios

1. **Sin allocations en audio thread**: Todas las estructuras pre-allocadas
2. **Cola lock-free**: `AbstractFifo` para comunicación entre threads
3. **Parámetros atómicos**: `std::atomic` para parámetros globales
4. **Buffers pre-allocados**: Tamaño máximo conocido

### Cola de Eventos

**Implementación**:
```cpp
juce::AbstractFifo eventFifo{EVENT_QUEUE_SIZE}; // 128 eventos
HitEvent eventQueue[EVENT_QUEUE_SIZE];          // Array fijo
```

**Uso**:
- **Escritura** (OSC thread): `eventFifo.write()` - lock-free
- **Lectura** (Audio thread): `eventFifo.read()` - lock-free
- **Límite**: `MAX_HITS_PER_BLOCK = 32` eventos por bloque

### Pre-allocation

**Voces**:
- Pre-allocation: hasta 32 voces
- Solo se activan según `maxVoices` (4-12)
- Sin allocations en `triggerVoice()`

**Buffers**:
- `plateBuffer`: Pre-allocado en `prepare()` (2 canales, `MAX_BLOCK_SIZE` samples)
- Sin reallocations en runtime

### Parámetros Atómicos

**Parámetros globales**:
```cpp
std::atomic<int> maxVoices{8};
std::atomic<float> metalness{0.5f};
std::atomic<int> waveform{0};
std::atomic<float> subOscMix{0.0f};
std::atomic<float> pitchRange{0.5f};
std::atomic<bool> limiterEnabled{true};
std::atomic<float> plateVolume{1.0f};
```

**Uso**:
- **Escritura** (UI thread): `atomic.store(value)`
- **Lectura** (Audio thread): `atomic.load()`

---

## Notas de Diseño

### JUCE 8.0.12 OSCReceiver

**Implementación**:
- Usa `OSCReceiver::Listener<MessageLoopCallback>` pattern
- Callback `oscMessageReceived()` se ejecuta en OSC thread (no audio thread)
- Mensajes se escriben a cola lock-free para procesamiento en audio thread

**Problemas conocidos**:
- Ver `INFORME_ERROR_OSC_JUCE8.md` para detalles de implementación
- API de JUCE 8.0.12 requiere listener pattern específico

### Limitaciones Conocidas

**Sliders no afectan significativamente**:
- Algunos controles (brightness, damping) tienen efecto limitado
- El timbre depende principalmente de factores inarmónicos fijos
- Requiere calibración de parámetros en código

**Timbre no suena metálico**:
- Problema conocido: sonido más "pluc" de madera que metálico
- Posibles causas:
  - Factores inarmónicos no optimizados
  - Excitación no adecuada
  - Falta de modulación o efectos

### Decisiones de Implementación

**Voice stealing**:
- Roba voz con menor amplitud residual o más antigua
- Prioriza mantener textura "cascade" sin gaps

**PlateSynth fail-safe**:
- Fade-out automático después de 2 segundos sin updates
- Previene sonido "pegado" si App A se desconecta

**Mapeo exponencial de Plate Volume**:
- Slider 0.1 → Volumen 0.5 (mitad del rango)
- Permite control preciso en volúmenes bajos
- Balancea con audio de partículas (plate suena más fuerte)

---

## Rendimiento

### Objetivos

- **CPU usage**: < 30% con 8 voces activas
- **Latencia**: < 10ms end-to-end (OSC → audio)
- **Estabilidad**: Sin glitches bajo carga normal (hasta 200 hits/segundo)

### Optimizaciones

- Pre-allocation de todas las estructuras
- Early returns en `renderNextSample()` si voz inactiva
- Actualización periódica de parámetros globales (cada 4 bloques)
- Procesamiento optimizado de modos (loop unrolled donde sea posible)

---

## Dependencias

- **JUCE** 8.0.12
- **juce_osc** — Comunicación OSC
- **juce_audio_basics** — Procesamiento de audio
- **juce_audio_processors** — Procesamiento avanzado
- **juce_gui_basics** — Interfaz de usuario

---

**Última actualización**: 2026-02-11
