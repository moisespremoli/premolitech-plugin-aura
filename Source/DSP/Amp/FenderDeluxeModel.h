#pragma once

#include "AmpModelBase.h"

namespace aura
{
    // Vintage category reference: a Fender Deluxe Reverb / blackface-style
    // clean-to-gently-breaking-up amp. Same "voiced simulation" approach and
    // caveats as JCM800Model - see that header for the honesty note - but
    // tuned for a much cleaner, brighter, less compressed character: a
    // single gentle symmetric soft-clip stage instead of two cascaded
    // asymmetric ones, and a tone stack with less mid scoop and a higher,
    // glassier treble/presence region.
    class FenderDeluxeModel : public AmpModelBase
    {
    public:
        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void reset() override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;

        void setGain (float normalised01) override;
        void setBass (float normalised01) override;
        void setMid (float normalised01) override;
        void setTreble (float normalised01) override;
        void setPresence (float normalised01) override;
        void setMaster (float normalised01) override;

        juce::String getModelName() const override { return "Fender Deluxe"; }

    private:
        void updateToneStack();

        double sampleRate = 44100.0;

        float gainParam = 0.5f, bassParam = 0.5f, midParam = 0.5f;
        float trebleParam = 0.5f, presenceParam = 0.5f, masterParam = 0.5f;

        // Same 9-stage layout as JCM800Model for consistency, but stage 2
        // (index 2) is left near-unity since a clean Fender-style amp
        // doesn't cascade two hard gain stages.
        juce::dsp::ProcessorChain<juce::dsp::Gain<double>,
                                   juce::dsp::WaveShaper<double>,
                                   juce::dsp::WaveShaper<double>,
                                   juce::dsp::IIR::Filter<double>,
                                   juce::dsp::IIR::Filter<double>,
                                   juce::dsp::IIR::Filter<double>,
                                   juce::dsp::IIR::Filter<double>,
                                   juce::dsp::WaveShaper<double>,
                                   juce::dsp::Gain<double>>
            chain;
    };
}
