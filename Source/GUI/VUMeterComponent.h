#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>

namespace aura
{
    // A simple, easy-to-read LED ladder meter: a row of segments that light
    // up green -> yellow -> red with the plugin's live output level, like a
    // classic hardware peak meter. Deliberately simpler than an analogue
    // needle gauge - no scale to read, just how many segments are lit.
    // Polls a level provider on a Timer so it reflects real audio.
    class VUMeterComponent : public juce::Component, private juce::Timer
    {
    public:
        explicit VUMeterComponent (std::function<float()> levelProviderToUse);

        void paint (juce::Graphics& g) override;

    private:
        void timerCallback() override;

        std::function<float()> levelProvider;
        float displayLevel = 0.0f; // smoothed 0-1 fraction of the segment ladder lit
    };
}
