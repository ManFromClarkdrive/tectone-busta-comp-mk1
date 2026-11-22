#include "PluginEditor.h"

static juce::Colour metal(int v) { return juce::Colour(v, v, v); }

void BusterLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float pos, float start, float end, juce::Slider&)
{
    const float size = (float)juce::jmin(w, h) * 0.72f;
    const float cx = x + w * 0.5f, cy = y + h * 0.43f;
    const float r = size * 0.5f;
    const float angle = start + pos * (end - start);
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillEllipse(cx-r+2, cy-r+4, size, size);
    juce::ColourGradient body(juce::Colour(0xff777b7e), cx-r, cy-r, juce::Colour(0xff171a1d), cx+r, cy+r, false);
    body.addColour(0.45, juce::Colour(0xff3b4145));
    g.setGradientFill(body); g.fillEllipse(cx-r, cy-r, size, size);
    g.setColour(juce::Colour(0xffaeb3b4)); g.drawEllipse(cx-r, cy-r, size, size, 1.2f);
    g.setColour(juce::Colour(0xff101214)); g.fillEllipse(cx-r*0.76f, cy-r*0.76f, r*1.52f, r*1.52f);
    g.setColour(juce::Colour(0xffb6a27a)); g.drawEllipse(cx-r*0.76f, cy-r*0.76f, r*1.52f, r*1.52f, 1.0f);
    juce::Path pointer; pointer.addRoundedRectangle(-2.0f, -r*0.78f, 4.0f, r*0.34f, 1.5f);
    g.setColour(juce::Colour(0xffe7d6ad));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));
    g.setColour(juce::Colour(0xffc9c1ad)); g.fillEllipse(cx-3, cy-3, 6, 6);
}

TectoneBusterCompMK1AudioProcessorEditor::TectoneBusterCompMK1AudioProcessorEditor(TectoneBusterCompMK1AudioProcessor& p)
: AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&look); setSize(980, 590);
    title.setText("TECTONE", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe5d6b7)); title.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(title);
    subtitle.setText("BUSTER COMP  /  MK1     •     OPTICAL TUBE COMPRESSOR", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffa79b83)); subtitle.setJustificationType(juce::Justification::centredLeft); addAndMakeVisible(subtitle);
    grLabel.setText("GAIN REDUCTION", juce::dontSendNotification);
    grLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd5c8a8)); grLabel.setJustificationType(juce::Justification::centred); addAndMakeVisible(grLabel);

    const char* ids[] = {"threshold","ratio","attack","release","gain","mix","sc_hpf","warmth"};
    const char* names[] = {"THRESHOLD","RATIO","ATTACK","RELEASE","MAKEUP","MIX","SC HPF","WARMTH"};
    for (size_t i=0;i<knobs.size();++i) {
        knobs[i] = std::make_unique<Control>(names[i]);
        knobs[i]->setName(names[i]); knobs[i]->setPopupDisplayEnabled(true, true, this);
        addAndMakeVisible(*knobs[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, ids[i], *knobs[i]);
    }
    startTimerHz(24);
}
TectoneBusterCompMK1AudioProcessorEditor::~TectoneBusterCompMK1AudioProcessorEditor() { setLookAndFeel(nullptr); stopTimer(); }

void TectoneBusterCompMK1AudioProcessorEditor::timerCallback()
{ gr = processor.getGainReduction(); repaint(20, 105, 940, 105); }

void TectoneBusterCompMK1AudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient bg(juce::Colour(0xff34393c), 0, 0, juce::Colour(0xff101315), 0, (float)getHeight(), false);
    bg.addColour(0.35, juce::Colour(0xff22282b)); g.setGradientFill(bg); g.fillAll();
    g.setColour(juce::Colour(0xff77736a)); g.drawRoundedRectangle(bounds.reduced(7), 9, 2);
    g.setColour(juce::Colour(0xff090b0c)); g.fillRoundedRectangle(18, 18, 944, 78, 5);
    g.setColour(juce::Colour(0xffa68f65)); g.drawRoundedRectangle(18, 18, 944, 78, 5, 1.2f);
    g.setColour(juce::Colour(0xff1a1e20)); g.fillRoundedRectangle(24, 112, 932, 126, 6);
    g.setColour(juce::Colour(0xff786a50)); g.drawRoundedRectangle(24, 112, 932, 126, 6, 1.0f);

    // Simulated illuminated VU / gain-reduction meter.
    g.setColour(juce::Colour(0xff080a0b)); g.fillRoundedRectangle(260, 142, 460, 62, 5);
    g.setColour(juce::Colour(0xffb8a37b)); g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("0      -2      -4      -6      -8      -10      -12      -14      -16 dB", 274, 145, 432, 15, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd6b77c)); g.fillRoundedRectangle(278, 172, juce::jlimit(0.0f, 420.0f, gr * 25.0f), 13, 3);
    g.setColour(juce::Colour(0xff756b58)); g.drawRoundedRectangle(278, 172, 420, 13, 3, 1.0f);
    g.setColour(juce::Colour(0xffc8bda5)); g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("HAND-BUILT CHARACTER  •  TECTONE STUDIOS", 28, 552, 920, 18, juce::Justification::centred);
    g.setColour(juce::Colour(0xffb7a47f)); g.drawLine(30, 250, 950, 250, 1.0f);
    g.setColour(juce::Colour(0xff8e8066)); g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("INPUT / OPTICAL CONTROL", 34, 254, 360, 18, juce::Justification::centredLeft);
    g.drawText("OUTPUT / TONE", 604, 254, 340, 18, juce::Justification::centredRight);
}

void TectoneBusterCompMK1AudioProcessorEditor::resized()
{
    title.setBounds(38, 28, 250, 42); subtitle.setBounds(40, 67, 600, 20);
    grLabel.setBounds(380, 115, 220, 20);
    const int y = 285, w = 112, h = 205, gap = 8;
    const int startX = 28;
    for (int i=0;i<8;++i) knobs[(size_t)i]->setBounds(startX + i*(w+gap), y, w, h);
}
