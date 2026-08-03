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

        // Fast attack, slower release so the ladder reads like an analogue
        // meter's ballistics rather than jittering sample-to-sample.
        if (level > displayLevel)
            displayLevel = level;
        else
            displayLevel = displayLevel * 0.85f + level * 0.15f;

        if (level >= peakHold)
        {
            peakHold = level;
            peakHoldCounter = 0;
        }
        else if (++peakHoldCounter > 45) // ~1.5s hold at 30Hz before it decays
        {
            peakHold *= 0.95f;
        }

        repaint();
    }

    void VUMeterComponent::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff0a0a0c));
        g.fillRoundedRectangle (bounds, 3.0f);
        g.setColour (juce::Colour (0xff333338));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

        constexpr int numLeds = 12;
        auto ledArea = bounds.reduced (3.0f);
        const auto ledHeight = (ledArea.getHeight() - (float) (numLeds - 1) * 2.0f) / (float) numLeds;

        const auto levelDb = juce::Decibels::gainToDecibels (juce::jmax (displayLevel, 1.0e-5f));
        const auto peakDb = juce::Decibels::gainToDecibels (juce::jmax (peakHold, 1.0e-5f));

        for (int i = 0; i < numLeds; ++i)
        {
            // LED 0 (bottom) = -30dB ... LED (numLeds-1) (top) = 0dB.
            const auto ledDb = juce::jmap ((float) i, 0.0f, (float) (numLeds - 1), -30.0f, 0.0f);
            const bool lit = levelDb >= ledDb;
            const bool isPeakMarker = std::abs (peakDb - ledDb) < 1.4f;

            const auto colour = ledDb > -3.0f  ? juce::Colour (0xffe0473f)
                               : ledDb > -12.0f ? juce::Colour (0xffe8a33d)
                                                 : juce::Colour (0xff4caf50);

            const auto ledY = ledArea.getBottom() - (float) (i + 1) * (ledHeight + 2.0f) + 2.0f;
            juce::Rectangle<float> led (ledArea.getX(), ledY, ledArea.getWidth(), ledHeight);

            if (lit || isPeakMarker)
            {
                g.setColour (colour);
                g.fillRoundedRectangle (led, 1.5f);
                g.setColour (colour.withAlpha (0.30f));
                g.fillRoundedRectangle (led.expanded (1.0f), 1.5f);
            }
            else
            {
                g.setColour (colour.withAlpha (0.10f));
                g.fillRoundedRectangle (led, 1.5f);
            }
        }
    }
}
