#include "JCM800Model.h"
#include "AmpFactory.h"

namespace aura
{
    void JCM800Model::prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        chain.prepare (spec);

        // Preamp stage 1: slight DC bias before the tanh so positive and
        // negative half-cycles clip asymmetrically (even-harmonic content,
        // the "tube-like" character), then the bias is subtracted back out
        // so the stage doesn't add a DC offset downstream.
        chain.get<1>().functionToUse = [] (double x)
        {
            constexpr double bias = 0.15;
            return std::tanh (x + bias) - std::tanh (bias);
        };

        // Preamp stage 2: a second, more symmetric gain stage - this is what
        // gives a cascaded-gain-stage amp (JCM800 high-gain channel) its
        // extra sustain compared to a single clipping stage.
        chain.get<2>().functionToUse = [] (double x)
        {
            return std::tanh (x * 1.5);
        };

        // Power-amp stage: gentler cubic soft-clip, simulating power-tube
        // compression rather than the harder preamp distortion.
        chain.get<7>().functionToUse = [] (double x)
        {
            x = juce::jlimit (-1.5, 1.5, x);
            return x - (x * x * x) / 6.0;
        };

        updateToneStack();
        reset();
    }

    void JCM800Model::reset()
    {
        chain.reset();
    }

    void JCM800Model::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        chain.process (context);
    }

    void JCM800Model::setGain (float normalised01)
    {
        gainParam = normalised01;
        // Drive into the first clipping stage: unity up to ~25x, so the
        // amp cleans up nicely at low settings and screams at full gain.
        chain.get<0>().setGainLinear (1.0f + gainParam * 24.0f);
    }

    void JCM800Model::setBass (float normalised01)      { bassParam = normalised01;     updateToneStack(); }
    void JCM800Model::setMid (float normalised01)       { midParam = normalised01;      updateToneStack(); }
    void JCM800Model::setTreble (float normalised01)    { trebleParam = normalised01;   updateToneStack(); }
    void JCM800Model::setPresence (float normalised01)  { presenceParam = normalised01; updateToneStack(); }

    void JCM800Model::setMaster (float normalised01)
    {
        masterParam = normalised01;
        chain.get<8>().setGainDecibels (juce::jmap (masterParam, 0.0f, 1.0f, -24.0f, 6.0f));
    }

    void JCM800Model::updateToneStack()
    {
        // Frequency centres and ranges chosen to land in the same ballpark
        // as a real Marshall-style passive tone stack (deep mid scoop
        // available, bass/treble as broad shelves either side of it) -
        // tuned by ear against reference JCM800 recordings, not derived
        // from a component-level circuit solve.
        const auto bassDb     = juce::jmap (bassParam,     0.0f, 1.0f, -12.0f, 12.0f);
        const auto midDb      = juce::jmap (midParam,      0.0f, 1.0f, -15.0f,  6.0f);
        const auto trebleDb   = juce::jmap (trebleParam,   0.0f, 1.0f, -12.0f, 12.0f);
        const auto presenceDb = juce::jmap (presenceParam,  0.0f, 1.0f,   0.0f, 12.0f);

        chain.get<3>().coefficients = juce::dsp::IIR::Coefficients<double>::makeLowShelf (
            sampleRate, 120.0, 0.707, juce::Decibels::decibelsToGain ((double) bassDb));

        chain.get<4>().coefficients = juce::dsp::IIR::Coefficients<double>::makePeakFilter (
            sampleRate, 500.0, 0.7, juce::Decibels::decibelsToGain ((double) midDb));

        chain.get<5>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 3000.0, 0.707, juce::Decibels::decibelsToGain ((double) trebleDb));

        chain.get<6>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 5000.0, 0.707, juce::Decibels::decibelsToGain ((double) presenceDb));
    }

    static AmpModelRegistrar<JCM800Model> registrar ("JCM800");
}
