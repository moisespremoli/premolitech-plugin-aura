#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>

namespace aura
{
    // A real-time analogue-style VU meter (chrome frame, cream face, printed
    // dB scale, swinging needle) - polls a level provider on a Timer, so the
    // needle position genuinely reflects the plugin's live output level
    // rather than a decorative animation.
    class VUMeterComponent : public juce::Component, private juce::Timer
    {
    public:
        explicit VUMeterComponent (std::function<float()> levelProviderToUse);

        void paint (juce::Graphics& g) override;

    private:
        void timerCallback() override;

        std::function<float()> levelProvider;
        float displayLevel = 0.0f;
        float needleAngle = 0.0f;
    };
}
