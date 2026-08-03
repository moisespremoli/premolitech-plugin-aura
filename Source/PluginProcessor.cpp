#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "DSP/Modules/NoiseGateModule.h"
#include "DSP/Modules/CompressorModule.h"
#include "DSP/Amp/AmpModule.h"
#include "DSP/Cab/CabModule.h"
#include "DSP/Modules/EQModule.h"
#include "DSP/Modules/LimiterModule.h"
#include "DSP/Amp/AmpFactory.h"
#include "DSP/Amp/AmpModelRegistry.h"

namespace aura
{
    namespace
    {
        constexpr auto irPathPropertyName = "cabIRFilePath";
    }

    AuraAudioProcessor::AuraAudioProcessor()
        : AudioProcessor (BusesProperties()
                               .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
        , apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        // Signal chain order, fixed for now (Fase 6 adds drag & drop
        // reordering on top of SignalChain::moveModule - the engine already
        // supports it, the UI just doesn't expose it yet):
        //   Input -> Gate -> Compressor -> Amp -> Cab -> EQ -> Limiter -> Output
        signalChain.addModule (std::make_unique<NoiseGateModule> (apvts));
        signalChain.addModule (std::make_unique<CompressorModule> (apvts));
        signalChain.addModule (std::make_unique<AmpModule> (apvts));

        auto cab = std::make_unique<CabModule> (apvts);
        cabModule = cab.get();
        signalChain.addModule (std::move (cab));

        signalChain.addModule (std::make_unique<EQModule> (apvts));
        signalChain.addModule (std::make_unique<LimiterModule> (apvts));
    }

