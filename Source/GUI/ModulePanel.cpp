#include "ModulePanel.h"

namespace aura
{
    namespace
    {
        const juce::Colour creamTop     (0xfffdfbf7);
        const juce::Colour creamBottom  (0xffe6dbc4);
        const juce::Colour tanBorder    (0xff8a7a60);
        const juce::Colour inkBrown     (0xff443322);
        const juce::Colour chromeLight  (0xfff0f0f0);
        const juce::Colour chromeDark   (0xff666666);

        // A small chrome screw head, like the ones holding a real amp's
        // control-panel faceplate on - purely decorative, drawn at each
        // panel's four corners.
        void drawScrew (juce::Graphics& g, juce::Point<float> centre)
        {
            constexpr float r = 5.0f;
            juce::Rectangle<float> bounds (r * 2.0f, r * 2.0f);
            bounds.setCentre (centre);

            juce::ColourGradient grad (chromeLight, bounds.getX(), bounds.getY(),
                                        chromeDark, bounds.getRight(), bounds.getBottom(), false);
            g.setGradientFill (grad);
            g.fillEllipse (bounds);
            g.setColour (juce::Colours::black.withAlpha (0.4f));
            g.drawEllipse (bounds, 0.5f);

            juce::Path slot;
            slot.addRoundedRectangle (-r * 0.7f, -0.6f, r * 1.4f, 1.2f, 0.5f);
            g.setColour (juce::Colour (0xff2a2a2a));
            g.fillPath (slot, juce::AffineTransform::rotation (0.5f).translated (centre.x, centre.y));
        }
    }

    ModulePanel::ModulePanel (juce::String titleText) : title (std::move (titleText))
    {
        bypassButton.setVisible (false);
        comboBox.setVisible (false);
        toolbarButton.setVisible (false);
        statusLabel.setVisible (false);
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
        knob->caption.setFont (juce::Font (juce::FontOptions (11.0f)).boldened());
        knob->caption.setColour (juce::Label::textColourId, inkBrown);

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

    juce::TextButton& ModulePanel::addToolbarButton (const juce::String& buttonText, std::function<void()> onClick)
    {
        hasToolbar = true;
        toolbarButton.setButtonText (buttonText);
        toolbarButton.setVisible (true);
        toolbarButton.onClick = std::move (onClick);
        toolbarButton.setColour (juce::TextButton::buttonColourId, creamBottom);
        toolbarButton.setColour (juce::TextButton::textColourOffId, inkBrown);
        addAndMakeVisible (toolbarButton);

        statusLabel.setVisible (true);
        statusLabel.setJustificationType (juce::Justification::centredLeft);
        statusLabel.setFont (juce::Font (juce::FontOptions (10.0f)));
        statusLabel.setColour (juce::Label::textColourId, inkBrown.withAlpha (0.75f));
        statusLabel.setMinimumHorizontalScale (0.7f);
        addAndMakeVisible (statusLabel);

        return toolbarButton;
    }

    void ModulePanel::setStatusText (const juce::String& text)
    {
        statusLabel.setText (text, juce::dontSendNotification);
    }

    void ModulePanel::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient panelGradient (creamTop, 0.0f, 0.0f, creamBottom, 0.0f, bounds.getHeight(), false);
        g.setGradientFill (panelGradient);
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (tanBorder.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds.reduced (1.0f), 6.0f, 1.5f);

        for (auto corner : { juce::Point<float> (14.0f, 14.0f),
                              juce::Point<float> (bounds.getWidth() - 14.0f, 14.0f),
                              juce::Point<float> (14.0f, bounds.getHeight() - 14.0f),
                              juce::Point<float> (bounds.getWidth() - 14.0f, bounds.getHeight() - 14.0f) })
            drawScrew (g, corner);

        auto titleArea = juce::Rectangle<int> (24, 4, getWidth() - 44, 18);
        g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.1f).boldened());
        g.setColour (inkBrown);
        g.drawText (title, titleArea, juce::Justification::centredLeft);
    }

    void ModulePanel::resized()
    {
        auto bounds = getLocalBounds().reduced (20, 8);
        bounds.removeFromTop (14); // space consumed by the title in paint()

        auto headerRow = bounds.removeFromTop ((hasCombo || hasToolbar) ? 24 : 0);
        if (hasCombo)
        {
            comboBox.setBounds (headerRow);
        }
        else if (hasToolbar)
        {
            toolbarButton.setBounds (headerRow.removeFromLeft (78));
            headerRow.removeFromLeft (6);
            statusLabel.setBounds (headerRow);
        }

        if (hasBypass)
        {
            constexpr int ledSize = 16;
            bypassButton.setBounds (getWidth() - ledSize - 22, 6, ledSize, ledSize);
        }

        if (! knobs.empty())
        {
            const auto knobAreaWidth = bounds.getWidth() / (int) knobs.size();
            for (auto& k : knobs)
            {
                auto cell = bounds.removeFromLeft (knobAreaWidth);
                auto captionArea = cell.removeFromBottom (14);
                const auto knobSize = juce::jmin (cell.getWidth(), cell.getHeight());
                k->slider.setBounds (cell.withSizeKeepingCentre (knobSize, knobSize));
                k->caption.setBounds (captionArea);
            }
        }
    }
}
