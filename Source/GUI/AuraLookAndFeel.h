#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aura
{
    // Hand-drawn "hardware amp" look, entirely procedural (gradients/paths -
    // no bitmaps): chrome-bezel rotary knobs with an LED-style value arc,
    // illuminated round LED toggles for module bypass, and dark brushed-metal
    // combo boxes. Applied globally to the editor via setLookAndFeel().
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
    };
}
