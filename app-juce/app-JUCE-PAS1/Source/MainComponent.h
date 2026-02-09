#pragma once

#include <JuceHeader.h>
#include "SynthesisEngine.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::Slider::Listener,
                       public juce::Button::Listener,
                       public juce::Timer
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
    void timerCallback() override;

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
    
    juce::Slider driveSlider;
    juce::Label driveLabel;
    
    juce::Slider reverbMixSlider;
    juce::Label reverbMixLabel;
    
    juce::ToggleButton limiterToggle;
    juce::Label limiterLabel;
    
    juce::TextButton testTriggerButton;
    
    juce::Label outputLevelLabel;
    juce::Label activeVoicesLabel;
    
    //==============================================================================
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     double min, double max, double defaultValue, double interval = 0.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
