#include "ModulePanel.h"

namespace aura
{
    namespace
    {
        const juce::Colour creamBottom  (0xffe6dbc4);
        const juce::Colour inkBrown     (0xff443322);
        const juce::Colour printedText  (0xffd9d3c4);
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
        knob->caption.setColour (juce::Label::textColourId, printedText);

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
        statusLabel.setColour (juce::Label::textColourId, printedText.withAlpha (0.8f));
        statusLabel.setMinimumHorizontalScale (0.7f);
        addAndMakeVisible (statusLabel);

        return toolbarButton;
    }

    void ModulePanel::setStatusText (const juce::String& text)
    {
        statusLabel.setText (text, juce::dontSendNotification);
    }

    void ModulePanel::layoutKnobsExplicit (const std::vector<juce::Point<int>>& centresInParentSpace, int diameter)
    {
        explicitLayout = true;
        for (size_t i = 0; i < knobs.size() && i < centresInParentSpace.size(); ++i)
        {
            const auto centreLocal = centresInParentSpace[i] - getPosition();
            knobs[i]->slider.setBounds (juce::Rectangle<int> (diameter, diameter).withCentre (centreLocal));
            knobs[i]->caption.setVisible (false);
        }
    }

    void ModulePanel::layoutBypassExplicit (juce::Point<int> centreInParentSpace, int diameter)
    {
        explicitLayout = true;
        const auto centreLocal = centreInParentSpace - getPosition();
        bypassButton.setBounds (juce::Rectangle<int> (diameter, diameter).withCentre (centreLocal));
    }

    void ModulePanel::setExplicitLayoutMode (bool shouldUseExplicitLayout)
    {
        explicitLayout = shouldUseExplicitLayout;
    }

    void ModulePanel::layoutComboExplicit (juce::Rectangle<int> boundsInParentSpace)
    {
        explicitLayout = true;
        comboBox.setBounds (boundsInParentSpace.translated (-getPosition().x, -getPosition().y));
    }

    void ModulePanel::layoutToolbarExplicit (juce::Rectangle<int> buttonBoundsInParentSpace,
                                              juce::Rectangle<int> statusBoundsInParentSpace)
    {
        explicitLayout = true;
        toolbarButton.setBounds (buttonBoundsInParentSpace.translated (-getPosition().x, -getPosition().y));
        statusLabel.setBounds (statusBoundsInParentSpace.translated (-getPosition().x, -getPosition().y));
    }

    void ModulePanel::paint (juce::Graphics& g)
    {
        // In explicit-layout mode the section title and knob captions are
        // already baked into a background image (see PluginEditor) - drawing
        // our own here would just duplicate/misalign them.
        if (explicitLayout)
            return;

        // No background, border, or corner screws here by design - this
        // group sits directly on the editor's single continuous steel
        // panel (see PluginEditor::paint()), matching how PLI-1A/PLI-2A/
        // PHATTER print all their labelled knob groups straight onto one
        // seamless painted panel rather than nesting boxes.
        auto titleArea = juce::Rectangle<int> (2, 2, getWidth() - 4, 16);
        g.setFont (juce::Font (juce::FontOptions (12.0f)).withExtraKerningFactor (0.12f).boldened());
        g.setColour (printedText);
        g.drawText (title, titleArea, juce::Justification::centredLeft);

        g.setColour (printedText.withAlpha (0.35f));
        g.drawLine ((float) titleArea.getX(), (float) titleArea.getBottom() + 1.0f,
                    (float) getWidth() - 2.0f, (float) titleArea.getBottom() + 1.0f, 1.0f);
    }

    void ModulePanel::resized()
    {
        auto bounds = getLocalBounds().reduced (4, 2);
        if (! explicitLayout)
            bounds.removeFromTop (20); // space consumed by the title + divider in paint()

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

        if (hasBypass && ! explicitLayout)
        {
            constexpr int ledSize = 16;
            bypassButton.setBounds (getWidth() - ledSize - 4, 1, ledSize, ledSize);
        }

        if (! knobs.empty() && ! explicitLayout)
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
