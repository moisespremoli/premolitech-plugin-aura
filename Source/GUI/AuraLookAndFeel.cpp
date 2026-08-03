#include "AuraLookAndFeel.h"

namespace aura
{
    AuraLookAndFeel::AuraLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::whitesmoke);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::textColourId, juce::Colours::whitesmoke);
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff5a5a60));
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1e1e22));
        setColour (juce::PopupMenu::textColourId, juce::Colours::whitesmoke);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xffe8a33d));
        setColour (juce::Label::textColourId, juce::Colours::whitesmoke);
    }

    void AuraLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider&)
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Drop shadow, offset slightly down so the knob reads as raised off
        // the panel.
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (bounds.translated (0.0f, 2.0f));

        // Chrome bezel ring - a diagonal light-to-dark gradient reads as a
        // curved metal rim under a single light source.
        juce::ColourGradient bezelGradient (juce::Colour (0xffcfcfd4), bounds.getX(), bounds.getY(),
                                             juce::Colour (0xff2b2b2e), bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill (bezelGradient);
        g.fillEllipse (bounds);

        // Recessed knob body.
        auto body = bounds.reduced (radius * 0.14f);
        juce::ColourGradient bodyGradient (juce::Colour (0xff4a4a50), body.getX() + body.getWidth() * 0.3f, body.getY(),
                                            juce::Colour (0xff141416), body.getCentreX(), body.getBottom(), false);
        g.setGradientFill (bodyGradient);
        g.fillEllipse (body);

        // Specular highlight, upper-left, to sell the glossy plastic/metal
        // cap look.
        auto highlight = body.reduced (radius * 0.3f).withSizeKeepingCentre (radius * 0.55f, radius * 0.4f)
                              .translated (-radius * 0.18f, -radius * 0.3f);
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillEllipse (highlight);

        // Value arc: a dim full-range track behind a lit amber arc showing
        // the current position, like a UAD/Neural-DSP style indicator ring.
        const auto arcRadius = radius * 0.98f;
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                              rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff2a2a2e));
        g.strokePath (track, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (0xffe8a33d));
        g.strokePath (valueArc, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Pointer, rotated to the current value.
        juce::Path pointer;
        const auto pointerLength = radius * 0.6f;
        const auto pointerThickness = juce::jmax (2.0f, radius * 0.1f);
        pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength * 0.95f,
                                      pointerThickness, pointerLength, pointerThickness * 0.4f);
        g.setColour (juce::Colours::whitesmoke.withAlpha (0.9f));
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

        // Centre cap.
        g.setColour (juce::Colour (0xff1c1c1e));
        g.fillEllipse (juce::Rectangle<float> (radius * 0.26f, radius * 0.26f).withCentre (centre));
    }

    void AuraLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool /*highlighted*/, bool /*down*/)
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto ledDiameter = juce::jmin (bounds.getHeight() - 2.0f, bounds.getWidth() - 2.0f);
        const auto ledBounds = juce::Rectangle<float> (ledDiameter, ledDiameter).withCentre (bounds.getCentre());

        g.setColour (juce::Colour (0xff0c0c0e));
        g.fillEllipse (ledBounds.expanded (2.0f));

        // Bypass semantics: toggled ON == bypassed == signal NOT flowing
        // through this module, so the LED reads red when on, green when the
        // module is actively processing - matching how a real pedal's status
        // LED works (lit = engaged).
        const bool bypassed = button.getToggleState();
        const auto colour = bypassed ? juce::Colour (0xff8a2c26) : juce::Colour (0xff4caf50);

        juce::ColourGradient glow (colour.brighter (0.7f), ledBounds.getCentreX(), ledBounds.getY(),
                                    colour.darker (0.5f), ledBounds.getCentreX(), ledBounds.getBottom(), false);
        g.setGradientFill (glow);
        g.fillEllipse (ledBounds);

        if (! bypassed)
        {
            g.setColour (colour.withAlpha (0.30f));
            g.fillEllipse (ledBounds.expanded (3.0f));
        }

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawEllipse (ledBounds.reduced (1.0f), 1.0f);
    }

    void AuraLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                         int buttonX, int buttonY, int buttonW, int buttonH,
                                         juce::ComboBox&)
    {
        auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);

        juce::ColourGradient bodyGradient (juce::Colour (0xff3d3d42), 0.0f, 0.0f,
                                            juce::Colour (0xff17171a), 0.0f, (float) height, false);
        g.setGradientFill (bodyGradient);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colour (0xff5a5a60));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        juce::Path arrow;
        auto arrowZone = juce::Rectangle<float> ((float) buttonX, (float) buttonY, (float) buttonW, (float) buttonH)
                              .reduced (buttonW * 0.28f, buttonH * 0.36f);
        arrow.addTriangle (arrowZone.getX(), arrowZone.getY(),
                           arrowZone.getRight(), arrowZone.getY(),
                           arrowZone.getCentreX(), arrowZone.getBottom());
        g.setColour (juce::Colour (0xffe8a33d));
        g.fillPath (arrow);
    }

    juce::Font AuraLookAndFeel::getComboBoxFont (juce::ComboBox&)
    {
        return juce::Font (juce::FontOptions (14.0f)).withExtraKerningFactor (0.02f);
    }

    juce::Font AuraLookAndFeel::getLabelFont (juce::Label&)
    {
        return juce::Font (juce::FontOptions (13.0f));
    }
}
