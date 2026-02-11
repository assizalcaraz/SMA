#include "MainComponent.h"
#include <random>

//==============================================================================
MainComponent::MainComponent()
    : pitchRandomGen(std::random_device{}()), pitchRandomDist(0.0f, 1.0f)
{
    // Configurar sliders y labels (solo controles que funcionan)
    setupSlider(voicesSlider, voicesLabel, "Voices", 4.0, 12.0, 8.0, 1.0); // Rango 4-12 para estabilidad RT
    setupSlider(metalnessSlider, metalnessLabel, "Pitch", 0.0, 1.0, 0.5);
    setupSlider(subOscMixSlider, subOscMixLabel, "SubOsc Mix", 0.0, 1.0, 0.0);
    setupSlider(pitchRangeSlider, pitchRangeLabel, "Pitch Range", 0.0, 1.0, 0.5); // Rango de variación de pitch random (0=sin variación, 1=máxima variación)
    setupSlider(plateVolumeSlider, plateVolumeLabel, "Plate Volume", 0.0, 1.0, 1.0);
    
    // Waveform selector
    waveformComboBox.addItem("Noise", 1);
    waveformComboBox.addItem("Sine", 2);
    waveformComboBox.addItem("Square", 3);
    waveformComboBox.addItem("Saw", 4);
    waveformComboBox.addItem("Triangle", 5);
    waveformComboBox.addItem("Click", 6);
    waveformComboBox.addItem("Pulse", 7);
    waveformComboBox.setSelectedId(1, juce::dontSendNotification); // Default: Noise
    waveformComboBox.addListener(this);
    addAndMakeVisible(&waveformComboBox);
    
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent(&waveformComboBox, false);
    waveformLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&waveformLabel);
    
    // Limiter toggle
    limiterToggle.setButtonText("Limiter");
    limiterToggle.setToggleState(true, juce::dontSendNotification);
    limiterToggle.addListener(this);
    addAndMakeVisible(&limiterToggle);
    
    limiterLabel.setText("Limiter", juce::dontSendNotification);
    limiterLabel.attachToComponent(&limiterToggle, false);
    addAndMakeVisible(&limiterLabel);
    
    // Test trigger button
    testTriggerButton.setButtonText("Test Trigger");
    testTriggerButton.addListener(this);
    addAndMakeVisible(&testTriggerButton);
    
    // Indicadores
    outputLevelLabel.setText("Output: 0.00 dB", juce::dontSendNotification);
    outputLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&outputLevelLabel);
    
    activeVoicesLabel.setText("Active Voices: 0", juce::dontSendNotification);
    activeVoicesLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&activeVoicesLabel);
    
    // OSC Receiver setup
    oscStatusLabel.setText("OSC: Disconnected", juce::dontSendNotification);
    oscStatusLabel.setJustificationType(juce::Justification::centred);
    oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    addAndMakeVisible(&oscStatusLabel);
    
    oscMessageCountLabel.setText("OSC Messages: 0/s", juce::dontSendNotification);
    oscMessageCountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&oscMessageCountLabel);
    
    // Initialize OSC receiver
    // Use OSCReceiver::Listener<MessageLoopCallback> pattern for JUCE 8.0.12
    if (oscReceiver.connect(9000))
    {
        oscReceiver.addListener(this);  // Register listener without address filter
        oscStatusLabel.setText("OSC: Connected (port 9000)", juce::dontSendNotification);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    else
    {
        oscStatusLabel.setText("OSC: Connection failed", juce::dontSendNotification);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
    
    lastOscActivityTimestamp = juce::Time::currentTimeMillis();
    lastOscCountUpdateTime = juce::Time::currentTimeMillis();
    
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
        
    // Iniciar timer para actualizar indicadores
    startTimer(50); // Actualizar cada 50ms

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // Remove OSC listener before disconnecting
    oscReceiver.removeListener(this);
    oscReceiver.disconnect();
    
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // Preparar el motor de síntesis
    synthesisEngine.prepare(sampleRate);
    
    // Log para debugging: verificar buffer size
    // Si el buffer es muy pequeño (< 256), puede causar sobrecarga del audio thread
    // Recomendado: buffer size de 256-512 samples para estabilidad RT
    DBG("Audio prepared: sampleRate=" << sampleRate 
        << ", bufferSize=" << samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Limpiar el buffer primero
    bufferToFill.clearActiveBufferRegion();
    
    // Renderizar el motor de síntesis
    if (bufferToFill.buffer != nullptr)
    {
        synthesisEngine.renderNextBlock(*bufferToFill.buffer, 
                                         bufferToFill.startSample, 
                                         bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // Resetear el motor de síntesis
    synthesisEngine.reset();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Título
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("PAS-1-SYNTH", 
                getLocalBounds().removeFromTop(40).reduced(10),
                juce::Justification::centred, false);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50); // Espacio para título
    
    const int sliderHeight = 60;
    const int margin = 10;
    const int labelWidth = 100;
    const int sliderWidth = 200;
    
    // Primera columna de sliders
    auto leftColumn = area.removeFromLeft(350);
    
    // Layout mejorado: label a la izquierda, slider a la derecha
    auto voicesArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    voicesLabel.setBounds(voicesArea.removeFromLeft(labelWidth));
    voicesSlider.setBounds(voicesArea);
    
    auto metalnessArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    metalnessLabel.setBounds(metalnessArea.removeFromLeft(labelWidth));
    metalnessSlider.setBounds(metalnessArea);
    
    // Waveform combo box
    auto waveformArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    waveformLabel.setBounds(waveformArea.removeFromLeft(labelWidth));
    waveformComboBox.setBounds(waveformArea);
    
    // SubOsc Mix slider
    auto subOscArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    subOscMixLabel.setBounds(subOscArea.removeFromLeft(labelWidth));
    subOscMixSlider.setBounds(subOscArea);
    
    // Pitch Range slider
    auto pitchRangeArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    pitchRangeLabel.setBounds(pitchRangeArea.removeFromLeft(labelWidth));
    pitchRangeSlider.setBounds(pitchRangeArea);
    
    // Plate Volume slider
    auto plateVolumeArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    plateVolumeLabel.setBounds(plateVolumeArea.removeFromLeft(labelWidth));
    plateVolumeSlider.setBounds(plateVolumeArea);
    
    // Segunda columna de controles
    auto rightColumn = area.removeFromRight(350);
    
    // Limiter toggle
    limiterToggle.setBounds(rightColumn.removeFromTop(30).reduced(margin));
    
    // Botón de test trigger
    testTriggerButton.setBounds(rightColumn.removeFromTop(40).reduced(margin));
    
    // Indicadores en la parte inferior
    auto bottomArea = area;
    outputLevelLabel.setBounds(bottomArea.removeFromTop(30).reduced(margin));
    activeVoicesLabel.setBounds(bottomArea.removeFromTop(30).reduced(margin));
    oscStatusLabel.setBounds(bottomArea.removeFromTop(30).reduced(margin));
    oscMessageCountLabel.setBounds(bottomArea.removeFromTop(30).reduced(margin));
}

//==============================================================================
void MainComponent::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &voicesSlider)
    {
        synthesisEngine.setMaxVoices((int)voicesSlider.getValue());
    }
    else if (slider == &metalnessSlider)
    {
        synthesisEngine.setMetalness((float)metalnessSlider.getValue());
    }
    else if (slider == &subOscMixSlider)
    {
        synthesisEngine.setSubOscMix((float)subOscMixSlider.getValue());
    }
    else if (slider == &pitchRangeSlider)
    {
        synthesisEngine.setPitchRange((float)pitchRangeSlider.getValue());
    }
    else if (slider == &plateVolumeSlider)
    {
        // Mapeo exponencial muy suave: slider 0.01 → volumen 0.1 (10% en lugar de 50%)
        // slider 0.0 → volumen 0.0, slider 1.0 → volumen 1.0
        // Mapeo ajustado para balancear con audio de partículas (plate suena más fuerte)
        float sliderValue = (float)plateVolumeSlider.getValue();
        float volume;
        
        if (sliderValue <= 0.0f)
        {
            volume = 0.0f;
        }
        else if (sliderValue <= 0.01f)
        {
            // Mapeo exponencial muy suave de 0.0-0.01 a 0.0-0.05 (5% máximo)
            // Usar exponente alto para curva muy gradual al inicio
            float normalized = sliderValue / 0.01f; // 0.0 a 1.0
            // Curva muy suave: exponente 0.8 hace que crezca muy gradualmente
            // Ejemplo: sliderValue=0.003 → normalized=0.3 → pow(0.3, 0.8)≈0.38 → volume≈0.019 (1.9%)
            float curve = std::pow(normalized, 0.8f); // Curva muy suave
            volume = curve * 0.05f; // Máximo 5% en sliderValue = 0.01 (reducido para mejor balance)
        }
        else
        {
            // Mapeo exponencial de 0.01-1.0 a 0.05-1.0 (más gradual que lineal)
            float normalized = (sliderValue - 0.01f) / 0.99f; // 0.0 a 1.0
            // Usar curva exponencial suave para el resto del rango también
            float curve = std::pow(normalized, 0.7f); // Curva suave
            volume = 0.05f + curve * 0.95f; // De 0.05 a 1.0
        }
        
        synthesisEngine.setPlateVolume(volume);
    }
}

//==============================================================================
void MainComponent::buttonClicked (juce::Button* button)
{
    if (button == &testTriggerButton)
    {
        synthesisEngine.triggerTestVoice();
    }
    else if (button == &limiterToggle)
    {
        synthesisEngine.setLimiterEnabled(limiterToggle.getToggleState());
    }
}

//==============================================================================
void MainComponent::comboBoxChanged (juce::ComboBox* comboBox)
{
    if (comboBox == &waveformComboBox)
    {
        int selectedId = waveformComboBox.getSelectedId();
        ModalVoice::ExcitationWaveform waveform = static_cast<ModalVoice::ExcitationWaveform>(selectedId - 1);
        synthesisEngine.setWaveform(waveform);
    }
}

//==============================================================================
void MainComponent::timerCallback()
{
    // Actualizar indicadores
    float outputLevel = synthesisEngine.getOutputLevel();
    float outputLevelDb = outputLevel > 0.0f ? 
        20.0f * std::log10(std::sqrt(outputLevel)) : -100.0f;
    
    outputLevelLabel.setText("Output: " + juce::String(outputLevelDb, 2) + " dB", 
                            juce::dontSendNotification);
    
    int activeVoices = synthesisEngine.getActiveVoiceCount();
    activeVoicesLabel.setText("Active Voices: " + juce::String(activeVoices), 
                             juce::dontSendNotification);
    
    // Update OSC indicators (messages are now processed via listener callback)
    juce::int64 currentTime = juce::Time::currentTimeMillis();
    if (currentTime - lastOscCountUpdateTime >= 1000) // Update every second
    {
        oscMessagesPerSecond.store(oscMessageCountAccumulator.load());
        oscMessageCountAccumulator.store(0);
        lastOscCountUpdateTime = currentTime;
    }
    
    int messagesPerSec = oscMessagesPerSecond.load();
    oscMessageCountLabel.setText("OSC Messages: " + juce::String(messagesPerSec) + "/s", 
                                juce::dontSendNotification);
    
    // Update OSC status color based on recent activity
    juce::int64 timeSinceLastMessage = currentTime - lastOscActivityTimestamp;
    if (timeSinceLastMessage < 2000) // Active if message in last 2 seconds
    {
        if (oscStatusLabel.getText() != "OSC: Connected (port 9000)")
        {
            oscStatusLabel.setText("OSC: Connected (port 9000)", juce::dontSendNotification);
            oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        }
    }
}

//==============================================================================
void MainComponent::setupSlider(juce::Slider& slider, juce::Label& label, 
                                const juce::String& name,
                                double min, double max, double defaultValue, double interval)
{
    slider.setRange(min, max, interval > 0.0 ? interval : 0.01);
    slider.setValue(defaultValue, juce::dontSendNotification);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    slider.addListener(this);
    addAndMakeVisible(&slider);
    
    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, false); // Cambiar a false para evitar superposición
    label.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&label);
}

