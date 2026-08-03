#pragma once

#include "../DSPModule.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // Standard feed-forward compressor + makeup gain, sitting before the amp
    // so pedal-style dynamics shaping happens ahead of the drive stage.
    class CompressorModule : public DSPModule
    {
    public:
        explicit CompressorModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "Compressor"; }
        bool isBypassed() const override;

    private:
        void updateParameters();

        juce::dsp::Compressor<double> compressor;
        juce::dsp::Gain<double> makeupGain;
    };
}
