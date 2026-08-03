#pragma once

#include "AmpModelBase.h"

namespace aura
{
    // Modern category reference: a 5150-style high-gain amp. Same "voiced
    // simulation" approach and caveats as JCM800Model - see that header -
    // but pushed further: harder-biased asymmetric clipping on both cascaded
    // stages, a tighter low end (higher bass-shelf corner so the low string
    // stays articulate under heavy gain), a deeper mid scoop, and a higher,
    // more aggressive presence region.
    class Peavey5150Model : public AmpModelBase
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

        juce::String getModelName() const override { return "5150"; }

    private:
        void updateToneStack();

        double sampleRate = 44100.0;

        float gainParam = 0.5f, bassParam = 0.5f, midParam = 0.5f;
        float trebleParam = 0.5f, presenceParam = 0.5f, masterParam = 0.5f;

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
