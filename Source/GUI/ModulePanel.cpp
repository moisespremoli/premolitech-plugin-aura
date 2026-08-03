#include "ModulePanel.h"

namespace aura
{
    ModulePanel::ModulePanel (juce::String titleText) : title (std::move (titleText))
    {
        bypassButton.setVisible (false);
        comboBox.setVisible (false);
    }

    juce::Slider& ModulePanel::addKnob (juce::AudioProcessorValueTreeState& apvts,
                                        const juce::String& paramID, const juce::String& captionText)
    {
        auto knob = std::make_unique<Knob>();
        knob->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob->slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob->slider.setPopupDisplayEnabled (true, true, this);
        knob->caption.setText (captionText, juce::dontSendNotification);
        knob->caption.setJustificationType (juce::Justification::centred);
        knob->caption.setFont (juce::Font (juce::FontOptions (11.0f)));
        knob->caption.setColour (juce::Label::textColourId, juce::Colours::whitesmoke.withAlpha (0.8f));

        addAndMakeVisible (knob->slider);
        addAndMakeVisible (knob->caption);
        knob->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramID, knob->slider);

        knobs.push_back (std::move (knob));
        return knobs.back()->slider;
    }

    void ModulePanel::addBypassToggle (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
    {
        hasBypass = true;
        bypassButton.setVisible (true);
        addAndMakeVisible (bypassButton);
        bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, paramID, bypassButton);
    }

    juce::ComboBox& ModulePanel::addComboBox (juce::AudioProcessorValueTreeState& apvts,
                                               const juce::String& paramID, const juce::StringArray& choices)
    {
        hasCombo = true;
        comboBox.setVisible (true);
        comboBox.addItemList (choices, 1);
        addAndMakeVisible (comboBox);
        comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, paramID, comboBox);

        // The attachment syncs asynchronously via the parameter's listener;
        // set the visible text immediately from the parameter's current
        // value so the box never renders blank on first paint.
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramID)))
            comboBox.setText (choiceParam->getCurrentChoiceName(), juce::dontSendNotification);

        return comboBox;
    }

    void ModulePanel::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient panelGradient (juce::Colour (0xff333338), 0.0f, 0.0f,
                                             juce::Colour (0xff1c1c1f), 0.0f, bounds.getHeight(), false);
        g.setGradientFill (panelGradient);
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawRoundedRectangle (bounds.reduced (1.0f), 6.0f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        // Engraved title: a dark offset copy under a light one reads as
        // stamped/etched metal rather than flat printed text.
        auto titleArea = juce::Rectangle<int> (10, 4, getWidth() - 20, 18);
        g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.08f).boldened());
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawText (title, titleArea.translated (0, 1), juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffb8b8be));
        g.drawText (title, titleArea, juce::Justification::centredLeft);
    }

    void ModulePanel::resized()
    {
        auto bounds = getLocalBounds().reduced (8);
        bounds.removeFromTop (18); // space consumed by the engraved title in paint()

        auto headerRow = bounds.removeFromTop (hasCombo ? 24 : 0);
        if (hasCombo)
            comboBox.setBounds (headerRow);

        if (hasBypass)
        {
            constexpr int ledSize = 16;
            bypassButton.setBounds (getWidth() - ledSize - 8, 6, ledSize, ledSize);
        }

        if (! knobs.empty())
        {
            const auto knobAreaWidth = bounds.getWidth() / (int) knobs.size();
            for (auto& k : knobs)
            {
                auto cell = bounds.removeFromLeft (knobAreaWidth);
                auto captionArea = cell.removeFromBottom (14);
                const auto knobSize = juce::jmin (cell.getWidth(), cell.getHeight()) - 4;
                k->slider.setBounds (cell.withSizeKeepingCentre (knobSize, knobSize));
                k->caption.setBounds (captionArea);
            }
        }
    }
}
