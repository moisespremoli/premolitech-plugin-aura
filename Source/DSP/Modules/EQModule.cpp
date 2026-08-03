#include "EQModule.h"

namespace aura
{
    EQModule::EQModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void EQModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        chain.prepare (spec);
        updateParameters();
        reset();
    }

    void EQModule::updateParameters()
    {
        const auto lowDb  = apvts.getRawParameterValue (ParamIDs::eqLow)->load();
        const auto midDb  = apvts.getRawParameterValue (ParamIDs::eqMid)->load();
        const auto highDb = apvts.getRawParameterValue (ParamIDs::eqHigh)->load();

        chain.get<0>().coefficients = juce::dsp::IIR::Coefficients<double>::makeLowShelf (
            sampleRate, 150.0, 0.707, juce::Decibels::decibelsToGain ((double) lowDb));

        chain.get<1>().coefficients = juce::dsp::IIR::Coefficients<double>::makePeakFilter (
            sampleRate, 1000.0, 0.7, juce::Decibels::decibelsToGain ((double) midDb));

        chain.get<2>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 4000.0, 0.707, juce::Decibels::decibelsToGain ((double) highDb));
    }

    void EQModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        updateParameters();
        chain.process (context);
    }

    void EQModule::reset()
    {
        chain.reset();
    }

    bool EQModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::eqBypass)->load() > 0.5f;
    }
}
