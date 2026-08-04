#include "VUMeterComponent.h"

namespace aura
{
    namespace
    {
        constexpr int numSegments = 14;
        constexpr int numGreen    = 9;
        constexpr int numYellow   = 3; // remaining (numSegments - numGreen - numYellow) are red
    }

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
        const auto target = juce::jmap (clampedDb, -30.0f, 3.0f, 0.0f, 1.0f);

        // Fast enough to read transients, slow enough not to flicker.
        displayLevel += (target - displayLevel) * 0.35f;

        repaint();
    }

    void VUMeterComponent::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        // Dark recessed bezel, like a small inset meter window on the panel.
        g.setColour (juce::Colour (0xff1c1a17));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

        auto ladder = bounds.reduced (4.0f);
        constexpr float gap = 2.0f;
        const auto segmentWidth = (ladder.getWidth() - gap * (float) (numSegments - 1)) / (float) numSegments;

        const int litCount = juce::roundToInt (displayLevel * (float) numSegments);

        for (int i = 0; i < numSegments; ++i)
        {
            auto seg = juce::Rectangle<float> (segmentWidth, ladder.getHeight())
                           .withPosition (ladder.getX() + (float) i * (segmentWidth + gap), ladder.getY());

            const bool lit = i < litCount;
            juce::Colour colour = i < numGreen ? juce::Colour (0xff2fbf4f)
                                 : i < numGreen + numYellow ? juce::Colour (0xffe0c020)
                                                             : juce::Colour (0xffd8331e);
            g.setColour (lit ? colour : colour.withAlpha (0.18f));
            g.fillRoundedRectangle (seg, 1.5f);
        }
    }
}
