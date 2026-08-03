#pragma once

namespace aura
{
    // JUCE's plugin build links every module through an intermediate static
    // "SharedCode" library, and a static-archive linker only pulls in .o
    // files that resolve some other undefined symbol. A model's .cpp whose
    // only content is a self-registering `AmpModelRegistrar` global has no
    // symbol anything else references, so the linker silently drops the
    // whole translation unit - the model never registers, even though the
    // registration code itself is correct. These forceLink functions exist
    // purely so PluginProcessor can call something real from each model's
    // .cpp, which forces the linker to include it. Add one line here (and
    // the matching empty definition in the new model's .cpp) whenever a new
    // amp model is added.
    void forceLinkJCM800Model();
    void forceLinkFenderDeluxeModel();
    void forceLinkPeavey5150Model();

    inline void forceLinkAllAmpModels()
    {
        forceLinkJCM800Model();
        forceLinkFenderDeluxeModel();
        forceLinkPeavey5150Model();
    }
}
