#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"

namespace aura
{
    AuraAudioProcessorEditor::AuraAudioProcessorEditor (AuraAudioProcessor& processorToEdit)
        : AudioProcessorEditor (&processorToEdit), processor (processorToEdit)
    {
        instrumentBox.addItemList (getInstrumentChoices(), 1);
        content.addAndMakeVisible (instrumentBox);
        instrumentAttachment = std::make_unique<ComboAttachment> (
            processor.apvts, ParamIDs::instrument, instrumentBox);

        if (auto* ampModelParam = dynamic_cast<juce::AudioParameterChoice*> (
                processor.apvts.getParameter (ParamIDs::ampModel)))
            ampModelBox.addItemList (ampModelParam->choices, 1);
        content.addAndMakeVisible (ampModelBox);
        ampModelAttachment = std::make_unique<ComboAttachment> (
            processor.apvts, ParamIDs::ampModel, ampModelBox);

        addToggleRow (ParamIDs::gateBypass, "Gate Bypass");
        addSliderRow (ParamIDs::gateThreshold, "Gate Threshold");
        addSliderRow (ParamIDs::gateAttack, "Gate Attack");
        addSliderRow (ParamIDs::gateRelease, "Gate Release");

        addToggleRow (ParamIDs::compBypass, "Comp Bypass");
        addSliderRow (ParamIDs::compThreshold, "Comp Threshold");
        addSliderRow (ParamIDs::compRatio, "Comp Ratio");
        addSliderRow (ParamIDs::compAttack, "Comp Attack");
        addSliderRow (ParamIDs::compRelease, "Comp Release");
        addSliderRow (ParamIDs::compMakeup, "Comp Makeup");

        addToggleRow (ParamIDs::ampBypass, "Amp Bypass");
        addSliderRow (ParamIDs::ampGain, "Amp Gain");
        addSliderRow (ParamIDs::ampBass, "Amp Bass");
        addSliderRow (ParamIDs::ampMid, "Amp Mid");
        addSliderRow (ParamIDs::ampTreble, "Amp Treble");
        addSliderRow (ParamIDs::ampPresence, "Amp Presence");
        addSliderRow (ParamIDs::ampMaster, "Amp Master");

        addToggleRow (ParamIDs::cabBypass, "Cab Bypass");
        addSliderRow (ParamIDs::cabMix, "Cab Mix");

        addToggleRow (ParamIDs::eqBypass, "EQ Bypass");
        addSliderRow (ParamIDs::eqLow, "EQ Low");
        addSliderRow (ParamIDs::eqMid, "EQ Mid");
        addSliderRow (ParamIDs::eqHigh, "EQ High");

        addToggleRow (ParamIDs::limiterBypass, "Limiter Bypass");
        addSliderRow (ParamIDs::limiterCeiling, "Limiter Ceiling");

        addSliderRow (ParamIDs::inputGain, "Input Gain");
        addSliderRow (ParamIDs::outputGain, "Output Gain");

        viewport.setViewedComponent (&content, false);
        viewport.setScrollBarsShown (true, false);
        addAndMakeVisible (viewport);

        setResizable (true, true);
        setSize (420, 600);
    }

    AuraAudioProcessorEditor::~AuraAudioProcessorEditor() = default;

    AuraAudioProcessorEditor::Row& AuraAudioProcessorEditor::addSliderRow (
        const juce::String& paramID, const juce::String& labelText)
    {
        auto row = std::make_unique<Row>();
        row->label.setText (labelText, juce::dontSendNotification);
        row->slider.setSliderStyle (juce::Slider::LinearHorizontal);
        row->slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 20);
        content.addAndMakeVisible (row->label);
        content.addAndMakeVisible (row->slider);
        row->attachment = std::make_unique<SliderAttachment> (processor.apvts, paramID, row->slider);
        rows.push_back (std::move (row));
        return *rows.back();
    }

    AuraAudioProcessorEditor::ToggleRow& AuraAudioProcessorEditor::addToggleRow (
        const juce::String& paramID, const juce::String& labelText)
    {
        auto toggle = std::make_unique<ToggleRow>();
        toggle->button.setButtonText (labelText);
        content.addAndMakeVisible (toggle->button);
        toggle->attachment = std::make_unique<ButtonAttachment> (processor.apvts, paramID, toggle->button);
        toggles.push_back (std::move (toggle));
        return *toggles.back();
    }

    void AuraAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colour (0xff1a1a1e));
    }

    void AuraAudioProcessorEditor::resized()
    {
        viewport.setBounds (getLocalBounds());

        constexpr int rowHeight = 28;
        constexpr int padding = 8;
        const int contentWidth = getWidth() - viewport.getScrollBarThickness() - 2 * padding;

        int y = padding;

        auto layoutCombo = [&] (juce::ComboBox& box)
        {
            box.setBounds (padding, y, contentWidth, rowHeight - 4);
            y += rowHeight;
        };
        layoutCombo (instrumentBox);
        layoutCombo (ampModelBox);

        // Placeholder GUI: bypass toggles first, then every slider. Signal
        // chain ordering isn't meaningful here - Fase 6 replaces this whole
        // layout with the real modular chain view.
        for (auto& t : toggles)
        {
            t->button.setBounds (padding, y, contentWidth, rowHeight - 4);
            y += rowHeight;
        }

        constexpr int labelWidth = 110;
        for (auto& r : rows)
        {
            r->label.setBounds (padding, y, labelWidth, rowHeight - 4);
            r->slider.setBounds (padding + labelWidth, y, contentWidth - labelWidth, rowHeight - 4);
            y += rowHeight;
        }

        content.setSize (contentWidth + 2 * padding, y + padding);
    }
}
