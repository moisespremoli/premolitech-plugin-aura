#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>

namespace aura
{
    // A real-time analogue-style VU meter of our own design (chrome frame,
    // aged parchment face with a classic -20..+3 dB scale, red "hot" zone,
    // printed markings, swinging needle) - polls a level provider on a
    // Timer, so the needle position genuinely reflects the plugin's live
    // output level rather than a decorative animation.
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

        // A handful of fixed, pre-randomised grime/foxing blotches on the
        // face, in (0-1, 0-1, radius-fraction, alpha) unit-space so they
        // scale with the component - generated once so they stay put
        // across the 30Hz repaint instead of shimmering.
        struct Blotch { float x, y, r, alpha; };
        std::vector<Blotch> blotches;
    };
}
