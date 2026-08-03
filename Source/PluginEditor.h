#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/AuraLookAndFeel.h"
#include "GUI/ModulePanel.h"
#include "GUI/VUMeterComponent.h"

namespace aura
{
    // Full "vintage tube amp" GUI, mixing procedural juce::Graphics drawing
    // with real processed photographic materials (tolex, worn steel panel,
    // leather handle, grille cloth, chicken-head knobs, jewel lights): a
    // black tolex-covered cabinet (leather handle, chrome corners) around an
    // aged/worn steel control panel, with a branded header (wordmark, jewel
    // power light, analogue VU needle, instrument selector) over module
    // panels that mirror the real signal chain (Gate -> Comp -> Amp -> Cab
    // -> EQ -> Limiter), each drawn with numbered-scale knobs and a jewel
    // bypass LED. The colour palette and worn-metal look evoke vintage
    // American tube amps in general, not any one manufacturer's specific
    // trade dress or logo.
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
