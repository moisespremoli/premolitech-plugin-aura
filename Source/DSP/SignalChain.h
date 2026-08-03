#pragma once

#include "DSPModule.h"
#include <vector>
#include <memory>

namespace aura
{
    // Owns the ordered list of processing modules and drives them each block.
    // Reordering (for the future drag & drop UI) is just moving entries
    // around in `modules` - the engine doesn't care what order they're in.
    class SignalChain
    {
    public:
        void addModule (std::unique_ptr<DSPModule> module)
        {
            modules.push_back (std::move (module));
        }

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            for (auto& m : modules)
                m->prepare (spec);
        }

        void process (juce::dsp::ProcessContextReplacing<double>& context)
        {
            for (auto& m : modules)
                if (! m->isBypassed())
                    m->process (context);
        }

        void reset()
        {
            for (auto& m : modules)
                m->reset();
        }

        // Moves the module currently at `fromIndex` so it sits at `toIndex`,
        // shifting the modules in between. Used by the drag & drop reorder
        // UI (Phase 6) - the engine itself has no opinion on ordering.
        void moveModule (int fromIndex, int toIndex)
        {
            if (fromIndex == toIndex
                || ! juce::isPositiveAndBelow (fromIndex, (int) modules.size())
                || ! juce::isPositiveAndBelow (toIndex, (int) modules.size()))
                return;

            auto moved = std::move (modules[(size_t) fromIndex]);
            modules.erase (modules.begin() + fromIndex);
            modules.insert (modules.begin() + toIndex, std::move (moved));
        }

        int getNumModules() const { return (int) modules.size(); }
        DSPModule* getModule (int index) const
        {
            return juce::isPositiveAndBelow (index, (int) modules.size()) ? modules[(size_t) index].get() : nullptr;
        }

    private:
        std::vector<std::unique_ptr<DSPModule>> modules;
    };
}
