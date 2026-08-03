#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace aura
{
    // Common interface every processing block in the signal chain implements
    // (gate, compressor, amp, cab, EQ, limiter, and any future module). The
    // SignalChain only ever talks to modules through this interface, so new
    // modules can be added without touching the engine.
    class DSPModule
    {
    public:
        explicit DSPModule (juce::AudioProcessorValueTreeState& stateToUse) : apvts (stateToUse) {}
        virtual ~DSPModule() = default;

        virtual void prepare (const juce::dsp::ProcessSpec& spec) = 0;
        virtual void process (juce::dsp::ProcessContextReplacing<double>& context) = 0;
        virtual void reset() = 0;

        // Human-readable name for GUI/preset display purposes.
        virtual juce::String getName() const = 0;

        // Whether this instance currently passes audio through unmodified.
        // Modules are expected to back this with their own bypass parameter
        // and honour it at the top of process().
        virtual bool isBypassed() const = 0;

    protected:
        juce::AudioProcessorValueTreeState& apvts;
    };
}
