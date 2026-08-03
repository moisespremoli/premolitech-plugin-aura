#include "FenderDeluxeModel.h"
#include "AmpFactory.h"

namespace aura
{
    void FenderDeluxeModel::prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        chain.prepare (spec);

        // Single gentle symmetric soft-clip stage - clean until pushed hard,
        // no DC bias (no even-harmonic emphasis the way a biased tube stage
        // has), which is what keeps a blackface-style amp sounding "clean
        // and glassy" rather than "warm and compressed".
        chain.get<1>().functionToUse = [] (double x)
        {
            return std::tanh (x * 0.8);
        };

        // Second stage stays near-unity: a clean amp doesn't cascade two
        // hard-clipping gain stages the way a high-gain amp does.
        chain.get<2>().functionToUse = [] (double x)
        {
            return std::tanh (x * 1.05);
        };

        // Power-amp stage: very gentle cubic soft-clip, only audible once
        // the master is pushed hard (tube power-amp "sag" at high volume).
        chain.get<7>().functionToUse = [] (double x)
        {
            x = juce::jlimit (-2.0, 2.0, x);
            return x - (x * x * x) / 12.0;
        };

        updateToneStack();
        reset();
    }

    void FenderDeluxeModel::reset()
    {
        chain.reset();
    }

    void FenderDeluxeModel::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        chain.process (context);
    }

    void FenderDeluxeModel::setGain (float normalised01)
    {
        gainParam = normalised01;
        // Much lower drive ceiling than the JCM800 - a clean amp shouldn't
        // scream even at full gain, just break up softly.
        chain.get<0>().setGainLinear (1.0f + gainParam * 8.0f);
    }

    void FenderDeluxeModel::setBass (float normalised01)      { bassParam = normalised01;     updateToneStack(); }
    void FenderDeluxeModel::setMid (float normalised01)       { midParam = normalised01;      updateToneStack(); }
    void FenderDeluxeModel::setTreble (float normalised01)    { trebleParam = normalised01;   updateToneStack(); }
    void FenderDeluxeModel::setPresence (float normalised01)  { presenceParam = normalised01; updateToneStack(); }

    void FenderDeluxeModel::setMaster (float normalised01)
    {
        masterParam = normalised01;
        chain.get<8>().setGainDecibels (juce::jmap (masterParam, 0.0f, 1.0f, -24.0f, 6.0f));
    }

    void FenderDeluxeModel::updateToneStack()
    {
        // Blackface-style tone stack: shallower mid scoop than a Marshall,
        // and the treble/presence region sits higher and brighter to give
        // the characteristic "glassy" Fender top end.
        const auto bassDb     = juce::jmap (bassParam,     0.0f, 1.0f, -10.0f, 10.0f);
        const auto midDb      = juce::jmap (midParam,      0.0f, 1.0f,  -8.0f,  8.0f);
        const auto trebleDb   = juce::jmap (trebleParam,   0.0f, 1.0f, -12.0f, 14.0f);
        const auto presenceDb = juce::jmap (presenceParam,  0.0f, 1.0f,   0.0f, 10.0f);

        chain.get<3>().coefficients = juce::dsp::IIR::Coefficients<double>::makeLowShelf (
            sampleRate, 100.0, 0.707, juce::Decibels::decibelsToGain ((double) bassDb));

        chain.get<4>().coefficients = juce::dsp::IIR::Coefficients<double>::makePeakFilter (
            sampleRate, 500.0, 0.7, juce::Decibels::decibelsToGain ((double) midDb));

        chain.get<5>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 3500.0, 0.707, juce::Decibels::decibelsToGain ((double) trebleDb));

        chain.get<6>().coefficients = juce::dsp::IIR::Coefficients<double>::makeHighShelf (
            sampleRate, 6000.0, 0.707, juce::Decibels::decibelsToGain ((double) presenceDb));
    }

    static AmpModelRegistrar<FenderDeluxeModel> registrar ("Fender Deluxe");
}
