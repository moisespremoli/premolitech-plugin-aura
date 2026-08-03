#include "Peavey5150Model.h"
#include "AmpFactory.h"

namespace aura
{
    void Peavey5150Model::prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        chain.prepare (spec);

        // Stage 1: strongly DC-biased tanh - more bias than the JCM800 model,
        // pushing further into asymmetric (even-harmonic-heavy) clipping.
        chain.get<1>().functionToUse = [] (double x)
        {
            constexpr double bias = 0.28;
            return std::tanh (x + bias) - std::tanh (bias);
        };

        // Stage 2: a harder, near-square clip - the second cascaded gain
        // stage that gives high-gain modern amps their compressed, sustained
        // character rather than a JCM800's looser cascade.
        chain.get<2>().functionToUse = [] (double x)
        {
            return std::tanh (x * 2.2);
        };

        // Power-amp stage: firmer cubic soft-clip than the JCM800's, since
        // modern high-gain amps run their power section closer to its own
        // ceiling too.
        chain.get<7>().functionToUse = [] (double x)
        {
            x = juce::jlimit (-1.2, 1.2, x);
            return x - (x * x * x) / 5.0;
        };

        updateToneStack();
        reset();
    }

    void Peavey5150Model::reset()
    {
        chain.reset();
    }

    void Peavey5150Model::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        chain.process (context);
    }

    void Peavey5150Model::setGain (float normalised01)
    {
        gainParam = normalised01;
        // Highest drive ceiling of the three reference models - this is the
        // "screams at full gain" high-gain amp.
        chain.get<0>().setGainLinear (1.0f + gainParam * 40.0f);
    }

    void Peavey5150Model::setBass (float normalised01)      { bassParam = normalised01;     updateToneStack(); }
    void Peavey5150Model::setMid (float normalised01)       { midParam = normalised01;      updateToneStack(); }
    void Peavey5150Model::setTreble (float normalised01)    { trebleParam = normalised01;   updateToneStack(); }
    void Peavey5150Model::setPresence (float normalised01)  { presenceParam = normalised01; updateToneStack(); }

    void Peavey5150Model::setMaster (float normalised01)
    {
        masterParam = normalised01;
        chain.get<8>().setGainDecibels (juce::jmap (masterParam, 0.0f, 1.0f, -24.0f, 6.0f));
    }

    void Peavey5150Model::updateToneStack()
    {
        // Tighter, higher-corner bass shelf than the JCM800/Fender models so
        // low strings stay articulate under heavy gain, a deeper mid scoop,
        // and a hotter, higher presence region for pick-attack cut-through.
        const auto bassDb     = juce::jmap (bassParam,     0.0f, 1.0f, -10.0f, 10.0f);
        const auto midDb      = juce::jmap (midParam,      0.0f, 1.0f, -18.0f,  4.0f);
        const auto trebleDb   = juce::jmap (trebleParam,   0.0f, 1.0f, -12.0f, 12.0f);
        const auto presenceDb = juce::jmap (presenceParam,  0.0f, 1.0f,   0.0f, 14.0f);

        chain.get<3>().coefficients = juce::dsp::IIR::Coefficients<double>::makeLowShelf (
            sampleRate, 160.0, 0.707, juce::Decibels::decibelsToGain ((double) bassDb));

        chain.get<4>().coefficients = juce::dsp::IIR::Coefficients<double>::makePeakFilter (
            sampleRate, 550.0, 0.65, juce::Decibels::decibelsToGain ((double) midDb));

        chain.get<5>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 3200.0, 0.707, juce::Decibels::decibelsToGain ((double) trebleDb));

        chain.get<6>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 5500.0, 0.707, juce::Decibels::decibelsToGain ((double) presenceDb));
    }

    static AmpModelRegistrar<Peavey5150Model> registrar ("5150");
}
