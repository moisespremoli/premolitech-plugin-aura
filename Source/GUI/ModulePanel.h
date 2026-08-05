#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>
#include <functional>

namespace aura
{
    // A single labelled group of controls on the amp's single continuous
    // steel front panel: a printed section title, an optional bypass LED,
    // an optional combo box (e.g. the amp model selector), and a row of
    // rotary knobs each with a caption underneath. Deliberately has no
    // background fill, border, or corner screws of its own - unlike a real
    // Premoli Labs rack unit (PLI-1A, PLI-2A, PHATTER), which prints all of
    // its labelled knob groups directly on one seamless painted panel
    // rather than nesting each group in its own bordered box. Used once per
    // DSP module (Gate, Comp, Amp, Cab, EQ, Limiter) so the panel layout
    // mirrors the real signal chain instead of being a flat parameter list.
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

        // Positions every knob added so far at an explicit centre (given in
        // this component's PARENT's coordinate space, translated to local
        // internally) instead of the normal even-distribution layout, and
        // hides the knob captions - for panels drawn over a background
        // image that already shows each knob's real position and printed
        // label (e.g. a photographed faceplate used as-is). Also suppresses
        // this panel's own title/divider drawing, since the label is
        // already baked into that background. Bounds still need to be set
        // (as usual, via setBounds()) large enough to contain every knob's
        // circle, since JUCE clips a component's children to its own
        // bounds.
        // showCaptionBelow draws this panel's own caption label under a
        // knob instead of hiding it - for the rare knob (e.g. CABINET's
        // MIX) that has no printed label of its own in the background to
        // rely on.
        void layoutKnobsExplicit (const std::vector<juce::Point<int>>& centresInParentSpace, int diameter,
                                   bool showCaptionsBelow = false);

        // As layoutKnobsExplicit(), for the single bypass LED.
        void layoutBypassExplicit (juce::Point<int> centreInParentSpace, int diameter);

        // As layoutKnobsExplicit(), for the combo box / toolbar button.
        void layoutComboExplicit (juce::Rectangle<int> boundsInParentSpace);
        void layoutToolbarExplicit (juce::Rectangle<int> buttonBoundsInParentSpace,
                                     juce::Rectangle<int> statusBoundsInParentSpace);

        // Switches on explicit-layout mode (see layoutKnobsExplicit) before
        // the first setBounds()/resized() pass, so the combo box/toolbar
        // button - which still use the normal header-row layout - don't
        // reserve space for a title that will never be drawn. Call this
        // once, right after adding all of a panel's controls.
        void setExplicitLayoutMode (bool shouldUseExplicitLayout);

        void paint (juce::Graphics& g) override;
        void resized() override;

    private:
        bool explicitLayout = false;
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
