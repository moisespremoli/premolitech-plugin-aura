#pragma once

#include "../DSPModule.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // Final safety limiter before Output - catches anything the amp/EQ
    // stages sent over the ceiling so the plugin never clips the host.
    class LimiterModule : public DSPModule
    {
    public:
        explicit LimiterModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "Limiter"; }
        bool isBypassed() const override;

    private:
        juce::dsp::Limiter<double> limiter;
    };
}
