# Informe de Error: Integración OSCReceiver en JUCE 8.0.12

## Contexto del Proyecto

**Proyecto:** Sistema Modular Audiovisual  
**Aplicación:** app-JUCE-PAS1 (JUCE Standalone Application)  
**Versión JUCE:** 8.0.12  
**Plataforma:** macOS  
**Objetivo:** Implementar receptor OSC para recibir mensajes `/hit` y `/state` desde openFrameworks

## Estado Actual

- ✅ Módulo `juce_osc` agregado al proyecto `.jucer` y visible en Projucer
- ✅ Proyecto regenerado desde Projucer (Save and Open in IDE)
- ✅ `JuceHeader.h` incluye `#include <juce_osc/juce_osc.h>`
- ✅ `OSCReceiver` se reconoce (no hay errores de tipo desconocido)
- ❌ **Error de compilación:** `MainComponent` es una clase abstracta
- ❌ **Error de compilación:** Método `override` no coincide con función virtual

## Errores de Compilación

### Error 1: Clase Abstracta
```
/path/to/project/Main.cpp:68:39 
Allocating an object of abstract class type 'MainComponent'
```

**Ubicación:** `Main.cpp:68`
```cpp
auto* mainComponent = new MainComponent();
```

**Causa:** `MainComponent` hereda de una clase base que tiene métodos virtuales puros sin implementar.

### Error 2: Método Override Incorrecto
```
/path/to/project/MainComponent.h:98:102 
Non-virtual member function marked 'override' hides virtual member function
```

**Ubicación:** `MainComponent.h:98`
```cpp
void oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message) override;
```

**Causa:** La firma del método no coincide con la función virtual esperada por la clase base.

## Implementación Actual

### Herencia de MainComponent
```cpp
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::Timer,
                       public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
```

### Registro de Listeners (Constructor)
```cpp
if (oscReceiver.connect(9000))
{
    // Register listeners for specific OSC address patterns
    oscReceiver.addListener(this, "/hit");
    oscReceiver.addListener(this, "/state");
    // ...
}
```

### Implementación del Callback
```cpp
void MainComponent::oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message)
{
    // Update activity timestamp
    lastOscActivityTimestamp = juce::Time::currentTimeMillis();
    oscMessageCountAccumulator++;
    
    // Parse message address pattern
    juce::String address = pattern.toString();
    
    if (address == "/hit")
    {
        mapOSCHitToEvent(message);
    }
    else if (address == "/state")
    {
        updateOSCState(message);
    }
}
```

## Análisis del Problema

### Hipótesis 1: API Incorrecta de ListenerWithOSCAddress
La clase `OSCReceiver::ListenerWithOSCAddress<MessageLoopCallback>` puede requerir:
- Una firma diferente del callback
- Métodos virtuales adicionales que no se están implementando
- Un tipo de callback diferente al especificado

### Hipótesis 2: MessageLoopCallback Incorrecto
El template parameter `juce::OSCReceiver::MessageLoopCallback` puede no ser el correcto. Posibles alternativas:
- `OSCReceiver::RealtimeCallback`
- `OSCReceiver::AsyncCallback`
- Otro tipo de callback

### Hipótesis 3: Métodos Virtuales Faltantes
La clase base puede requerir implementar métodos adicionales además de `oscMessageReceived()`.

## Información de la API JUCE 8.0.12

### Módulo OSC
- **Ubicación:** `/ruta/a/JUCE/modules/juce_osc/` (típicamente `/Applications/JUCE/modules/juce_osc/` en macOS)
- **Versión:** 8.0.12
- **Incluido en:** `JuceLibraryCode/JuceHeader.h` (después de regenerar)

### OSCReceiver
- **Tipo:** `juce::OSCReceiver`
- **Método de conexión:** `bool connect(int portNumber)`
- **Método de registro:** `addListener(ListenerType*, const OSCAddressPattern&)` (probable)

### Listener Pattern
- **Clase base intentada:** `OSCReceiver::ListenerWithOSCAddress<CallbackType>`
- **Callback esperado:** Probablemente diferente a la implementación actual

## Requisitos de la Solución

1. **Mantener contrato OSC:** No cambiar el formato de mensajes `/hit` y `/state`
2. **Preservar lógica de mapeo:** `mapOSCHitToEvent()` y `updateOSCState()` deben funcionar igual
3. **No modificar motor de síntesis:** `SynthesisEngine` no debe cambiar
4. **Compilar en JUCE 8.0.12:** La solución debe funcionar con esta versión específica

## Código Relevante

### MainComponent.h (líneas clave)
```cpp
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::Timer,
                       public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
    // ...
    void oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message) override;
    // ...
    juce::OSCReceiver oscReceiver;
    // ...
};
```

### MainComponent.cpp (registro de listeners)
```cpp
if (oscReceiver.connect(9000))
{
    oscReceiver.addListener(this, "/hit");
    oscReceiver.addListener(this, "/state");
}
```

