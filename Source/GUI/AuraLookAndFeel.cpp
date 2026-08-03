#include "AuraLookAndFeel.h"

namespace aura
{
    namespace
    {
        const juce::Colour creamTop     (0xfffdfbf7);
        const juce::Colour creamBottom  (0xffe6dbc4);
        const juce::Colour chromeLight  (0xfff0f0f0);
        const juce::Colour chromeDark   (0xff666666);
        const juce::Colour inkBrown     (0xff443322);
        const juce::Colour scaleNumberColour (0xff2a2018);
    }

    AuraLookAndFeel::AuraLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, inkBrown);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::textColourId, inkBrown);
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff8a7a60));
        setColour (juce::PopupMenu::backgroundColourId, creamTop);
        setColour (juce::PopupMenu::textColourId, inkBrown);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xffcdbb8e));
        setColour (juce::Label::textColourId, inkBrown);
    }

    void AuraLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider&)
    {
        // The slider's full allotted area is deliberately larger than the
        // knob itself, leaving a margin for the printed numbered scale
        // (like a real amp's panel silkscreen) to sit just inside the
        // component's own bounds - drawing further out would get clipped.
        auto outerBounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);
        const auto outerRadius = juce::jmin (outerBounds.getWidth(), outerBounds.getHeight()) / 2.0f;
        const auto centre = outerBounds.getCentre();
        const auto radius = outerRadius * 0.62f;
        const auto bounds = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Printed numbered scale (vintage "1 to 11" dial plate) drawn on the
        // panel around the knob, before the knob body itself.
        constexpr int numTicks = 11;
        for (int i = 0; i < numTicks; ++i)
        {
            const auto t = (float) i / (float) (numTicks - 1);
            const auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            const auto tickRadius = outerRadius * 0.92f;
            const auto tx = centre.x + tickRadius * std::sin (tickAngle);
            const auto ty = centre.y - tickRadius * std::cos (tickAngle);

            g.setColour (scaleNumberColour);
            g.setFont (juce::Font (juce::FontOptions (juce::jmax (8.0f, outerRadius * 0.16f))).boldened());
            g.drawText (juce::String (i + 1),
                        juce::Rectangle<float> (20.0f, 12.0f).withCentre ({ tx, ty }),
                        juce::Justification::centred);
        }

        // Drop shadow.
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (bounds.translated (0.0f, 2.0f));

        // Thin chrome bezel ring.
        juce::ColourGradient bezelGradient (chromeLight, bounds.getX(), bounds.getY(),
                                             chromeDark, bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill (bezelGradient);
        g.fillEllipse (bounds);

        // Black "chicken-head" knob body.
        auto body = bounds.reduced (radius * 0.12f);
        juce::ColourGradient bodyGradient (juce::Colour (0xff2a2a2a), body.getX() + body.getWidth() * 0.3f, body.getY(),
                                            juce::Colour (0xff0d0d0d), body.getCentreX(), body.getBottom(), false);
        g.setGradientFill (bodyGradient);
        g.fillEllipse (body);

        // Subtle ridged texture ring, evoking a moulded bakelite knob.
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        juce::Path ridgeRing;
        ridgeRing.addEllipse (body.reduced (radius * 0.18f));
        g.strokePath (ridgeRing, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

        // Pointer, rotated to the current value.
        juce::Path pointer;
        const auto pointerLength = radius * 0.62f;
        const auto pointerThickness = juce::jmax (2.0f, radius * 0.09f);
        pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength * 0.95f,
                                      pointerThickness, pointerLength, pointerThickness * 0.4f);
        g.setColour (juce::Colours::white.withAlpha (0.92f));
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

        // Centre cap.
        g.setColour (juce::Colour (0xff111111));
        g.fillEllipse (juce::Rectangle<float> (radius * 0.22f, radius * 0.22f).withCentre (centre));
    }

    void AuraLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool /*highlighted*/, bool /*down*/)
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto ledDiameter = juce::jmin (bounds.getHeight() - 2.0f, bounds.getWidth() - 2.0f);
        const auto ledBounds = juce::Rectangle<float> (ledDiameter, ledDiameter).withCentre (bounds.getCentre());

        // Chrome bezel jewel-light housing.
        juce::ColourGradient bezelGradient (chromeLight, ledBounds.getX(), ledBounds.getY(),
                                             chromeDark, ledBounds.getRight(), ledBounds.getBottom(), false);
        g.setGradientFill (bezelGradient);
        g.fillEllipse (ledBounds.expanded (2.0f));

        // Bypass semantics: toggled ON == bypassed == signal NOT flowing
        // through this module, so the jewel glows red when on, green when
        // the module is actively processing.
        const bool bypassed = button.getToggleState();
        const auto colour = bypassed ? juce::Colour (0xffcc1111) : juce::Colour (0xff3f9142);

        juce::ColourGradient glow (colour.brighter (0.7f), ledBounds.getX() + ledBounds.getWidth() * 0.35f,
                                    ledBounds.getY() + ledBounds.getHeight() * 0.35f,
                                    colour.darker (0.6f), ledBounds.getCentreX(), ledBounds.getBottom(), false);
        g.setGradientFill (glow);
        g.fillEllipse (ledBounds.reduced (1.5f));

        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillEllipse (ledBounds.reduced (ledDiameter * 0.28f).translated (-ledDiameter * 0.12f, -ledDiameter * 0.12f)
                           .withSizeKeepingCentre (ledDiameter * 0.28f, ledDiameter * 0.2f));
    }

    void AuraLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                         int buttonX, int buttonY, int buttonW, int buttonH,
                                         juce::ComboBox&)
    {
        auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);

        juce::ColourGradient bodyGradient (creamTop, 0.0f, 0.0f, creamBottom, 0.0f, (float) height, false);
        g.setGradientFill (bodyGradient);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colour (0xff8a7a60));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        juce::Path arrow;
        auto arrowZone = juce::Rectangle<float> ((float) buttonX, (float) buttonY, (float) buttonW, (float) buttonH)
                              .reduced (buttonW * 0.28f, buttonH * 0.36f);
        arrow.addTriangle (arrowZone.getX(), arrowZone.getY(),
                           arrowZone.getRight(), arrowZone.getY(),
                           arrowZone.getCentreX(), arrowZone.getBottom());
        g.setColour (inkBrown);
        g.fillPath (arrow);
    }

    juce::Font AuraLookAndFeel::getComboBoxFont (juce::ComboBox&)
    {
        return juce::Font (juce::FontOptions (14.0f)).boldened().withExtraKerningFactor (0.02f);
    }

    juce::Font AuraLookAndFeel::getLabelFont (juce::Label&)
    {
        return juce::Font (juce::FontOptions (13.0f));
    }
}
