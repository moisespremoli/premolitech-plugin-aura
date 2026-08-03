#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace aura
{
    // Minimal functional editor - a flat list of sliders/toggles/combo boxes
    // bound to every APVTS parameter via attachments. This is explicitly a
    // Fase 6 placeholder: it exists so the plugin is testable end to end
    // before the real custom-graphics UI (metallic panels, animated meters,
    // modular drag & drop chain view, instrument-aware layouts) is built.
    class AuraAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit AuraAudioProcessorEditor (AuraAudioProcessor& processorToEdit);
        ~AuraAudioProcessorEditor() override;

        void paint (juce::Graphics& g) override;
        void resized() override;

    private:
        using SliderAttachment  = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ButtonAttachment  = juce::AudioProcessorValueTreeState::ButtonAttachment;
        using ComboAttachment   = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

        AuraAudioProcessor& processor;

        struct Row
        {
            juce::Label label;
            juce::Slider slider;
            std::unique_ptr<SliderAttachment> attachment;
        };

        struct ToggleRow
        {
            juce::ToggleButton button;
            std::unique_ptr<ButtonAttachment> attachment;
        };

        juce::ComboBox instrumentBox;
        std::unique_ptr<ComboAttachment> instrumentAttachment;

        juce::ComboBox ampModelBox;
        std::unique_ptr<ComboAttachment> ampModelAttachment;

        std::vector<std::unique_ptr<Row>> rows;
        std::vector<std::unique_ptr<ToggleRow>> toggles;

        juce::Viewport viewport;
        juce::Component content;

        Row& addSliderRow (const juce::String& paramID, const juce::String& labelText);
        ToggleRow& addToggleRow (const juce::String& paramID, const juce::String& labelText);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuraAudioProcessorEditor)
    };
}
