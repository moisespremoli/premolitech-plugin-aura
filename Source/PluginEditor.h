#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/AuraLookAndFeel.h"
#include "GUI/ModulePanel.h"
#include "GUI/VUMeterComponent.h"

namespace aura
{
    // Full hand-drawn "vintage tube amp" GUI: a black tolex-style cabinet
    // (leather handle, chrome corners) around a cream control panel, with a
    // branded header (script-ish logo, jewel power light, analogue VU
    // needle, instrument selector) over module panels that mirror the real
    // signal chain (Gate -> Comp -> Amp -> Cab -> EQ -> Limiter), each drawn
    // with numbered-scale knobs and a jewel bypass LED. Everything is
    // procedural juce::Graphics drawing - no image assets. The colour
    // palette evokes vintage American tube amps in general, not any one
    // manufacturer's specific trade dress or logo.
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
        juce::Rectangle<int> getGrilleBounds() const;

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