//==============================================================================
void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    // Update activity timestamp and counters (atomic, thread-safe)
    lastOscActivityTimestamp = juce::Time::currentTimeMillis();
    oscMessageCountAccumulator++;
    
    // Get address from message using correct JUCE 8.0.12 API
    juce::String address = message.getAddressPattern().toString();
    
    // Route to appropriate handler based on address pattern
    if (address == "/hit")
    {
        mapOSCHitToEvent(message);
    }
    else if (address == "/state")
    {
        updateOSCState(message);
    }
    else if (address == "/plate")
    {
        mapOSCPlateToEvent(message);
    }
    // Silently ignore unknown addresses (no crash, no log spam)
}

//==============================================================================
void MainComponent::mapOSCHitToEvent(const juce::OSCMessage& message)
{
    // Validate message format: /hit id(int32) x(float) y(float) energy(float) surface(int32)
    if (message.size() != 5)
    {
        // Malformed message - silently discard
        return;
    }
    
    // Extract and validate arguments
    if (!message[0].isInt32() || !message[1].isFloat32() || !message[2].isFloat32() || 
        !message[3].isFloat32() || !message[4].isInt32())
    {
        // Wrong argument types - silently discard
        return;
    }
    
    // Extract values and clamp to valid ranges
    int id = message[0].getInt32();
    float x = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());
    float y = juce::jlimit(0.0f, 1.0f, message[2].getFloat32());
    float energy = juce::jlimit(0.0f, 1.0f, message[3].getFloat32());
    int surface = message[4].getInt32();
    
    // Map parameters according to "Coin Cascade" design
    
    // Amplitude: amp = energy^1.5 (micro-collisions → very low but audible)
    float amplitude = std::pow(energy, 1.5f);
    
    // Brightness: lerp(0.3, 1.0, energy)
    float brightness = 0.3f + (energy * 0.7f);
    
    // Pan: pan = (x * 2.0) - 1.0 (-1 = left, +1 = right)
    // Note: Pan will be applied in VoiceManager if stereo support is added
    // For now, we'll store it but not use it (mono output)
    float pan = (x * 2.0f) - 1.0f;
    (void)pan; // Suppress unused variable warning
    
    // Damping: lerp(0.2, 0.8, 1.0 - y)
    // Upper screen → drier (lower damping), Lower screen → longer decay (higher damping)
    float damping = 0.2f + ((1.0f - y) * 0.6f);
    
    // Base frequency: Random dentro de rango controlado por slider
    // Frecuencia base: 300 Hz (centro del rango metálico)
    // Rango de variación controlado por pitchRange slider (0.0 = sin variación, 1.0 = ±200 Hz)
    float pitchRange = synthesisEngine.getPitchRange();
    float centerFreq = 300.0f; // Frecuencia central (Hz)
    float maxVariation = 200.0f; // Variación máxima (±200 Hz)
    
    // Generar random pitch: centerFreq ± (pitchRange * maxVariation * random)
    float randomValue = pitchRandomDist(pitchRandomGen); // 0.0 a 1.0
    float variation = (randomValue * 2.0f - 1.0f) * pitchRange * maxVariation; // -maxVariation a +maxVariation
    float baseFreq = centerFreq + variation;
    baseFreq = juce::jlimit(100.0f, 800.0f, baseFreq); // Clamp a rango seguro
    
    // Metalness: usar valor global (unificado para todos los eventos)
    // No modificar por surface para que bordes y colisiones suenen igual
    float metalness = synthesisEngine.getMetalness();
    
    // Mapeo adaptativo de energy → waveform (Fase 4: Excitación Adaptativa)
    // Alta energía → formas agresivas, baja energía → formas suaves
    ModalVoice::ExcitationWaveform waveform;
    if (energy > 0.7f)
        waveform = ModalVoice::ExcitationWaveform::Square; // Alta energía → agresivo
    else if (energy > 0.4f)
        waveform = ModalVoice::ExcitationWaveform::Saw; // Media-alta → brillante
    else if (energy > 0.2f)
        waveform = ModalVoice::ExcitationWaveform::Noise; // Media-baja → ruido
    else
        waveform = ModalVoice::ExcitationWaveform::Sine; // Baja energía → suave
    
    // Obtener subOscMix global del engine
    float subOscMix = synthesisEngine.getSubOscMix();
    
    // Trigger voice through RT-safe queue
    synthesisEngine.triggerVoiceFromOSC(baseFreq, amplitude, damping, brightness, metalness, waveform, subOscMix);
    
    (void)id; // Suppress unused variable warning (id is for future use)
}

