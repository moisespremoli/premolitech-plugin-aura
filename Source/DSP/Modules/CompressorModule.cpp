#include "CompressorModule.h"

namespace aura
{
    CompressorModule::CompressorModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void CompressorModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        compressor.prepare (spec);
        makeupGain.prepare (spec);
        reset();
    }

    void CompressorModule::updateParameters()
    {
        compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::compThreshold)->load());
        compressor.setRatio (apvts.getRawParameterValue (ParamIDs::compRatio)->load());
        compressor.setAttack (apvts.getRawParameterValue (ParamIDs::compAttack)->load());
        compressor.setRelease (apvts.getRawParameterValue (ParamIDs::compRelease)->load());
        makeupGain.setGainDecibels (apvts.getRawParameterValue (ParamIDs::compMakeup)->load());
    }

    void CompressorModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        updateParameters();
        compressor.process (context);
        makeupGain.process (context);
    }

    void CompressorModule::reset()
    {
        compressor.reset();
        makeupGain.reset();
    }

    bool CompressorModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::compBypass)->load() > 0.5f;
    }
}
