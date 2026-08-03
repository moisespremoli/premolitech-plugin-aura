#pragma once

#include "../DSPModule.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // Simple 3-band post-cab EQ (low shelf / mid peak / high shelf) - tone
    // shaping after the amp+cab stage, as opposed to the amp's own tone
    // stack which happens before the cab.
    class EQModule : public DSPModule
    {
    public:
        explicit EQModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "EQ"; }
        bool isBypassed() const override;

    private:
        void updateParameters();

        double sampleRate = 44100.0;
        juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<double>,
                                   juce::dsp::IIR::Filter<double>,
                                   juce::dsp::IIR::Filter<double>>
            chain;
    };
}
