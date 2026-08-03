#include "CabModule.h"
#include "BinaryData.h"

namespace aura
{
    CabModule::CabModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void CabModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        preparedSpec = spec;
        convolution.prepare (spec);
        mixer.prepare (spec);
        mixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
        floatScratch.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
        loadPlaceholderImpulseResponse();
        reset();
    }

    void CabModule::loadPlaceholderImpulseResponse()
    {
        convolution.loadImpulseResponse (
            BinaryData::placeholder_cab_wav,
            (size_t) BinaryData::placeholder_cab_wavSize,
            juce::dsp::Convolution::Stereo::no,
            juce::dsp::Convolution::Trim::no,
            0,
            juce::dsp::Convolution::Normalise::yes);
    }

    bool CabModule::loadImpulseResponse (const juce::File& file)
    {
        if (! file.existsAsFile())
            return false;

        convolution.loadImpulseResponse (file,
                                          juce::dsp::Convolution::Stereo::yes,
                                          juce::dsp::Convolution::Trim::yes,
                                          0,
                                          juce::dsp::Convolution::Normalise::yes);
        return true;
    }

    void CabModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        auto block = context.getOutputBlock();
        const auto numChannels = block.getNumChannels();
        const auto numSamples  = block.getNumSamples();

        mixer.setWetMixProportion (apvts.getRawParameterValue (ParamIDs::cabMix)->load());
        mixer.pushDrySamples (context.getInputBlock());

        // double -> float bridge into the convolution engine.
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* src = block.getChannelPointer (ch);
            auto* dst = floatScratch.getWritePointer ((int) ch);
            for (size_t i = 0; i < numSamples; ++i)
                dst[i] = (float) src[i];
        }

        juce::dsp::AudioBlock<float> floatBlock (floatScratch);
        auto trimmedFloatBlock = floatBlock.getSubBlock (0, numSamples);
        juce::dsp::ProcessContextReplacing<float> floatContext (trimmedFloatBlock);
        convolution.process (floatContext);

        // float -> double back into the main chain.
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* src = floatScratch.getReadPointer ((int) ch);
            auto* dst = block.getChannelPointer (ch);
            for (size_t i = 0; i < numSamples; ++i)
                dst[i] = (double) src[i];
        }

        mixer.mixWetSamples (block);
    }

    void CabModule::reset()
    {
        convolution.reset();
        mixer.reset();
    }

    bool CabModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::cabBypass)->load() > 0.5f;
    }
}
