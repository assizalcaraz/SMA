#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Configurar sliders y labels
    setupSlider(voicesSlider, voicesLabel, "Voices", 4.0, 12.0, 8.0, 1.0); // Rango 4-12 para estabilidad RT
    setupSlider(metalnessSlider, metalnessLabel, "Metalness", 0.0, 1.0, 0.5);
    setupSlider(brightnessSlider, brightnessLabel, "Brightness", 0.0, 1.0, 0.5);
    setupSlider(dampingSlider, dampingLabel, "Damping", 0.0, 1.0, 0.5);
    setupSlider(driveSlider, driveLabel, "Drive", 0.0, 1.0, 0.0);
    setupSlider(reverbMixSlider, reverbMixLabel, "Reverb Mix", 0.0, 1.0, 0.0);
    
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
    const int labelWidth = 120;
    const int sliderWidth = 200;
    
    // Primera columna de sliders
    auto leftColumn = area.removeFromLeft(350);
    
    voicesSlider.setBounds(leftColumn.removeFromTop(sliderHeight).reduced(margin));
    metalnessSlider.setBounds(leftColumn.removeFromTop(sliderHeight).reduced(margin));
    brightnessSlider.setBounds(leftColumn.removeFromTop(sliderHeight).reduced(margin));
    dampingSlider.setBounds(leftColumn.removeFromTop(sliderHeight).reduced(margin));
    
    // Segunda columna de sliders
    auto rightColumn = area.removeFromLeft(350);
    
    driveSlider.setBounds(rightColumn.removeFromTop(sliderHeight).reduced(margin));
    reverbMixSlider.setBounds(rightColumn.removeFromTop(sliderHeight).reduced(margin));
    
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
    else if (slider == &brightnessSlider)
    {
        synthesisEngine.setBrightness((float)brightnessSlider.getValue());
    }
    else if (slider == &dampingSlider)
    {
        synthesisEngine.setDamping((float)dampingSlider.getValue());
    }
    else if (slider == &driveSlider)
    {
        synthesisEngine.setDrive((float)driveSlider.getValue());
    }
    else if (slider == &reverbMixSlider)
    {
        synthesisEngine.setReverbMix((float)reverbMixSlider.getValue());
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
    label.attachToComponent(&slider, true);
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
    
    // Base frequency: 200 + (y * 400) Hz (200-600 Hz range)
    float baseFreq = 200.0f + (y * 400.0f);
    
    // Metalness: optional modulation based on surface
    // Use current global metalness, optionally modulate by surface
    float metalness = synthesisEngine.getMetalness();
    // Optional: slight variation by surface (0-3)
    if (surface >= 0 && surface <= 3)
    {
        // Small variation: ±0.1 based on surface
        float surfaceMod = (surface - 1.5f) * 0.066f; // -0.1 to +0.1
        metalness = juce::jlimit(0.0f, 1.0f, metalness + surfaceMod);
    }
    
    // Trigger voice through RT-safe queue
    synthesisEngine.triggerVoiceFromOSC(baseFreq, amplitude, damping, brightness, metalness);
    
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
    // activity → reverbMix: reverbMix = activity * 0.5
    globalReverbMix.store(activity * 0.5f);
    
    // gesture → drive: drive = gesture
    globalDrive.store(gesture);
    
    // presence → master level (optional: gently reduce if presence < 0.5)
    globalPresence.store(presence);
    
    // Apply global parameters to synthesis engine
    synthesisEngine.setReverbMix(globalReverbMix.load());
    synthesisEngine.setDrive(globalDrive.load());
    
    // Optional: Apply presence to master level (could be implemented in SynthesisEngine)
    // For now, we store it but don't apply it
}
