#pragma once

#include <juce_core/juce_core.h>

namespace aura
{
    enum class Instrument
    {
        ElectricGuitar = 0,
        Bass,
        AcousticGuitar
    };

    inline juce::StringArray getInstrumentChoices()
    {
        return { "Electric Guitar", "Bass", "Acoustic Guitar" };
    }

    inline Instrument instrumentFromIndex (int index)
    {
        return static_cast<Instrument> (juce::jlimit (0, 2, index));
    }
}
