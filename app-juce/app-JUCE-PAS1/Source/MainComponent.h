#pragma once

#include <JuceHeader.h>
#include "SynthesisEngine.h"
#include <random>

// Include OSC module directly (needed until project is regenerated from Projucer)
// After regenerating, this can be removed as it will be in JuceHeader.h
// Try multiple include paths to ensure it works
#if ! JUCE_MODULE_AVAILABLE_juce_osc
    #if __has_include(<juce_osc/juce_osc.h>)
        #include <juce_osc/juce_osc.h>
    #else
        #include "../../../../../../../../../Applications/JUCE/modules/juce_osc/juce_osc.h"
    #endif
#endif

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

private:
    //==============================================================================
    SynthesisEngine synthesisEngine;

    // UI Controls
    juce::Slider voicesSlider;
    juce::Label voicesLabel;
    
    juce::Slider metalnessSlider;
    juce::Label metalnessLabel;
    
    juce::ComboBox waveformComboBox;
    juce::Label waveformLabel;
    
    juce::Slider subOscMixSlider;
    juce::Label subOscMixLabel;
    
    juce::Slider pitchRangeSlider;
    juce::Label pitchRangeLabel;
    
    juce::Slider plateVolumeSlider;
    juce::Label plateVolumeLabel;
    
    juce::ToggleButton limiterToggle;
    juce::Label limiterLabel;
    
    juce::TextButton testTriggerButton;
    
    juce::Label outputLevelLabel;
    juce::Label activeVoicesLabel;
    
    // OSC Receiver
    juce::OSCReceiver oscReceiver;
    juce::Label oscStatusLabel;
    juce::Label oscMessageCountLabel;
    std::atomic<int> oscMessageCount{0};
    std::atomic<int> oscMessagesPerSecond{0};
    juce::int64 lastOscActivityTimestamp = 0;
    std::atomic<int> oscMessageCountAccumulator{0};
    juce::int64 lastOscCountUpdateTime = 0;
    
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
