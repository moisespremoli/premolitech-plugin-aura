#pragma once

#include "AmpModelBase.h"
#include <memory>
#include <functional>
#include <map>

namespace aura
{
    // Registry of every amp model the plugin knows about. Adding a new amp
    // (vintage/modern/boutique/bass/acoustic) means writing one subclass of
    // AmpModelBase and adding one registration line here - nothing else in
    // the engine, GUI wiring, or preset system needs to change.
    class AmpFactory
    {
    public:
        using Creator = std::function<std::unique_ptr<AmpModelBase>()>;

        static AmpFactory& getInstance()
        {
            static AmpFactory instance;
            return instance;
        }

        void registerModel (const juce::String& id, Creator creator)
        {
            creators[id] = std::move (creator);
        }

        std::unique_ptr<AmpModelBase> create (const juce::String& id) const
        {
            auto it = creators.find (id);
            if (it != creators.end())
                return it->second();
            return nullptr;
        }

        juce::StringArray getAvailableModelIDs() const
        {
            juce::StringArray ids;
            for (auto& [id, creator] : creators)
                ids.add (id);
            return ids;
        }

    private:
        AmpFactory() = default;
        std::map<juce::String, Creator> creators;
    };

    // Registers a model type under `id` the first time this translation
    // unit is touched. Declare one static instance per model .cpp file,
    // e.g. `static AmpModelRegistrar<JCM800Model> registrar ("JCM800");`.
    template <typename ModelType>
    struct AmpModelRegistrar
    {
        explicit AmpModelRegistrar (const juce::String& id)
        {
            AmpFactory::getInstance().registerModel (id, [] { return std::make_unique<ModelType>(); });
        }
    };
}
