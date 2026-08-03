#include "VUMeterComponent.h"
#include "UIAssets.h"

namespace aura
{
    namespace
    {
        // dB tick marks for a classic VU scale, mapped through the same
        // linear transfer function as the needle (see timerCallback) so a
        // printed number lines up exactly with where the needle points at
        // that level. "hot" marks the red zone (0 dB and above). At this
        // component's small on-screen size the marks from -7 up to +3 sit
        // only a few degrees apart, so only a sparse, well-spaced subset
        // gets a printed number (showLabel) - the rest still get a tick
        // line, just no text, to avoid the numbers overlapping into an
        // unreadable blob.
        struct DbTick { float db; const char* label; bool hot; bool showLabel; };
        constexpr DbTick dbTicks[] = {
            { -20.0f, "20", false, true },  { -10.0f, "10", false, true },
            { -7.0f, "7", false, false },   { -5.0f, "5", false, true },
            { -3.0f, "3", false, false },   { -2.0f, "2", false, false },
            { -1.0f, "1", false, false },   { 0.0f, "0", true, true },
            { 3.0f, "3", true, false },
        };

        float dbToAngle (float db)
        {
            return juce::jmap (db, -30.0f, 3.0f, -0.78f, 0.78f);
        }
    }

    VUMeterComponent::VUMeterComponent (std::function<float()> levelProviderToUse)
        : levelProvider (std::move (levelProviderToUse))
    {
        juce::Random rng (778899);
        for (int i = 0; i < 5; ++i)
        {
            blotches.push_back ({ rng.nextFloat(), rng.nextFloat() * 0.7f + 0.15f,
                                   rng.nextFloat() * 0.10f + 0.05f, rng.nextFloat() * 0.10f + 0.05f });
        }

        startTimerHz (30);
    }

    void VUMeterComponent::timerCallback()
    {
        const auto level = levelProvider ? levelProvider() : 0.0f;
        const auto levelDb = juce::Decibels::gainToDecibels (juce::jmax (level, 1.0e-5f));
        const auto clampedDb = juce::jlimit (-30.0f, 3.0f, levelDb);
        const auto targetAngle = dbToAngle (clampedDb);

        // Mechanical-needle-like smoothing: fast enough to read transients,
        // slow enough not to look like a jittery digital meter.
        needleAngle += (targetAngle - needleAngle) * 0.25f;

        repaint();
    }

    void VUMeterComponent::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        // Real photographed dark-metal VU bezel (same asset family as the
        // other Premoli Labs plug-ins' VU frame), stretched to the
        // component's bounds - see the knob comment in AuraLookAndFeel for
        // why this uses setFillType()+fillRect() rather than drawImage()
        // inside a child component's paint().
        const auto& frameImage = UIAssets::getVuFrame();
        juce::Rectangle<float> face;
        if (frameImage.isValid())
        {
            const auto sx = bounds.getWidth() / (float) frameImage.getWidth();
            const auto sy = bounds.getHeight() / (float) frameImage.getHeight();
            g.setFillType (juce::FillType (frameImage, juce::AffineTransform::scale (sx, sy)));
            g.fillRect (bounds);
            g.setFillType (juce::FillType (juce::Colours::black));

            // Inner viewing-hole fractions measured directly from the
            // source frame image (left 8.76% / top 15.50% / right 8.53% /
            // bottom 15.12% of its own bounds), so our own drawn face sits
            // exactly inside the frame's cutout instead of guessing insets.
            face = bounds.withTrimmedLeft (bounds.getWidth() * 0.0876f)
                         .withTrimmedTop (bounds.getHeight() * 0.1550f)
                         .withTrimmedRight (bounds.getWidth() * 0.0853f)
                         .withTrimmedBottom (bounds.getHeight() * 0.1512f);
        }
        else
        {
            juce::ColourGradient frameGradient (juce::Colour (0xffe8e4d8), bounds.getX(), bounds.getY(),
                                                 juce::Colour (0xff5a564c), bounds.getRight(), bounds.getBottom(), false);
            g.setGradientFill (frameGradient);
            g.fillRoundedRectangle (bounds, 6.0f);
            face = bounds.reduced (5.0f);
        }

