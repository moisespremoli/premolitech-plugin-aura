#pragma once

#include "../DSPModule.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // Cabinet simulation via impulse-response convolution. This is the
    // engine only - the drag & drop file browser, IR library management
    // (tags/folders/favourites), and IR-A/IR-B blending described in the
    // spec are Phase 3/6 GUI work that sits on top of loadImpulseResponse().
    class CabModule : public DSPModule
    {
    public:
        explicit CabModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "Cab"; }
        bool isBypassed() const override;

        // Loads a WAV/AIFF file (mono or stereo, any sample rate - JUCE's
        // Convolution resamples internally to match the current session
        // rate). Returns false if the file couldn't be read as audio.
        bool loadImpulseResponse (const juce::File& file);

        // Loads the small placeholder IR bundled as BinaryData, so the
        // convolution path is exercised end-to-end even before a real
        // cabinet IR library is wired up.
        void loadPlaceholderImpulseResponse();

    private:
        // juce::dsp::Convolution is float-only internally (its FFT engine
        // doesn't have a double-precision path), while the rest of the
        // chain runs in double. This module is the bridge: it converts to
        // float, convolves, and converts back, so the double-precision
        // guarantee described in the spec holds everywhere else.
        juce::dsp::Convolution convolution;
        juce::dsp::DryWetMixer<double> mixer;
        juce::dsp::ProcessSpec preparedSpec {};
        juce::AudioBuffer<float> floatScratch;
    };
}
