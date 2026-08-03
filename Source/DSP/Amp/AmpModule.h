#pragma once

#include "../DSPModule.h"
#include "AmpFactory.h"
#include "../../Core/ParameterIDs.h"

namespace aura
{
    // DSPModule adapter around whichever AmpModelBase is currently selected.
    // Owns the active model and swaps it out (via AmpFactory) when the user
    // changes the "ampModel" parameter, without the SignalChain needing to
    // know that happened.
    class AmpModule : public DSPModule
    {
    public:
        explicit AmpModule (juce::AudioProcessorValueTreeState& stateToUse);

        void prepare (const juce::dsp::ProcessSpec& spec) override;
        void process (juce::dsp::ProcessContextReplacing<double>& context) override;
        void reset() override;

        juce::String getName() const override { return "Amp"; }
        bool isBypassed() const override;

    private:
        void updateActiveModel();
        void updateParameters();

        juce::dsp::ProcessSpec currentSpec {};
        juce::String currentModelID;
        std::unique_ptr<AmpModelBase> activeModel;
    };
}