### MainComponent.cpp (callback)
```cpp
void MainComponent::oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message)
{
    juce::String address = pattern.toString();
    if (address == "/hit")
        mapOSCHitToEvent(message);
    else if (address == "/state")
        updateOSCState(message);
}
```

## Lo Que Se Necesita

1. **Verificar la API correcta de JUCE 8.0.12:**
   - ¿Cuál es la clase base correcta para el listener?
   - ¿Cuál es la firma exacta del callback requerido?
   - ¿Qué métodos virtuales deben implementarse?

2. **Corregir la herencia:**
   - Usar la clase base correcta
   - Implementar todos los métodos virtuales requeridos

3. **Corregir el callback:**
   - Ajustar la firma del método para que coincida con la función virtual
   - Asegurar que el patrón de dirección se reciba correctamente

4. **Verificar el registro:**
   - Confirmar que `addListener(this, "/hit")` es la sintaxis correcta
   - Verificar que los patrones de dirección se registren correctamente

## Archivos del Proyecto

### Archivos Modificados
- `app-juce/app-JUCE-PAS1/Source/MainComponent.h`
- `app-juce/app-JUCE-PAS1/Source/MainComponent.cpp`
- `app-juce/app-JUCE-PAS1/app-JUCE-PAS1.jucer` (módulo juce_osc agregado)
- `app-juce/app-JUCE-PAS1/JuceLibraryCode/JuceHeader.h` (include de juce_osc agregado)

### Archivos No Modificados (No Tocar)
- `app-juce/app-JUCE-PAS1/Source/SynthesisEngine.h/cpp` (motor de síntesis)
- `app-juce/app-JUCE-PAS1/Source/VoiceManager.h/cpp` (gestión de voces)
- `app-juce/app-JUCE-PAS1/Source/ModalVoice.h/cpp` (voz individual)

## Información Adicional

### Contrato OSC (No Modificar)
- **Mensaje `/hit`:** `(int32 id, float x, float y, float energy, int32 surface)`
- **Mensaje `/state`:** `(float activity, float gesture, float presence)`
- **Puerto:** 9000
- **Host:** 127.0.0.1

### Lógica de Mapeo (Preservar)
- `mapOSCHitToEvent()`: Mapea parámetros OSC a síntesis (energy→amp, x→pan, y→damping/freq)
- `updateOSCState()`: Actualiza parámetros globales (activity→reverbMix, gesture→drive)

## Verificación de API

**Resultado de búsqueda en módulo OSC:**
- ❌ `ListenerWithOSCAddress` NO existe en `/ruta/a/JUCE/modules/juce_osc/juce_osc.h`
- ❌ No se encontraron clases `Listener` en el módulo
- ❌ No se encontró `oscMessageReceived` como método virtual
- ❌ No se encontró `addListener` con patrones de dirección

**Conclusión:** La API `ListenerWithOSCAddress` probablemente no existe en JUCE 8.0.12. Se requiere usar una API diferente.

## Preguntas para el LLM de Soporte

1. ¿Cuál es la API correcta de `OSCReceiver` en JUCE 8.0.12 para recibir mensajes OSC?
2. ¿Existe alguna clase listener para `OSCReceiver` en JUCE 8.0.12?
3. ¿Cómo se registran callbacks para mensajes OSC específicos (`/hit`, `/state`)?
4. ¿Es necesario procesar mensajes manualmente usando un timer o thread separado?
5. ¿Hay ejemplos de código JUCE 8.0.12 usando `OSCReceiver`?
6. ¿La API de OSC cambió entre versiones de JUCE? ¿Cuál es la forma correcta en 8.0.12?

## Comandos de Verificación

Para verificar la API disponible:
```bash
# Buscar definiciones en el módulo OSC (reemplazar /ruta/a/JUCE con la ruta real de instalación)
grep -r "ListenerWithOSCAddress" /ruta/a/JUCE/modules/juce_osc/
grep -r "oscMessageReceived" /ruta/a/JUCE/modules/juce_osc/
grep -r "addListener" /ruta/a/JUCE/modules/juce_osc/
```

## Estado del Proyecto

- **Fase 6:** ✅ Completada (Sintetizador básico funcionando)
- **Fase 7:** ⏳ En progreso (Receptor OSC - bloqueado por error de API)
- **Compilación:** ❌ Falla con errores de clase abstracta
- **Funcionalidad:** ⏸️ Pendiente hasta resolver errores de compilación

## Código Completo Relevante

