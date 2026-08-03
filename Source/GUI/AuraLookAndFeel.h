#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aura
{
    // Hardware-panel look mixing real photographed materials (chicken-head
    // knobs, jewel LEDs) with procedural drawing (printed numbered scale,
    // combo box). Applied globally to the editor via setLookAndFeel().
    class AuraLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        AuraLookAndFeel();

        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                float sliderPosProportional, float rotaryStartAngle,
                                float rotaryEndAngle, juce::Slider& slider) override;

        void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                            int buttonX, int buttonY, int buttonW, int buttonH,
                            juce::ComboBox& box) override;

        juce::Font getComboBoxFont (juce::ComboBox&) override;
        juce::Font getLabelFont (juce::Label&) override;

        // When the background already prints its own numbered dial scale
        // (e.g. a photographed panel used as-is), set this false so
        // drawRotarySlider() only draws the knob itself and doesn't
        // superimpose a second, likely-misaligned scale on top of it.
        bool showPrintedScale = true;
    };
}