        // Warm cream VU face - matches the "VU LEVEL INDICATOR" faces used
        // across the other Premoli Labs plug-ins - drawn procedurally
        // inside the real frame's cutout so the live needle can animate.
        juce::ColourGradient faceGradient (juce::Colour (0xfff2e9cf), face.getCentreX(), face.getCentreY() - face.getHeight() * 0.15f,
                                            juce::Colour (0xffcfc09a), face.getX(), face.getY(), true);
        g.setGradientFill (faceGradient);
        g.fillRect (face);

        // A few small fixed foxing/grime blotches - own design detail, not
        // copied from any reference art - so the face doesn't read as a
        // flat digital gradient.
        for (auto& b : blotches)
        {
            juce::Rectangle<float> spot (face.getWidth() * b.r * 2.0f, face.getWidth() * b.r * 2.0f);
            spot.setCentre ({ face.getX() + face.getWidth() * b.x, face.getY() + face.getHeight() * b.y });
            g.setColour (juce::Colour (0xff6b5a3a).withAlpha (b.alpha));
            g.fillEllipse (spot);
        }

        g.setColour (juce::Colour (0xff2a2018));
        g.drawRect (face, 1.2f);

        auto labelArea = face.removeFromBottom (13.0f);
        const auto pivot = juce::Point<float> (face.getCentreX(), face.getBottom() - 2.0f);
        const auto arcRadius = juce::jmin (face.getWidth() * 0.46f, face.getHeight() * 0.88f);

        // Scale arc, spanning a touch wider than the needle's own travel so
        // real needle deflections never quite pin the end stops.
        g.setColour (juce::Colour (0xff2a2018));
        juce::Path scaleArc;
        scaleArc.addArc (pivot.x - arcRadius, pivot.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                          -0.86f, 0.86f, true);
        g.strokePath (scaleArc, juce::PathStrokeType (1.3f));

        // Red "hot" zone from 0 dB to the top of the scale, like a real VU.
        {
            juce::Path hotArc;
            hotArc.addArc (pivot.x - arcRadius, pivot.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                            dbToAngle (0.0f), dbToAngle (3.0f), true);
            g.setColour (juce::Colour (0xffb02418));
            g.strokePath (hotArc, juce::PathStrokeType (2.0f));
        }

        const auto tickFont = juce::Font (juce::FontOptions (juce::jmax (7.0f, arcRadius * 0.16f))).boldened();
        g.setFont (tickFont);
        for (auto& tick : dbTicks)
        {
            const auto a = dbToAngle (tick.db);
            const auto inner = pivot.getPointOnCircumference (arcRadius - 4.0f, a);
            const auto outer = pivot.getPointOnCircumference (arcRadius + 1.5f, a);
            g.setColour (tick.hot ? juce::Colour (0xffb02418) : juce::Colour (0xff2a2018));
            g.drawLine ({ inner, outer }, tick.hot ? 1.6f : 1.1f);

            if (tick.showLabel)
            {
                const auto labelPos = pivot.getPointOnCircumference (arcRadius - 13.0f, a);
                g.drawText (tick.label, juce::Rectangle<float> (12.0f, 10.0f).withCentre (labelPos),
                            juce::Justification::centred);
            }
        }

        g.setFont (juce::Font (juce::FontOptions (9.5f)).italicised().boldened());
        g.setColour (juce::Colour (0xff2a2018));
        g.drawText ("VU", labelArea, juce::Justification::centred);

        // Tapered filled needle (wide at the pivot, narrow at the tip)
        // rather than a plain stroked line, for a slightly more mechanical,
        // less "digital vector" look.
        const auto needleLength = arcRadius * 0.92f;
        constexpr float needleBaseWidth = 2.6f;
        juce::Path needle;
        needle.startNewSubPath (pivot.x - needleBaseWidth * 0.5f, pivot.y);
        needle.lineTo (pivot.x + needleBaseWidth * 0.5f, pivot.y);
        needle.lineTo (pivot.x, pivot.y - needleLength);
        needle.closeSubPath();
        g.setColour (juce::Colour (0xffcc1111));
        g.fillPath (needle, juce::AffineTransform::rotation (needleAngle, pivot.x, pivot.y));

        juce::ColourGradient capGradient (juce::Colour (0xff777777), pivot.x - 3.0f, pivot.y - 3.0f,
                                           juce::Colour (0xff1a1a1a), pivot.x + 3.0f, pivot.y + 3.0f, false);
        g.setGradientFill (capGradient);
        g.fillEllipse (juce::Rectangle<float> (7.0f, 7.0f).withCentre (pivot));
    }
}
