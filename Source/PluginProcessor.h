#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/SignalChain.h"
#include "Core/Instrument.h"

namespace aura
{
    class AuraAudioProcessor : public juce::AudioProcessor
    {
    public:
        AuraAudioProcessor();
        ~AuraAudioProcessor() override = default;

        void prepareToPlay (double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
        void processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) override;
        bool supportsDoublePrecisionProcessing() const override { return true; }

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return JucePlugin_Name; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const juce::String getProgramName (int) override { return {}; }
        void changeProgramName (int, const juce::String&) override {}

        void getStateInformation (juce::MemoryBlock& destData) override;
        void setStateInformation (const void* data, int sizeInBytes) override;

        juce::AudioProcessorValueTreeState apvts;

    private:
        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void doProcessBlock (juce::AudioBuffer<double>& buffer);

        SignalChain signalChain;

        // processBlock(float) bridges through this when the host (or the
        // Standalone wrapper) hasn't opted into double-precision processing.
        juce::AudioBuffer<double> doubleBridgeBuffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuraAudioProcessor)
    };
}
