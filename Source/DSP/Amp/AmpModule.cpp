#include "AmpModule.h"

namespace aura
{
    AmpModule::AmpModule (juce::AudioProcessorValueTreeState& stateToUse)
        : DSPModule (stateToUse)
    {
    }

    void AmpModule::prepare (const juce::dsp::ProcessSpec& spec)
    {
        currentSpec = spec;
        updateActiveModel();
        if (activeModel != nullptr)
            activeModel->prepare (spec);
    }

    void AmpModule::updateActiveModel()
    {
        auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (ParamIDs::ampModel));
        const auto modelID = choiceParam != nullptr ? choiceParam->choices[choiceParam->getIndex()] : juce::String ("JCM800");

        if (modelID == currentModelID && activeModel != nullptr)
            return;

        auto newModel = AmpFactory::getInstance().create (modelID);
        if (newModel == nullptr)
            newModel = AmpFactory::getInstance().create ("JCM800"); // safe fallback

        if (newModel != nullptr)
        {
            newModel->prepare (currentSpec);
            activeModel = std::move (newModel);
            currentModelID = modelID;
        }
    }

    void AmpModule::updateParameters()
    {
        if (activeModel == nullptr)
            return;

        activeModel->setGain     (apvts.getRawParameterValue (ParamIDs::ampGain)->load());
        activeModel->setBass     (apvts.getRawParameterValue (ParamIDs::ampBass)->load());
        activeModel->setMid      (apvts.getRawParameterValue (ParamIDs::ampMid)->load());
        activeModel->setTreble   (apvts.getRawParameterValue (ParamIDs::ampTreble)->load());
        activeModel->setPresence (apvts.getRawParameterValue (ParamIDs::ampPresence)->load());
        activeModel->setMaster   (apvts.getRawParameterValue (ParamIDs::ampMaster)->load());
    }

    void AmpModule::process (juce::dsp::ProcessContextReplacing<double>& context)
    {
        updateActiveModel();
        updateParameters();
        if (activeModel != nullptr)
            activeModel->process (context);
    }

    void AmpModule::reset()
    {
        if (activeModel != nullptr)
            activeModel->reset();
    }

    bool AmpModule::isBypassed() const
    {
        return apvts.getRawParameterValue (ParamIDs::ampBypass)->load() > 0.5f;
    }
}
