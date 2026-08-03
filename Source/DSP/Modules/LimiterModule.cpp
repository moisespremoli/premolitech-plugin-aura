#include "LimiterModule.h"

namespace aura
{
    LimiterModule::LimiterModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void LimiterModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        limiter.prepare (spec);
        reset();
    }

    void LimiterModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        limiter.setThreshold (apvts.getRawParameterValue (ParamIDs::limiterCeiling)->load());
        limiter.process (context);
    }

    void LimiterModule::reset()
    {
        limiter.reset();
    }

    bool LimiterModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::limiterBypass)->load() > 0.5f;
    }
}
