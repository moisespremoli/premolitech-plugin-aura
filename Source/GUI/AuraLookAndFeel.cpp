#include "AuraLookAndFeel.h"
#include "UIAssets.h"

namespace aura
{
    namespace
    {
        const juce::Colour creamTop     (0xfffdfbf7);
        const juce::Colour creamBottom  (0xffe6dbc4);
        const juce::Colour inkBrown     (0xff443322);
        const juce::Colour scaleNumberColour (0xffe9dfc4);
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

            const auto tickFont = juce::Font (juce::FontOptions (juce::jmax (8.0f, outerRadius * 0.16f))).boldened();
            const auto tickArea = juce::Rectangle<float> (20.0f, 12.0f).withCentre ({ tx, ty });
            g.setFont (tickFont);
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.drawText (juce::String (i + 1), tickArea.translated (0.0f, 1.0f), juce::Justification::centred);
            g.setColour (scaleNumberColour);
            g.drawText (juce::String (i + 1), tickArea, juce::Justification::centred);
        }

        // Drop shadow, grounding the real knob photo onto the panel.
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillEllipse (bounds.translated (0.0f, 2.0f));

        // Real photographed "chicken-head" knob, picked from a set of
        // pre-rotated whole-image frames (see UIAssets.h). Filled via
        // setFillType()+fillRect() rather than drawImage(): on this
        // renderer, Graphics::drawImage() washes images out to translucent
        // whenever it runs inside a child component's own paint() (which
        // this LookAndFeel callback always does, being called from
        // Slider::paint()) - confirmed by the exact same draw call
        // rendering solid black at the top-level editor's paint() and
        // washed out everywhere else. The image-as-FillType technique
        // already used for the cream panel texture doesn't have that
        // problem, so every image draw in this file uses it instead.
        const int frameIndex = juce::jlimit (0, UIAssets::numKnobFrames - 1,
            (int) std::lround (sliderPos * (float) (UIAssets::numKnobFrames - 1)));
        const auto& knobFrame = UIAssets::getKnobFrame (frameIndex);
        if (knobFrame.isValid())
        {
            const auto scale = bounds.getWidth() / (float) knobFrame.getWidth();
            g.setFillType (juce::FillType (knobFrame, juce::AffineTransform::scale (scale)
                                                            .translated (bounds.getX(), bounds.getY())));
            g.fillRect (bounds);
            g.setFillType (juce::FillType (juce::Colours::black));
        }
    }

    void AuraLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool /*highlighted*/, bool /*down*/)
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto ledDiameter = juce::jmin (bounds.getHeight() - 2.0f, bounds.getWidth() - 2.0f);
        const auto ledBounds = juce::Rectangle<float> (ledDiameter, ledDiameter).withCentre (bounds.getCentre());

        // Bypass semantics: toggled ON == bypassed == signal NOT flowing
        // through this module, so the jewel glows red when on, green when
        // the module is actively processing. Real photographed jewel-light
        // assets (chrome bezel baked in) - see the knob comment in
        // drawRotarySlider for why this uses setFillType()+fillEllipse()
        // rather than drawImage().
        const bool bypassed = button.getToggleState();
        const auto& jewelImage = bypassed ? UIAssets::getJewelRed() : UIAssets::getJewelGreen();
        if (jewelImage.isValid())
        {
            const auto scale = ledBounds.getWidth() / (float) jewelImage.getWidth();
            g.setFillType (juce::FillType (jewelImage, juce::AffineTransform::scale (scale)
                                                            .translated (ledBounds.getX(), ledBounds.getY())));
            g.fillEllipse (ledBounds);
            g.setFillType (juce::FillType (juce::Colours::black));
        }
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