//==============================================================================
void MainComponent::updateOSCState(const juce::OSCMessage& message)
{
    // Validate message format: /state activity(float) gesture(float) presence(float)
    if (message.size() != 3)
    {
        // Malformed message - silently discard
        return;
    }
    
    // Extract and validate arguments
    if (!message[0].isFloat32() || !message[1].isFloat32() || !message[2].isFloat32())
    {
        // Wrong argument types - silently discard
        return;
    }
    
    // Extract values and clamp to valid ranges
    float activity = juce::jlimit(0.0f, 1.0f, message[0].getFloat32());
    float gesture = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());
    float presence = juce::jlimit(0.0f, 1.0f, message[2].getFloat32());
    
    // Map global parameters (non-RT thread safe, but atomic)
    // presence → master level (optional: gently reduce if presence < 0.5)
    globalPresence.store(presence);
    
    // Optional: Apply presence to master level (could be implemented in SynthesisEngine)
    // For now, we store it but don't apply it
}

//==============================================================================
void MainComponent::mapOSCPlateToEvent(const juce::OSCMessage& message)
{
    // Validate message format: /plate freq(float) amp(float) mode(int32)
    if (message.size() != 3)
    {
        // Malformed message - silently discard
        return;
    }
    
    // Extract and validate arguments
    if (!message[0].isFloat32() || !message[1].isFloat32() || !message[2].isInt32())
    {
        // Wrong argument types - silently discard
        return;
    }
    
    // Extract values and clamp to valid ranges
    float freq = juce::jlimit(20.0f, 2000.0f, message[0].getFloat32());
    float amp = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());
    int mode = juce::jlimit(0, 7, message[2].getInt32());
    
    // Trigger plate synth through RT-safe method (to be implemented in SynthesisEngine)
    synthesisEngine.triggerPlateFromOSC(freq, amp, mode);
}
