#pragma once

#include <juce_dsp/juce_dsp.h>

namespace aura
{
    // Every amplifier model (vintage, modern, boutique, bass, acoustic
    // preamp...) implements this. The chain around it (gate, comp, cab, EQ)
    // never needs to know which specific amp is loaded - only AmpFactory and
    // AmpModule do. Adding a new amp means adding one new subclass and one
    // registration line; nothing else in the plugin changes.
    class AmpModelBase
    {
    public:
        virtual ~AmpModelBase() = default;

        virtual void prepare (const juce::dsp::ProcessSpec& spec) = 0;
        virtual void reset() = 0;
        virtual void process (juce::dsp::ProcessContextReplacing<double>& context) = 0;

        // Common control set every amp model exposes. A given model is free
        // to reinterpret a control (e.g. an acoustic preamp might use
        // "presence" as a notch-filter depth instead of a treble shelf) but
        // the surface stays consistent so the GUI doesn't need per-model
        // layouts.
        virtual void setGain (float normalised01) = 0;
        virtual void setBass (float normalised01) = 0;
        virtual void setMid (float normalised01) = 0;
        virtual void setTreble (float normalised01) = 0;
        virtual void setPresence (float normalised01) = 0;
        virtual void setMaster (float normalised01) = 0;

        virtual juce::String getModelName() const = 0;
    };
}
