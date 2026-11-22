#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static float dbToGain(float db) { return std::pow(10.0f, db / 20.0f); }
static float gainToDb(float g) { return 20.0f * std::log10(std::max(g, 1.0e-8f)); }

TectoneBusterCompMK1AudioProcessor::TectoneBusterCompMK1AudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  parameters(*this, nullptr, "BUSTER_STATE", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout TectoneBusterCompMK1AudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<APF>("threshold", "Threshold", juce::NormalisableRange<float>(-36.0f, 0.0f, 0.1f), -18.0f));
    p.add(std::make_unique<APF>("ratio", "Ratio", juce::NormalisableRange<float>(1.5f, 10.0f, 0.1f), 3.0f));
    p.add(std::make_unique<APF>("attack", "Attack", juce::NormalisableRange<float>(0.5f, 100.0f, 0.1f, 0.45f), 12.0f));
    p.add(std::make_unique<APF>("release", "Release", juce::NormalisableRange<float>(40.0f, 1200.0f, 1.0f, 0.45f), 260.0f));
    p.add(std::make_unique<APF>("gain", "Makeup", juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f), 3.0f));
    p.add(std::make_unique<APF>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    p.add(std::make_unique<APF>("sc_hpf", "Sidechain HPF", juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 80.0f));
    p.add(std::make_unique<APF>("warmth", "Warmth", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 22.0f));
    return p;
}

bool TectoneBusterCompMK1AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void TectoneBusterCompMK1AudioProcessor::prepareToPlay(double sr, int)
{
    sampleRateHz = sr; envelope = hpX1L = hpY1L = hpX1R = hpY1R = 0.0f;
    gainReductionDb.store(0.0f);
}

void TectoneBusterCompMK1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    const float threshold = *parameters.getRawParameterValue("threshold");
    const float ratio = *parameters.getRawParameterValue("ratio");
    const float attackMs = *parameters.getRawParameterValue("attack");
    const float releaseMs = *parameters.getRawParameterValue("release");
    const float makeup = dbToGain(*parameters.getRawParameterValue("gain"));
    const float wet = *parameters.getRawParameterValue("mix") * 0.01f;
    const float warmth = *parameters.getRawParameterValue("warmth") * 0.01f;
    const float hpf = *parameters.getRawParameterValue("sc_hpf");
    const float hpCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * hpf / (float)sampleRateHz);
    const float attackCoeff = std::exp(-1.0f / (0.001f * attackMs * (float)sampleRateHz));
    const float releaseCoeff = std::exp(-1.0f / (0.001f * releaseMs * (float)sampleRateHz));
    float maxGr = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float inL = left[i], inR = right ? right[i] : inL;
        const float scL = inL - hpX1L + hpCoeff * hpY1L;
        const float scR = inR - hpX1R + hpCoeff * hpY1R;
        hpX1L = inL; hpY1L = scL; hpX1R = inR; hpY1R = scR;
        const float detector = std::sqrt(0.5f * (scL * scL + scR * scR) + 1.0e-12f);
        const float coeff = detector > envelope ? attackCoeff : releaseCoeff;
        envelope = coeff * envelope + (1.0f - coeff) * detector;

        const float levelDb = gainToDb(envelope);
        const float over = levelDb - threshold;
        // Smooth soft-knee transition (6 dB knee), followed by a progressive optical-style curve.
        const float knee = 6.0f;
        float reduction = 0.0f;
        if (over > -knee * 0.5f)
        {
            const float x = juce::jmax(0.0f, over + knee * 0.5f);
            const float hard = x * (1.0f - 1.0f / ratio);
            const float blend = juce::jlimit(0.0f, 1.0f, (over + knee * 0.5f) / knee);
            reduction = hard * blend;
        }
        // Gentle program-dependent saturation in the gain-control path.
        reduction = reduction * (1.0f - 0.035f * juce::jlimit(0.0f, 1.0f, reduction / 18.0f));
        maxGr = juce::jmax(maxGr, reduction);
        const float compGain = dbToGain(-reduction) * makeup;

        auto colour = [warmth](float x)
        {
            const float drive = 1.0f + warmth * 1.8f;
            const float shaped = std::tanh(x * drive) / std::tanh(drive);
            return x * (1.0f - warmth * 0.18f) + shaped * (warmth * 0.18f);
        };
        const float wetL = colour(inL * compGain);
        const float wetR = colour(inR * compGain);
        left[i] = inL * (1.0f - wet) + wetL * wet;
        if (right) right[i] = inR * (1.0f - wet) + wetR * wet;
    }
    gainReductionDb.store(maxGr);
}

juce::AudioProcessorEditor* TectoneBusterCompMK1AudioProcessor::createEditor()
{ return new TectoneBusterCompMK1AudioProcessorEditor(*this); }

void TectoneBusterCompMK1AudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void TectoneBusterCompMK1AudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new TectoneBusterCompMK1AudioProcessor(); }