    juce::AudioProcessorValueTreeState::ParameterLayout AuraAudioProcessor::createParameterLayout()
    {
        // Must run before AmpFactory is queried below - see AmpModelRegistry.h
        // for why the linker needs this explicit nudge.
        forceLinkAllAmpModels();

        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { ParamIDs::instrument, 1 }, "Instrument", getInstrumentChoices(), 0));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::inputGain, 1 }, "Input Gain",
            juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, "dB"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::outputGain, 1 }, "Output Gain",
            juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

        // Noise gate
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::gateBypass, 1 }, "Gate Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::gateThreshold, 1 }, "Gate Threshold",
            juce::NormalisableRange<float> (-80.0f, 0.0f, 0.1f), -50.0f, "dB"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::gateAttack, 1 }, "Gate Attack",
            juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.4f), 1.0f, "ms"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::gateRelease, 1 }, "Gate Release",
            juce::NormalisableRange<float> (5.0f, 1000.0f, 1.0f, 0.4f), 100.0f, "ms"));

        // Compressor
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::compBypass, 1 }, "Comp Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::compThreshold, 1 }, "Comp Threshold",
            juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -20.0f, "dB"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::compRatio, 1 }, "Comp Ratio",
            juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f, ":1"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::compAttack, 1 }, "Comp Attack",
            juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.4f), 5.0f, "ms"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::compRelease, 1 }, "Comp Release",
            juce::NormalisableRange<float> (5.0f, 1000.0f, 1.0f, 0.4f), 80.0f, "ms"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::compMakeup, 1 }, "Comp Makeup",
            juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f, "dB"));

        // Amp
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::ampBypass, 1 }, "Amp Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { ParamIDs::ampModel, 1 }, "Amp Model",
            AmpFactory::getInstance().getAvailableModelIDs(), 0));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampGain, 1 }, "Amp Gain",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampBass, 1 }, "Amp Bass",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampMid, 1 }, "Amp Mid",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampTreble, 1 }, "Amp Treble",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampPresence, 1 }, "Amp Presence",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.3f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::ampMaster, 1 }, "Amp Master",
            juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));

        // Cab
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::cabBypass, 1 }, "Cab Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::cabMix, 1 }, "Cab Mix",
            juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

        // EQ
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::eqBypass, 1 }, "EQ Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::eqLow, 1 }, "EQ Low",
            juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::eqMid, 1 }, "EQ Mid",
            juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::eqHigh, 1 }, "EQ High",
            juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, "dB"));

        // Limiter
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { ParamIDs::limiterBypass, 1 }, "Limiter Bypass", false));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::limiterCeiling, 1 }, "Limiter Ceiling",
            juce::NormalisableRange<float> (-12.0f, 0.0f, 0.05f), -0.3f, "dB"));

        return { params.begin(), params.end() };
    }

    void AuraAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec { sampleRate,
                                       (juce::uint32) samplesPerBlock,
                                       (juce::uint32) getTotalNumOutputChannels() };
        signalChain.prepare (spec);
        doubleBridgeBuffer.setSize ((int) spec.numChannels, samplesPerBlock);
    }

    void AuraAudioProcessor::releaseResources()
    {
        signalChain.reset();
    }

    bool AuraAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
    {
        const auto set = layouts.getMainOutputChannelSet();
        if (set != juce::AudioChannelSet::mono() && set != juce::AudioChannelSet::stereo())
            return false;
        return layouts.getMainInputChannelSet() == set;
    }

    void AuraAudioProcessor::doProcessBlock (juce::AudioBuffer<double>& buffer)
    {
        const auto inputDb  = apvts.getRawParameterValue (ParamIDs::inputGain)->load();
        const auto outputDb = apvts.getRawParameterValue (ParamIDs::outputGain)->load();
        buffer.applyGain (juce::Decibels::decibelsToGain ((double) inputDb));

        juce::dsp::AudioBlock<double> block (buffer);
        juce::dsp::ProcessContextReplacing<double> context (block);
        signalChain.process (context);

        buffer.applyGain (juce::Decibels::decibelsToGain ((double) outputDb));

        double peak = 0.0;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                peak = juce::jmax (peak, std::abs (data[i]));
        }
        outputLevel.store ((float) peak, std::memory_order_relaxed);
    }

    void AuraAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer&)
    {
        juce::ScopedNoDenormals noDenormals;
        doProcessBlock (buffer);
    }

    void AuraAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
    {
        juce::ScopedNoDenormals noDenormals;

        const auto numChannels = buffer.getNumChannels();
        const auto numSamples  = buffer.getNumSamples();
        doubleBridgeBuffer.setSize (numChannels, numSamples, false, false, true);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* src = buffer.getReadPointer (ch);
            auto* dst = doubleBridgeBuffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
                dst[i] = (double) src[i];
        }

        doProcessBlock (doubleBridgeBuffer);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* src = doubleBridgeBuffer.getReadPointer (ch);
            auto* dst = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
                dst[i] = (float) src[i];
        }
    }

    juce::AudioProcessorEditor* AuraAudioProcessor::createEditor()
    {
        return new AuraAudioProcessorEditor (*this);
    }

    void AuraAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
    {
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }

    void AuraAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
    {
        std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
        if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
            return;

        apvts.replaceState (juce::ValueTree::fromXml (*xml));

        // The IR file path is a custom property on the ValueTree (not an
        // APVTS parameter), so replaceState() restores the property but not
        // the actual loaded convolution buffer - reload it explicitly.
        const auto irPath = apvts.state.getProperty (irPathPropertyName).toString();
        if (irPath.isNotEmpty())
            loadCabinetIRFromFile (juce::File (irPath));
    }

    bool AuraAudioProcessor::loadCabinetIRFromFile (const juce::File& file)
    {
        if (cabModule == nullptr || ! cabModule->loadImpulseResponse (file))
            return false;

        apvts.state.setProperty (irPathPropertyName, file.getFullPathName(), nullptr);
        return true;
    }

    juce::String AuraAudioProcessor::getCabinetIRName() const
    {
        return cabModule != nullptr ? cabModule->getIRDisplayName() : juce::String();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new aura::AuraAudioProcessor();
}
