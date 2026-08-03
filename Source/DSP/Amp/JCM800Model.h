#pragma once

#include "AmpModelBase.h"

namespace aura
{
    // Reference amp model for the first release: a Marshall JCM800-style
    // "modern high-gain" preamp + power amp.
    //
    // Honesty note: this is a *voiced* simulation (cascaded gain stages into
    // asymmetric waveshapers, feeding a 3-band tone stack tuned to the
    // JCM800's characteristic mid scoop, then a softer power-amp saturation
    // stage with a presence shelf ahead of it) - not a literal SPICE/WDF
    // solve of the real passive tone-stack network. A circuit-accurate model
    // (like the Pultec WDF work) is a research effort in itself and is
    // intentionally out of scope for this first model; the architecture
    // (AmpModelBase/AmpFactory) is what lets a more rigorous model replace
    // this one later without touching anything else in the plugin.
    class JCM800Model : public AmpModelBase
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

        juce::String getModelName() const override { return "JCM800"; }

    private:
        void updateToneStack();

        double sampleRate = 44100.0;

        float gainParam = 0.5f, bassParam = 0.5f, midParam = 0.5f;
        float trebleParam = 0.5f, presenceParam = 0.5f, masterParam = 0.5f;

        // 0: drive gain into stage 1, 1: preamp shaper 1, 2: preamp shaper 2,
        // 3: bass shelf, 4: mid peak (scoop), 5: treble shelf,
        // 6: presence shelf (pre power stage), 7: power-amp shaper,
        // 8: master volume.
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
