#pragma once
#include <JuceHeader.h>

class TectoneBusterCompMK1AudioProcessor : public juce::AudioProcessor
{
public:
    TectoneBusterCompMK1AudioProcessor();
    ~TectoneBusterCompMK1AudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "TECTONE BUSTER COMP MK1"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double get tailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    float getGainReduction() const { return gainReductionDb.load(); }
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    std::atomic<float> gainReductionDb { 0.0f };
    double sampleRateHz = 44100.0;
    float envelope = 0.0f, hpX1L = 0.0f, hpY1L = 0.0f, hpX1R = 0.0f, hpY1R = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TectoneBusterCompMK1AudioProcessor)
};
