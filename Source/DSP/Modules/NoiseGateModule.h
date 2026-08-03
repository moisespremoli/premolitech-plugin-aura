#pragma once

#include "../DSPModule.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // Threshold-based noise gate, first block in the chain. Wraps
    // juce::dsp::NoiseGate rather than reinventing envelope-follower math.
    class NoiseGateModule : public DSPModule
    {
    public:
        explicit NoiseGateModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "Noise Gate"; }
        bool isBypassed() const override;

    private:
        void updateParameters();

        juce::dsp::NoiseGate<double> gate;
    };
}
