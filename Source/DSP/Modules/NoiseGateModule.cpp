#include "NoiseGateModule.h"

namespace aura
{
    NoiseGateModule::NoiseGateModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void NoiseGateModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        gate.prepare (spec);
        reset();
    }

    void NoiseGateModule::updateParameters()
    {
        gate.setThreshold (apvts.getRawParameterValue (ParamIDs::gateThreshold)->load());
        gate.setAttack (apvts.getRawParameterValue (ParamIDs::gateAttack)->load());
        gate.setRelease (apvts.getRawParameterValue (ParamIDs::gateRelease)->load());
        gate.setRatio (10.0f); // fixed hard-knee ratio; not exposed as a parameter yet
    }

    void NoiseGateModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        updateParameters();
        gate.process (context);
    }

    void NoiseGateModule::reset()
    {
        gate.reset();
    }

    bool NoiseGateModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::gateBypass)->load() > 0.5f;
    }
}
