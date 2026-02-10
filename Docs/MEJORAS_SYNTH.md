# Propuestas de Mejora para el Sintetizador

## Estado Actual

El sintetizador actual usa **síntesis modal** con:
- 4 modos resonantes por voz
- Excitación: noise burst diferenciado (ruido blanco con high-pass)
- Polifonía: 4-12 voces
- Parámetros: Voices, Pitch (metalness)

## Problemas Identificados

- Controles eliminados (no funcionaban): brightness, damping, drive, reverb
- Timbre puede ser más rico y expresivo
- Excitación limitada a noise burst

## Propuestas de Mejora

### 1. Agregar Diferentes Formas de Onda para Excitación

**Objetivo**: Hacer la excitación más variada y expresiva

**Implementación**:
- Agregar selector de forma de onda en `ModalVoice`
- Formas disponibles:
  - **Noise** (actual) - ruido blanco diferenciado
  - **Sine** - impulso sinusoidal suave
  - **Square** - impulso cuadrado (más agresivo)
  - **Saw** - diente de sierra (más brillante)
  - **Triangle** - triangular (suave)
  - **Pulse** - pulso estrecho (muy agudo)
  - **Click** - impulso delta (muy percusivo)

**Ventajas**:
- Más variedad tímbrica
- Diferentes caracteres metálicos
- Control expresivo

**Costo RT**: Bajo (solo cambiar algoritmo de generación)

---

### 2. Agregar Osciladores Adicionales

**Objetivo**: Enriquecer el timbre sumando osciladores a la síntesis modal

**Opción A: Oscilador de Sub (Sub-oscillator)**
- Oscilador a 1 octava abajo (freq/2)
- Forma de onda: square o sine
- Mezcla ajustable
- Añade cuerpo y peso al sonido

**Opción B: Oscilador de Alta Frecuencia**
- Oscilador a 1-2 octavas arriba
- Forma de onda: square o saw
- Añade brillo y presencia

**Opción C: Oscilador de Detune**
- Múltiples osciladores con detune sutil (±5-10 cents)
- Efecto de "chorus" o "unison"
- Hace el sonido más rico y ancho

**Implementación**:
```cpp
class ModalVoice {
    // Osciladores adicionales
    float subOscPhase = 0.0f;
    float subOscFreq = 0.0f;
    float subOscMix = 0.0f; // 0-1
    
    float highOscPhase = 0.0f;
    float highOscFreq = 0.0f;
    float highOscMix = 0.0f;
    
    // Renderizar osciladores
    float renderSubOsc();
    float renderHighOsc();
};
```

**Ventajas**:
- Timbre más rico y completo
- Más opciones de diseño sonoro
- Mantiene síntesis modal como base

**Costo RT**: Medio (2-3 osciladores adicionales por voz)

---

### 3. Mejorar Síntesis Modal

**Opción A: Aumentar Modos**
- De 4 a 6-8 modos
- Más riqueza espectral
- Timbre más complejo

**Opción B: Modos Adaptativos**
- Modos que cambian según energía del hit
- Hits fuertes = más modos activos
- Hits suaves = menos modos

**Opción C: Modos con Envolventes Individuales**
- Cada modo con su propia envolvente ADSR
- Control más fino del decaimiento
- Timbre más expresivo

---

### 4. Agregar Modulación

**FM (Frequency Modulation)**
- Modular frecuencia de modos con oscilador
- Efectos: vibrato, timbre dinámico
- Parámetro: depth y rate

**AM (Amplitude Modulation)**
- Modular amplitud de modos
- Efectos: tremolo, textura
- Parámetro: depth y rate

**Ring Modulation**
- Multiplicar señal con oscilador
- Efectos: timbres metálicos, sidebands
- Muy útil para timbres metálicos

---

### 5. Mejorar Excitación Actual

**Opción A: Excitación Multi-tap**
- Múltiples impulsos con delays
- Simula múltiples impactos
- Textura más compleja

**Opción B: Excitación con Filtro**
- Aplicar filtro a la excitación antes de los modos
- Control de contenido espectral
- Más control tímbrico

**Opción C: Excitación Adaptativa**
- Forma de excitación según energía
- Energy alta = excitación más agresiva
- Energy baja = excitación más suave

---

## Recomendación Priorizada

### Fase 1: Mejoras Rápidas (Alto Impacto, Bajo Costo)
1. **Agregar formas de onda para excitación** ⭐
   - Selector de forma de onda
   - 4-5 formas básicas (sine, square, saw, triangle, click)
   - Impacto: Alto
   - Costo: Bajo

2. **Mejorar excitación actual**
   - Excitación adaptativa según energía
   - Filtro opcional en excitación
   - Impacto: Medio-Alto
   - Costo: Bajo

### Fase 2: Mejoras Medias (Alto Impacto, Costo Medio)
3. **Agregar oscilador de sub**
   - Sub-oscillator a 1 octava abajo
   - Mezcla ajustable
   - Impacto: Alto
   - Costo: Medio

4. **Aumentar modos a 6**
   - De 4 a 6 modos resonantes
   - Más riqueza espectral
   - Impacto: Alto
   - Costo: Medio (CPU)

### Fase 3: Mejoras Avanzadas (Alto Impacto, Alto Costo)
5. **Agregar modulación FM/AM**
   - Modulación de frecuencia y amplitud
   - Timbre dinámico
   - Impacto: Alto
   - Costo: Alto (complejidad)

6. **Osciladores adicionales múltiples**
   - Sub + High + Detune
   - Timbre muy rico
   - Impacto: Muy Alto
   - Costo: Alto (CPU)

---

## Implementación Sugerida: Formas de Onda

### Estructura Propuesta

```cpp
enum class ExcitationWaveform {
    Noise,      // Ruido blanco diferenciado (actual)
    Sine,       // Impulso sinusoidal
    Square,     // Impulso cuadrado
    Saw,        // Diente de sierra
    Triangle,   // Triangular
    Click,      // Impulso delta
    Pulse       // Pulso estrecho
};

class ModalVoice {
    ExcitationWaveform currentWaveform = ExcitationWaveform::Noise;
    
    void generateExcitation() {
        switch (currentWaveform) {
            case ExcitationWaveform::Sine:
                generateSineExcitation();
                break;
            case ExcitationWaveform::Square:
                generateSquareExcitation();
                break;
            // ... etc
        }
    }
};
```

### Ventajas
- Fácil de implementar
- Bajo costo RT
- Alto impacto en variedad tímbrica
- Compatible con síntesis modal actual

---

## Conclusión

**Mejora más recomendada**: Agregar formas de onda para excitación
- Implementación simple
- Alto impacto en variedad tímbrica
- Bajo costo de CPU
- Mantiene arquitectura actual

**Siguiente paso**: Implementar selector de forma de onda con 4-5 opciones básicas.
