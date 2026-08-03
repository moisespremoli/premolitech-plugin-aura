#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aura
{
    // Hand-drawn "vintage tube amp" look, entirely procedural (gradients/
    // paths/text - no bitmaps): black chicken-head knobs on a printed
    // numbered scale (1-11), jewel-style LED toggles for module bypass, and
    // ivory/cream combo boxes with a thin chrome bezel. Applied globally to
    // the editor via setLookAndFeel(). Colour choices deliberately evoke
    // "vintage American tube amp" in general (cream panel, black covering,
    // chrome corners) rather than any single manufacturer's specific trade
    // dress or logo.
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
