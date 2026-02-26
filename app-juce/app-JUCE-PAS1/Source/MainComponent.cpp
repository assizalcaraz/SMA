#include "MainComponent.h"
#include <atomic>
#include <random>

//==============================================================================
MainComponent::MainComponent()
    : pitchRandomGen(std::random_device{}()), pitchRandomDist(0.0f, 1.0f)
{
    // Configurar sliders y labels
    setupSlider(voicesSlider, voicesLabel, "Voices", 4.0, 24.0, 8.0, 1.0); // M3: 4-24 para mayor polyfonía perceptual
    setupSlider(metalnessSlider, metalnessLabel, "Metalness", 0.0, 1.0, 0.5); // Dispersión de modos inarmónicos
    setupSlider(brightnessSlider, brightnessLabel, "Brightness", 0.0, 1.0, 0.5); // Tilt espectral (0=oscuro, 1=brillante)
    setupSlider(dampingSlider, dampingLabel, "Damping", 0.0, 1.0, 0.5); // Tiempo de decaimiento (0=corto, 1=largo)
    setupSlider(subOscMixSlider, subOscMixLabel, "SubOsc Mix", 0.0, 1.0, 0.0);
    setupSlider(pitchRangeSlider, pitchRangeLabel, "Pitch Range", 0.0, 1.0, 0.5); // Rango de variación de pitch random (0=sin variación, 1=máxima variación)
    
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
    
    // Clipper toggle (soft clip on output)
    limiterToggle.setButtonText("Clipper");
    limiterToggle.setToggleState(true, juce::dontSendNotification);
    limiterToggle.addListener(this);
    addAndMakeVisible(&limiterToggle);
    
    limiterLabel.setText("Clipper", juce::dontSendNotification);
    limiterLabel.attachToComponent(&limiterToggle, false);
    addAndMakeVisible(&limiterLabel);

    // M4 toggles (Density Comp, Center Bias) - visibility in resized when M4
    densityCompToggle.setButtonText("Density Comp");
    densityCompToggle.setToggleState(enableDensityCompensation, juce::dontSendNotification);
    densityCompToggle.addListener(this);
    addAndMakeVisible(&densityCompToggle);
    centerBiasToggle.setButtonText("Center Bias");
    centerBiasToggle.setToggleState(enableCenterBias, juce::dontSendNotification);
    centerBiasToggle.addListener(this);
    addAndMakeVisible(&centerBiasToggle);
    
    // M5: Preset selector
    presetComboBox.addItem("Dry Click", 1);
    presetComboBox.addItem("Bright Spray", 2);
    presetComboBox.addItem("Soft Foam", 3);
    presetComboBox.addItem("Heavy Border", 4);
    presetComboBox.addItem("Center Focus", 5);
    presetComboBox.addItem("Wide Dark", 6);
    presetComboBox.addItem("Crisp Short", 7);
    presetComboBox.addItem("Default (M4)", 8);
    presetComboBox.setSelectedId(8, juce::dontSendNotification); // Default (M4)
    presetComboBox.addListener(this);
    addAndMakeVisible(&presetComboBox);
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.attachToComponent(&presetComboBox, false);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&presetLabel);
    
    resetPresetButton.setButtonText("Reset");
    resetPresetButton.addListener(this);
    addAndMakeVisible(&resetPresetButton);
    
    demoModeToggle.setButtonText("Demo Mode");
    demoModeToggle.setToggleState(demoMode_, juce::dontSendNotification);
    demoModeToggle.addListener(this);
    addAndMakeVisible(&demoModeToggle);
    
    applyPreset(7); // Default (M4) initial state
    
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
    
    hitsCoverageLabel.setText("Hit Coverage: 100%", juce::dontSendNotification);
    hitsCoverageLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&hitsCoverageLabel);
    
    hitsStatsLabel.setText("Hits: 0/0 (0 discarded)", juce::dontSendNotification);
    hitsStatsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&hitsStatsLabel);
    
    m2FusionStatsLabel.setText("M2: raw 0 | fused 0/0 enq, 0 drop", juce::dontSendNotification);
    m2FusionStatsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&m2FusionStatsLabel);
    
    clipperHitCountLabel.setText("Clip: 0 blocks/s", juce::dontSendNotification);
    clipperHitCountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&clipperHitCountLabel);
    
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
    lastClipUpdateTime = juce::Time::currentTimeMillis();
    
    aggregatorTimer.owner = this;
    if (enableFusionAggregation)
        aggregatorTimer.startTimer(HitAggregator::WINDOW_MS);
    
    // M4: Sincronizar toggles al engine (A/B: cambiar enableM4Character etc. y recompilar o exponer en UI)
    synthesisEngine.setEnableM4Character(enableM4Character);
    synthesisEngine.setEnableDensityCompensation(enableDensityCompensation);

    // Tamaño por defecto: ancho reducido (columna derecha = Clipper + debug en vertical)
    setSize (540, 600);
        
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

    auto topArea = getLocalBounds().removeFromTop(50);
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    juce::String title;
    if (enableFusionAggregation && enableM4Character)
        title = "PAS-1 (M4 Character)";
    else if (enableFusionAggregation)
        title = "PAS-1 (M3 Perceptual)";
    else
        title = "PAS-1-SYNTH";
    g.drawText (title, topArea.removeFromTop(28).reduced(10), juce::Justification::centred, false);
    if (enableFusionAggregation)
    {
        g.setFont (12.0f);
        g.setColour (juce::Colours::lightgrey);
        g.drawText (enableM4Character ? "Perceptual Mode (M4)" : "Perceptual Mode", topArea.reduced(10), juce::Justification::centred, false);
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50); // Espacio para título
    
    const int sliderHeight = 56;
    const int margin = 8;
    const int labelWidth = 92;
    
    // Columna derecha: Clipper + Test + todos los textos de debug alineados verticalmente (ancho fijo)
    const int rightColumnWidth = 220;
    auto rightColumn = area.removeFromRight(rightColumnWidth);
    
    const int debugRowHeight = 22;
    // M5: Preset dropdown and Reset at top of right column
    auto presetRow = rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2);
    presetLabel.setBounds(presetRow.removeFromLeft(labelWidth));
    presetComboBox.setBounds(presetRow);
    resetPresetButton.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    demoModeToggle.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    rightColumn.removeFromTop(2); // small gap
    limiterToggle.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    bool m4Mode = enableFusionAggregation && enableM4Character;
    if (m4Mode)
    {
        densityCompToggle.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
        centerBiasToggle.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    }
    testTriggerButton.setBounds(rightColumn.removeFromTop(debugRowHeight + 4).reduced(margin, 2));
    rightColumn.removeFromTop(4); // separación
    outputLevelLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    activeVoicesLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    hitsCoverageLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    hitsStatsLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    m2FusionStatsLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    clipperHitCountLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    oscStatusLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    oscMessageCountLabel.setBounds(rightColumn.removeFromTop(debugRowHeight).reduced(margin, 2));
    
    // Columna izquierda: sliders (ocupa el resto del ancho)
    auto leftColumn = area.reduced(0, 0);
    auto voicesArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
    voicesLabel.setBounds(voicesArea.removeFromLeft(labelWidth));
    voicesSlider.setBounds(voicesArea);

    if (m4Mode)
    {
        // M4: solo Tone (brightness) y Decay (damping); Metalness oculto
        brightnessLabel.setText("Tone", juce::dontSendNotification);
        dampingLabel.setText("Decay", juce::dontSendNotification);
        auto toneArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        brightnessLabel.setBounds(toneArea.removeFromLeft(labelWidth));
        brightnessSlider.setBounds(toneArea);
        auto decayArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        dampingLabel.setBounds(decayArea.removeFromLeft(labelWidth));
        dampingSlider.setBounds(decayArea);
        metalnessLabel.setVisible(false);
        metalnessSlider.setVisible(false);
        metalnessLabel.setBounds(0, 0, 0, 0);
        metalnessSlider.setBounds(0, 0, 0, 0);
        densityCompToggle.setVisible(true);
        centerBiasToggle.setVisible(true);
    }
    else
    {
        brightnessLabel.setText("Brightness", juce::dontSendNotification);
        dampingLabel.setText("Damping", juce::dontSendNotification);
        auto metalnessArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        metalnessLabel.setBounds(metalnessArea.removeFromLeft(labelWidth));
        metalnessSlider.setBounds(metalnessArea);
        metalnessLabel.setVisible(true);
        metalnessSlider.setVisible(true);
        auto brightnessArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        brightnessLabel.setBounds(brightnessArea.removeFromLeft(labelWidth));
        brightnessSlider.setBounds(brightnessArea);
        auto dampingArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        dampingLabel.setBounds(dampingArea.removeFromLeft(labelWidth));
        dampingSlider.setBounds(dampingArea);
        densityCompToggle.setVisible(false);
        centerBiasToggle.setVisible(false);
        densityCompToggle.setBounds(0, 0, 0, 0);
        centerBiasToggle.setBounds(0, 0, 0, 0);
    }
    
    // M3: Waveform, SubOsc Mix, Pitch Range solo visibles sin fusión (evitar controles "muertos")
    bool showLegacyControls = !enableFusionAggregation;
    if (showLegacyControls)
    {
        auto waveformArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        waveformLabel.setBounds(waveformArea.removeFromLeft(labelWidth));
        waveformComboBox.setBounds(waveformArea);
        auto subOscArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        subOscMixLabel.setBounds(subOscArea.removeFromLeft(labelWidth));
        subOscMixSlider.setBounds(subOscArea);
        auto pitchRangeArea = leftColumn.removeFromTop(sliderHeight).reduced(margin);
        pitchRangeLabel.setBounds(pitchRangeArea.removeFromLeft(labelWidth));
        pitchRangeSlider.setBounds(pitchRangeArea);
    }
    waveformLabel.setVisible(showLegacyControls);
    waveformComboBox.setVisible(showLegacyControls);
    subOscMixLabel.setVisible(showLegacyControls);
    subOscMixSlider.setVisible(showLegacyControls);
    pitchRangeLabel.setVisible(showLegacyControls);
    pitchRangeSlider.setVisible(showLegacyControls);
    if (!showLegacyControls)
    {
        juce::Rectangle<int> empty(0, 0, 0, 0);
        waveformLabel.setBounds(empty);
        waveformComboBox.setBounds(empty);
        subOscMixLabel.setBounds(empty);
        subOscMixSlider.setBounds(empty);
        pitchRangeLabel.setBounds(empty);
        pitchRangeSlider.setBounds(empty);
    }
    
    // M5: Demo mode — hide advanced counters and optional toggles
    const bool showAdvanced = !demoMode_;
    m2FusionStatsLabel.setVisible(showAdvanced);
    clipperHitCountLabel.setVisible(showAdvanced);
    hitsStatsLabel.setVisible(showAdvanced);
    densityCompToggle.setVisible(m4Mode && showAdvanced);
    centerBiasToggle.setVisible(m4Mode && showAdvanced);
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
    else if (slider == &subOscMixSlider)
    {
        synthesisEngine.setSubOscMix((float)subOscMixSlider.getValue());
    }
    else if (slider == &pitchRangeSlider)
    {
        synthesisEngine.setPitchRange((float)pitchRangeSlider.getValue());
    }
}

