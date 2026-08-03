#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/AuraLookAndFeel.h"
#include "GUI/ModulePanel.h"
#include "GUI/VUMeterComponent.h"

namespace aura
{
    // Hardware-rack-style GUI matching the visual family of the other
    // Premoli Labs plug-ins (PLI-1A, PLI-2A, PHATTER): one continuous
    // weathered blue-grey steel panel (real photographed material) with
    // chrome corner screws, a branded header (bold wordmark, jewel power
    // light, analogue VU needle behind a real photographed bezel,
    // instrument selector), and labelled knob groups mirroring the real
    // signal chain (Gate -> Comp -> Amp -> Cab -> EQ -> Limiter) printed
    // straight onto the panel - no per-module boxes, no cabinet/tolex/
    // handle motif. Knobs are real photographed "chicken-head" pointer
    // knobs, same asset family as PLI-2A's.
    class AuraAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit AuraAudioProcessorEditor (AuraAudioProcessor& processorToEdit);
        ~AuraAudioProcessorEditor() override;

        void paint (juce::Graphics& g) override;
        void resized() override;

    private:
        using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

        AuraAudioProcessor& processor;
        AuraLookAndFeel lookAndFeel;

        juce::Image backgroundImage;
        void rebuildBackgroundImage();
        juce::Rectangle<int> getPanelBounds() const;

        juce::ComboBox instrumentBox;
        std::unique_ptr<ComboAttachment> instrumentAttachment;

        std::unique_ptr<juce::FileChooser> irFileChooser;
        void openIRFileChooser();

        VUMeterComponent outputMeter;

        ModulePanel inputPanel   { "INPUT" };
        ModulePanel gatePanel    { "NOISE GATE" };
        ModulePanel compPanel    { "COMPRESSOR" };
        ModulePanel ampPanel     { "AMPLIFIER" };
        ModulePanel cabPanel     { "CABINET" };
        ModulePanel eqPanel      { "EQUALIZER" };
        ModulePanel limiterPanel { "LIMITER" };
        ModulePanel outputPanel  { "OUTPUT" };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuraAudioProcessorEditor)
    };
}
