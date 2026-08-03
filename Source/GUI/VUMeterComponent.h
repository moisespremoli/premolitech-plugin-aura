#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>

namespace aura
{
    // A real-time LED-ladder output meter (green/amber/red segments, with
    // peak hold) - polls a level provider on a Timer, so what it shows is
    // genuinely the plugin's live output level, not a decorative animation.
    class VUMeterComponent : public juce::Component, private juce::Timer
    {
    public:
        explicit VUMeterComponent (std::function<float()> levelProviderToUse);

        void paint (juce::Graphics& g) override;

    private:
        void timerCallback() override;

        std::function<float()> levelProvider;
        float displayLevel = 0.0f;
        float peakHold = 0.0f;
        int peakHoldCounter = 0;
    };
}
