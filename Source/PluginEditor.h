#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/AuraLookAndFeel.h"
#include "GUI/ModulePanel.h"
#include "GUI/VUMeterComponent.h"

namespace aura
{
    // Full hand-drawn amp-panel GUI: a branded header (logo, instrument
    // selector, live output meter) over a row of module panels that mirror
    // the real signal chain (Gate -> Comp -> Amp -> Cab -> EQ -> Limiter),
    // each drawn as an inset metal frame with rotary knobs and a bypass LED.
    // Everything is procedural juce::Graphics drawing - no image assets.
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

        juce::ComboBox instrumentBox;
        std::unique_ptr<ComboAttachment> instrumentAttachment;

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