//==============================================================================
void MainComponent::buttonClicked (juce::Button* button)
{
    if (button == &testTriggerButton)
    {
        synthesisEngine.triggerTestVoice();
    }
    else if (button == &resetPresetButton)
    {
        presetComboBox.setSelectedId(8, juce::dontSendNotification); // Default (M4)
        applyPreset(7);
    }
    else if (button == &demoModeToggle)
    {
        demoMode_ = demoModeToggle.getToggleState();
        resized();
    }
    else if (button == &limiterToggle)
    {
        synthesisEngine.setLimiterEnabled(limiterToggle.getToggleState());
    }
    else if (button == &densityCompToggle)
    {
        enableDensityCompensation = densityCompToggle.getToggleState();
        synthesisEngine.setEnableDensityCompensation(enableDensityCompensation);
    }
    else if (button == &centerBiasToggle)
    {
        enableCenterBias = centerBiasToggle.getToggleState();
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
    else if (comboBox == &presetComboBox)
    {
        int id = presetComboBox.getSelectedId();
        if (id >= 1 && id <= 8)
            applyPreset(id - 1);
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
    
    // Actualizar estadísticas de hits
    int hitsReceived = synthesisEngine.getHitsReceived();
    int hitsTriggered = synthesisEngine.getHitsTriggered();
    int hitsDiscarded = synthesisEngine.getHitsDiscarded();
    float coverageRatio = synthesisEngine.getHitCoverageRatio();
    
    hitsStatsLabel.setText("Hits: " + juce::String(hitsTriggered) + "/" + 
                          juce::String(hitsReceived) + " (" + 
                          juce::String(hitsDiscarded) + " discarded)", 
                          juce::dontSendNotification);
    
    // Mostrar ratio de cobertura con color según umbral
    float coveragePercent = coverageRatio * 100.0f;
    juce::String coverageText = "Hit Coverage: " + juce::String(coveragePercent, 1) + "%";
    hitsCoverageLabel.setText(coverageText, juce::dontSendNotification);
    
    // Cambiar color según umbral (warning si < 90%)
    if (coverageRatio < 0.9f && hitsReceived > 10) // Solo mostrar warning si hay suficientes hits
    {
        hitsCoverageLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
    else if (coverageRatio < 0.7f && hitsReceived > 10)
    {
        hitsCoverageLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
    else
    {
        hitsCoverageLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    }
    
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
    
    // M2 fusion metrics: raw, fused produced/enqueued/dropped, coverage, queue loss
    int rawHits = synthesisEngine.getHitsReceived();
    int produced = fusedProduced.load(std::memory_order_relaxed);
    int enqueued = synthesisEngine.getFusedHitsEnqueued();
    int dropped = synthesisEngine.getFusedHitsDiscardedQueue();
    float cov = (rawHits > 0 && produced > 0) ? (float)enqueued / (float)rawHits : 0.0f;
    float qloss = (produced > 0) ? (float)dropped / (float)produced : 0.0f;
    m2FusionStatsLabel.setText("M2: raw " + juce::String(rawHits) + " | fused " + juce::String(produced) +
                               "/" + juce::String(enqueued) + " enq, " + juce::String(dropped) + " drop | cov " +
                               juce::String(cov * 100.0f, 1) + "% qloss " + juce::String(qloss * 100.0f, 1) + "%",
                               juce::dontSendNotification);
    
    // M3: clip rate = blocks/s (interpretable); recompute every 500 ms
    juce::int64 now = juce::Time::currentTimeMillis();
    if (now - lastClipUpdateTime >= 500)
    {
        int cur = synthesisEngine.getBlocksClippedCount();
        int deltaBlocks = cur - lastBlocksClippedCount;
        double deltaSec = (now - lastClipUpdateTime) * 0.001;
        if (deltaSec <= 0) deltaSec = 0.5;
        lastBlocksClippedCount = cur;
        lastClipUpdateTime = now;
        double blocksPerSec = deltaBlocks / deltaSec;
        clipperHitCountLabel.setText("Clip: " + juce::String(blocksPerSec, 1) + " blocks/s", juce::dontSendNotification);
    }
    
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
    else if (address == "/plate" && enablePlateSynth)
    {
        mapOSCPlateToEvent(message);
    }
    // Silently ignore unknown addresses (no crash, no log spam)
}

//==============================================================================
void AggregatorTimer::timerCallback()
{
    if (owner)
        owner->flushAggregatorWindow();
}

//==============================================================================
void MainComponent::applyPreset(int presetIndex)
{
    struct Preset { int voices; float tone; float decay; bool densityComp; bool centerBias; };
    static const Preset presets[] = {
        { 8,  0.25f, 0.25f, true,  true  },  // 0 Dry Click
        { 12, 0.85f, 0.45f, true,  true  },  // 1 Bright Spray
        { 10, 0.45f, 0.75f, true,  true  },  // 2 Soft Foam
        { 16, 0.35f, 0.85f, true,  true  },  // 3 Heavy Border
        { 8,  0.55f, 0.55f, true,  true  },  // 4 Center Focus
        { 14, 0.20f, 0.65f, false, false },  // 5 Wide Dark
        { 10, 0.70f, 0.30f, true,  false },  // 6 Crisp Short
        { 8,  0.50f, 0.50f, true,  true  },  // 7 Default (M4)
    };
    if (presetIndex < 0 || presetIndex >= 8)
        return;
    const Preset& p = presets[presetIndex];
    voicesSlider.setValue(p.voices, juce::dontSendNotification);
    brightnessSlider.setValue(p.tone, juce::dontSendNotification);
    dampingSlider.setValue(p.decay, juce::dontSendNotification);
    synthesisEngine.setMaxVoices(p.voices);
    synthesisEngine.setBrightness(p.tone);
    synthesisEngine.setDamping(p.decay);
    enableDensityCompensation = p.densityComp;
    enableCenterBias = p.centerBias;
    densityCompToggle.setToggleState(p.densityComp, juce::dontSendNotification);
    centerBiasToggle.setToggleState(p.centerBias, juce::dontSendNotification);
    synthesisEngine.setEnableDensityCompensation(p.densityComp);
    hitAggregator.setEnableCenterBias(p.centerBias);
}

//==============================================================================
void MainComponent::flushAggregatorWindow()
{
    hitAggregator.setEnableCenterBias(enableCenterBias);
    FusedHitSnapshot snaps[HitAggregator::NUM_QUADRANTS];
    int n = hitAggregator.closeWindow(snaps, HitAggregator::NUM_QUADRANTS);
    fusedProduced.fetch_add(n, std::memory_order_relaxed);
    float metalness = synthesisEngine.getMetalness();
    float subOscMix = synthesisEngine.getSubOscMix();
    for (int i = 0; i < n; i++)
    {
        snaps[i].metalness = metalness;
        snaps[i].subOscMix = subOscMix;
        synthesisEngine.enqueueFusedSnapshot(snaps[i]);
    }
}

//==============================================================================
void MainComponent::mapOSCHitToEvent(const juce::OSCMessage& message)
{
    // Validate message format: /hit id(int32) x(float) y(float) energy(float) surface(int32)
    if (message.size() != 5)
    {
        return;
    }
    
    if (!message[0].isInt32() || !message[1].isFloat32() || !message[2].isFloat32() || 
        !message[3].isFloat32() || !message[4].isInt32())
    {
        return;
    }
    
    int id = message[0].getInt32();
    float x = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());
    float y = juce::jlimit(0.0f, 1.0f, message[2].getFloat32());
    float energy = juce::jlimit(0.0f, 1.0f, message[3].getFloat32());
    int surface = message[4].getInt32();
    
    if (enableFusionAggregation)
    {
        synthesisEngine.incrementHitsReceived();
        hitAggregator.addHit(x, y, energy, surface);
        (void)id;
        return;
    }
    
    // Modo sin agregación: mapeo directo como antes
    float amplitude = std::pow(energy, 1.5f);
    float brightness = 0.3f + (energy * 0.7f);
    float damping = 0.2f + ((1.0f - y) * 0.6f);
    float pitchRange = synthesisEngine.getPitchRange();
    float centerFreq = 300.0f;
    float maxVariation = 200.0f;
    float randomValue = pitchRandomDist(pitchRandomGen);
    float variation = (randomValue * 2.0f - 1.0f) * pitchRange * maxVariation;
    float baseFreq = juce::jlimit(100.0f, 800.0f, centerFreq + variation);
    float metalness = synthesisEngine.getMetalness();
    
    ModalVoice::ExcitationWaveform waveform;
    if (energy > 0.7f)
        waveform = ModalVoice::ExcitationWaveform::Square;
    else if (energy > 0.4f)
        waveform = ModalVoice::ExcitationWaveform::Saw;
    else if (energy > 0.2f)
        waveform = ModalVoice::ExcitationWaveform::Noise;
    else
        waveform = ModalVoice::ExcitationWaveform::Sine;
    
    float subOscMix = synthesisEngine.getSubOscMix();
    synthesisEngine.triggerVoiceFromOSC(baseFreq, amplitude, damping, brightness, metalness, waveform, subOscMix);
    (void)id;
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