### MainComponent.h (completo)
```cpp
#pragma once

#include <JuceHeader.h>
#include "SynthesisEngine.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::Timer,
                       public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    MainComponent();
    ~MainComponent() override;
    
    // Audio callbacks
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    // UI callbacks
    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;
    void timerCallback() override;

private:
    SynthesisEngine synthesisEngine;
    
    // OSC Receiver
    juce::OSCReceiver oscReceiver;
    juce::Label oscStatusLabel;
    juce::Label oscMessageCountLabel;
    std::atomic<int> oscMessageCountAccumulator{0};
    std::atomic<int> oscMessagesPerSecond{0};
    juce::int64 lastOscActivityTimestamp = 0;
    juce::int64 lastOscCountUpdateTime = 0;
    
    // Global state
    std::atomic<float> globalReverbMix{0.0f};
    std::atomic<float> globalDrive{0.0f};
    std::atomic<float> globalPresence{1.0f};
    
    // OSC callback (ERROR: firma incorrecta)
    void oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message) override;
    
    // OSC parameter mapping (funcionan correctamente)
    void mapOSCHitToEvent(const juce::OSCMessage& message);
    void updateOSCState(const juce::OSCMessage& message);
    
    // UI helpers
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     double min, double max, double defaultValue, double interval = 0.0);
};
```

### MainComponent.cpp (registro OSC)
```cpp
MainComponent::MainComponent()
{
    // ... inicialización de UI ...
    
    // Initialize OSC receiver
    if (oscReceiver.connect(9000))
    {
        // ERROR: addListener con patrones puede no existir
        oscReceiver.addListener(this, "/hit");
        oscReceiver.addListener(this, "/state");
        
        oscStatusLabel.setText("OSC: Connected (port 9000)", juce::dontSendNotification);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    // ...
}
```

### MainComponent.cpp (callback OSC)
```cpp
void MainComponent::oscMessageReceived(const juce::OSCAddressPattern& pattern, const juce::OSCMessage& message)
{
    lastOscActivityTimestamp = juce::Time::currentTimeMillis();
    oscMessageCountAccumulator++;
    
    juce::String address = pattern.toString();
    
    if (address == "/hit")
    {
        mapOSCHitToEvent(message);  // Esta función funciona correctamente
    }
    else if (address == "/state")
    {
        updateOSCState(message);  // Esta función funciona correctamente
    }
}
```

### MainComponent.cpp (mapeo - NO MODIFICAR)
```cpp
void MainComponent::mapOSCHitToEvent(const juce::OSCMessage& message)
{
    // Validación y extracción de parámetros
    if (message.size() != 5) return;
    
    float x = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());
    float y = juce::jlimit(0.0f, 1.0f, message[2].getFloat32());
    float energy = juce::jlimit(0.0f, 1.0f, message[3].getFloat32());
    
    // Mapeo según especificación "Coin Cascade"
    float amplitude = std::pow(energy, 1.5f);
    float brightness = 0.3f + (energy * 0.7f);
    float damping = 0.2f + ((1.0f - y) * 0.6f);
    float baseFreq = 200.0f + (y * 400.0f);
    float metalness = synthesisEngine.getMetalness();
    
    // Trigger voice (RT-safe)
    synthesisEngine.triggerVoiceFromOSC(baseFreq, amplitude, damping, brightness, metalness);
}
```

## Solución Alternativa Implementada

Se ha implementado un enfoque que **NO usa herencia de listener**:

### Cambios Aplicados:
1. **Eliminada herencia de listener:** `MainComponent` ya no hereda de `OSCReceiver::ListenerWithOSCAddress`
2. **Procesamiento manual:** Los mensajes OSC se procesan en `timerCallback()` usando `getNextPacket()`
3. **Método helper:** `handleOSCMessage()` procesa mensajes individuales

### Código de la Solución Alternativa:

**MainComponent.h:**
```cpp
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::Timer
{
    // ...
    void handleOSCMessage(const juce::OSCMessage& message);  // NO override
    // ...
};
```

**MainComponent.cpp (timerCallback):**
```cpp
void MainComponent::timerCallback()
{
    // ... otros indicadores ...
    
    // Process OSC messages manually
    juce::OSCBundle bundle;
    while (oscReceiver.getNextPacket(bundle))
    {
        for (const auto& element : bundle)
        {
            if (element.isMessage())
            {
                handleOSCMessage(element.getMessage());
            }
        }
    }
}
```

**Nota:** Si `getNextPacket()` tampoco existe, se requerirá otra solución.

## Otras Soluciones a Considerar

Si `getNextPacket()` no existe:

1. **Callbacks lambda:**
   - `OSCReceiver` puede aceptar funciones lambda como callbacks
   - Registrar lambdas para cada patrón de dirección

2. **Thread separado:**
   - Crear un thread dedicado que lea mensajes OSC
   - Usar cola thread-safe para pasar mensajes al thread principal

3. **Verificar documentación JUCE 8.0.12:**
   - Revisar ejemplos oficiales de JUCE 8.0.12
   - Verificar si hay cambios en la API de OSCReceiver

## Información del Sistema

- **OS:** macOS
- **Compilador:** Clang (Xcode)
- **JUCE Path:** `/ruta/a/JUCE/modules/` (típicamente `/Applications/JUCE/modules/` en macOS)
- **Proyecto:** Xcode (macOS) exportado desde Projucer

---

**Fecha del informe:** 2025-02-09  
**Versión JUCE:** 8.0.12  
**Estado:** Bloqueado - Requiere corrección de API de OSCReceiver  
**Prioridad:** Alta - Bloquea implementación de Fase 7
