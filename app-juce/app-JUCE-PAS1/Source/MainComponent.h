#pragma once

#include <JuceHeader.h>
#include "SynthesisEngine.h"
#include "HitAggregator.h"
#include <random>

class MainComponent;

struct AggregatorTimer : juce::Timer
{
    MainComponent* owner = nullptr;
    void timerCallback() override;
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::ComboBox::Listener,
                       public juce::Timer,
                       private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;
    void timerCallback() override;

    /** Llamado por AggregatorTimer cada 20 ms para cerrar ventana y encolar FusedHitSnapshot. */
    void flushAggregatorWindow();

private:
    //==============================================================================
    SynthesisEngine synthesisEngine;

    // UI Controls
    juce::Slider voicesSlider;
    juce::Label voicesLabel;
    
    juce::Slider metalnessSlider;
    juce::Label metalnessLabel;
    
    juce::Slider brightnessSlider;
    juce::Label brightnessLabel;
    
    juce::Slider dampingSlider;
    juce::Label dampingLabel;
    
    juce::ComboBox waveformComboBox;
    juce::Label waveformLabel;
    
    juce::Slider subOscMixSlider;
    juce::Label subOscMixLabel;
    
    juce::Slider pitchRangeSlider;
    juce::Label pitchRangeLabel;
    
    juce::ToggleButton limiterToggle;
    juce::Label limiterLabel;
    
    juce::TextButton testTriggerButton;
    
    juce::Label outputLevelLabel;
    juce::Label activeVoicesLabel;
    juce::Label hitsCoverageLabel;
    juce::Label hitsStatsLabel;
    
    // OSC Receiver
    juce::OSCReceiver oscReceiver;
    juce::Label oscStatusLabel;
    juce::Label oscMessageCountLabel;
    std::atomic<int> oscMessageCount{0};
    std::atomic<int> oscMessagesPerSecond{0};
    juce::int64 lastOscActivityTimestamp = 0;
    std::atomic<int> oscMessageCountAccumulator{0};
    juce::int64 lastOscCountUpdateTime = 0;
    
    /** Si false (default), PAS ignora /plate y PlateSynth no recibe triggers. */
    bool enablePlateSynth = false;
    
    /** Si true (default v1), los /hit se agregan por cuadrante/ventana 20 ms y se encolan como FusedHitSnapshot. */
    bool enableAggregation = true;
    
    HitAggregator hitAggregator;
    AggregatorTimer aggregatorTimer;
    
    // Global state from /state messages (non-RT thread safe)
    std::atomic<float> globalPresence{1.0f};
    
    // Random number generator for pitch (thread-safe: solo se usa en UI thread)
    std::mt19937 pitchRandomGen;
    std::uniform_real_distribution<float> pitchRandomDist;
    
    //==============================================================================
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     double min, double max, double defaultValue, double interval = 0.0);
    
    // OSC callback (override from OSCReceiver::Listener)
    void oscMessageReceived(const juce::OSCMessage& message) override;
    
    // OSC parameter mapping
    void mapOSCHitToEvent(const juce::OSCMessage& message);
    void updateOSCState(const juce::OSCMessage& message);
    void mapOSCPlateToEvent(const juce::OSCMessage& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
