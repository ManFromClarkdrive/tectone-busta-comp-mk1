#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class BusterLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};

class TectoneBusterCompMK1AudioProcessorEditor : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit TectoneBusterCompMK1AudioProcessorEditor(TectoneBusterCompMK1AudioProcessor&);
    ~TectoneBusterCompMK1AudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    struct Control : juce::Slider
    {
        Control(const juce::String& labelText) : title(labelText)
        { setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20); setColour(textBoxTextColourId, juce::Colour(0xffe6dfcc)); setColour(textBoxBackgroundColourId, juce::Colour(0xff171a1d)); setColour(textBoxOutlineColourId, juce::Colour(0xff565044)); }
        juce::String title;
    };
    TectoneBusterCompMK1AudioProcessor& processor;
    BusterLookAndFeel look;
    std::array<std::unique_ptr<Control>, 8> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> attachments;
    juce::Label title, subtitle, grLabel;
    float gr = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TectoneBusterCompMK1AudioProcessorEditor)
};
