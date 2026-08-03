#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/AuraLookAndFeel.h"
#include "GUI/ModulePanel.h"
#include "GUI/VUMeterComponent.h"

namespace aura
{
    // GUI built directly on top of a single user-supplied reference photo
    // (a full mock-up of the AURA panel, printed labels/scale and all),
    // used as-is for the background. Every real control (knob, bypass
    // jewel, combo box, VU meter) is placed at an explicit pixel position
    // measured from that photo (see the *_GATE_JEWEL/*_KNOB-style constants
    // in PluginEditor.cpp) instead of an auto-computed layout, so each
    // control lines up with the matching cutout/label already drawn in the
    // image. AuraLookAndFeel's own printed knob scale is switched off
    // (showPrintedScale = false) since the photo already prints one.
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
