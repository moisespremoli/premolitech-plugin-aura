#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/SignalChain.h"
#include "DSP/Cab/CabModule.h"
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

        // Post-output-gain peak level, updated every block. Read by the
        // editor's VU meter component - genuinely driven by the audio, not
        // decorative.
        float getOutputLevel() const { return outputLevel.load (std::memory_order_relaxed); }

        // Loads a user-chosen IR into the cabinet module and remembers its
        // path in the plugin state so presets/session restore reload it.
        // Returns false if the file couldn't be read as audio.
        bool loadCabinetIRFromFile (const juce::File& file);
        juce::String getCabinetIRName() const;

    private:
        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void doProcessBlock (juce::AudioBuffer<double>& buffer);

        SignalChain signalChain;
        CabModule* cabModule = nullptr; // owned by signalChain; borrowed for IR loading/state restore
        std::atomic<float> outputLevel { 0.0f };

        // processBlock(float) bridges through this when the host (or the
        // Standalone wrapper) hasn't opted into double-precision processing.
        juce::AudioBuffer<double> doubleBridgeBuffer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuraAudioProcessor)
    };
}
