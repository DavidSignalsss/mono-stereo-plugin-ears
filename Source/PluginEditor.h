#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MonoStereoAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MonoStereoAudioProcessorEditor (MonoStereoAudioProcessor&);
    ~MonoStereoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& event) override;

private:
    MonoStereoAudioProcessor& audioProcessor;

    juce::Image backgroundImage;
    
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    
    juce::Label volumeLabel;
    juce::Label panLabel;

    bool isStereoMode = false;
    juce::Rectangle<int> switchArea;

    void toggleMode();
    void updatePanSliderState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoStereoAudioProcessorEditor)
};