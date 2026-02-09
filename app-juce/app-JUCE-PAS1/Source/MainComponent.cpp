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
    g.drawText ("Modal Synthesizer - Coin Cascade", 
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
