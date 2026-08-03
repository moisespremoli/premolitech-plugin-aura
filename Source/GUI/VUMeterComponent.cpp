#include "VUMeterComponent.h"

namespace aura
{
    VUMeterComponent::VUMeterComponent (std::function<float()> levelProviderToUse)
        : levelProvider (std::move (levelProviderToUse))
    {
        startTimerHz (30);
    }

    void VUMeterComponent::timerCallback()
    {
        const auto level = levelProvider ? levelProvider() : 0.0f;
        const auto levelDb = juce::Decibels::gainToDecibels (juce::jmax (level, 1.0e-5f));
        const auto clampedDb = juce::jlimit (-30.0f, 3.0f, levelDb);
        const auto targetAngle = juce::jmap (clampedDb, -30.0f, 3.0f, -0.78f, 0.78f);

        // Mechanical-needle-like smoothing: fast enough to read transients,
        // slow enough not to look like a jittery digital meter.
        needleAngle += (targetAngle - needleAngle) * 0.25f;

        repaint();
    }

    void VUMeterComponent::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient frameGradient (juce::Colour (0xfff0f0f0), bounds.getX(), bounds.getY(),
                                             juce::Colour (0xff666666), bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill (frameGradient);
        g.fillRoundedRectangle (bounds, 6.0f);

        auto face = bounds.reduced (5.0f);
        g.setColour (juce::Colour (0xfff4ebd0));
        g.fillRoundedRectangle (face, 3.0f);
        g.setColour (juce::Colour (0xff333333));
        g.drawRoundedRectangle (face, 3.0f, 1.2f);

        auto labelArea = face.removeFromBottom (12.0f);
        const auto pivot = juce::Point<float> (face.getCentreX(), face.getBottom() - 2.0f);
        const auto arcRadius = juce::jmin (face.getWidth() * 0.46f, face.getHeight() * 0.85f);

        g.setColour (juce::Colour (0xff333333));
        juce::Path scaleArc;
        scaleArc.addArc (pivot.x - arcRadius, pivot.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                          -0.9f, 0.9f, true);
        g.strokePath (scaleArc, juce::PathStrokeType (1.3f));

        for (int i = 0; i <= 6; ++i)
        {
            const auto t = (float) i / 6.0f;
            const auto a = juce::jmap (t, 0.0f, 1.0f, -0.9f, 0.9f);
            const auto inner = pivot.getPointOnCircumference (arcRadius - 4.0f, a);
            const auto outer = pivot.getPointOnCircumference (arcRadius + 1.5f, a);
            g.drawLine ({ inner, outer }, 1.1f);
        }

        g.setFont (juce::Font (juce::FontOptions (9.0f)).boldened());
        g.setColour (juce::Colour (0xff333333));
        g.drawText ("VU", labelArea, juce::Justification::centred);

        juce::Path needle;
        const auto needleLength = arcRadius * 0.9f;
        needle.startNewSubPath (pivot);
        needle.lineTo (pivot.x, pivot.y - needleLength);
        g.setColour (juce::Colour (0xffcc1111));
        g.strokePath (needle,
                      juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                      juce::AffineTransform::rotation (needleAngle, pivot.x, pivot.y));

        g.setColour (juce::Colour (0xff222222));
        g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (pivot));
    }
}
