#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>
#include <functional>

namespace aura
{
    // A single "module" on the amp's front panel: an inset metal frame with
    // an engraved title, an optional bypass LED in the corner, an optional
    // combo box (e.g. the amp model selector), and a row of rotary knobs
    // each with a caption underneath. Used once per DSP module (Gate, Comp,
    // Amp, Cab, EQ, Limiter) so the panel layout mirrors the real signal
    // chain instead of being a flat parameter list.
    class ModulePanel : public juce::Component
    {
    public:
        explicit ModulePanel (juce::String titleText);

        juce::Slider& addKnob (juce::AudioProcessorValueTreeState& apvts,
                                const juce::String& paramID, const juce::String& caption);

        void addBypassToggle (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);

        juce::ComboBox& addComboBox (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& paramID, const juce::StringArray& choices);

        // A small action button in the header row (e.g. "LOAD IR...") with a
        // status label taking up the rest of the row to show e.g. the
        // currently loaded file's name. Mutually exclusive with addComboBox()
        // in practice - only one panel needs either.
        juce::TextButton& addToolbarButton (const juce::String& buttonText, std::function<void()> onClick);
        void setStatusText (const juce::String& text);

        void paint (juce::Graphics& g) override;
        void resized() override;

    private:
        juce::String title;

        struct Knob
        {
            juce::Slider slider;
            juce::Label caption;
            std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        };
        std::vector<std::unique_ptr<Knob>> knobs;

        juce::ToggleButton bypassButton;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
        bool hasBypass = false;

        juce::ComboBox comboBox;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment;
        bool hasCombo = false;

        juce::TextButton toolbarButton;
        juce::Label statusLabel;
        bool hasToolbar = false;
    };
}
