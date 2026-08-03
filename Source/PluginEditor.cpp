#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"
#include "GUI/UIAssets.h"

namespace aura
{
    namespace
    {
        // The reference photo (Resources/UI/panel_photo.png) is a crop of a
        // user-supplied mock-up, 704x560 source pixels, scaled up to fill
        // this editor's window exactly (no distortion, since the window is
        // sized to the same aspect ratio in the constructor below).
        constexpr float photoScale = 1040.0f / 704.0f;

        juce::Point<int> scaledPoint (float sourceX, float sourceY)
        {
            return { juce::roundToInt (sourceX * photoScale), juce::roundToInt (sourceY * photoScale) };
        }

        juce::Rectangle<int> scaledRect (float x0, float y0, float x1, float y1)
        {
            return { juce::roundToInt (x0 * photoScale), juce::roundToInt (y0 * photoScale),
                     juce::roundToInt ((x1 - x0) * photoScale), juce::roundToInt ((y1 - y0) * photoScale) };
        }

        constexpr int standardKnobDiameter = 78;
        constexpr int bigKnobDiameter      = 120;
        constexpr int jewelDiameter        = 20;
    }

    AuraAudioProcessorEditor::AuraAudioProcessorEditor (AuraAudioProcessor& processorToEdit)
        : AudioProcessorEditor (&processorToEdit), processor (processorToEdit),
          outputMeter ([this] { return processor.getOutputLevel(); })
    {
        setLookAndFeel (&lookAndFeel);

        // The reference photo already prints its own numbered dial scale
        // around every knob hole - drawing our own on top would double up
        // and likely not line up with it.
        lookAndFeel.showPrintedScale = false;

        instrumentBox.addItemList (getInstrumentChoices(), 1);
        addAndMakeVisible (instrumentBox);
        instrumentAttachment = std::make_unique<ComboAttachment> (
            processor.apvts, ParamIDs::instrument, instrumentBox);

        addAndMakeVisible (outputMeter);

        addAndMakeVisible (inputPanel);
        inputPanel.addKnob (processor.apvts, ParamIDs::inputGain, "GAIN");

        addAndMakeVisible (gatePanel);
        gatePanel.addBypassToggle (processor.apvts, ParamIDs::gateBypass);
        gatePanel.addKnob (processor.apvts, ParamIDs::gateThreshold, "THRESH");
        gatePanel.addKnob (processor.apvts, ParamIDs::gateAttack, "ATTACK");
        gatePanel.addKnob (processor.apvts, ParamIDs::gateRelease, "RELEASE");

        addAndMakeVisible (compPanel);
        compPanel.addBypassToggle (processor.apvts, ParamIDs::compBypass);
        compPanel.addKnob (processor.apvts, ParamIDs::compThreshold, "THRESH");
        compPanel.addKnob (processor.apvts, ParamIDs::compRatio, "RATIO");
        compPanel.addKnob (processor.apvts, ParamIDs::compAttack, "ATTACK");
        compPanel.addKnob (processor.apvts, ParamIDs::compRelease, "RELEASE");
        compPanel.addKnob (processor.apvts, ParamIDs::compMakeup, "MAKEUP");

        addAndMakeVisible (ampPanel);
        ampPanel.addBypassToggle (processor.apvts, ParamIDs::ampBypass);
        if (auto* ampModelParam = dynamic_cast<juce::AudioParameterChoice*> (
                processor.apvts.getParameter (ParamIDs::ampModel)))
            ampPanel.addComboBox (processor.apvts, ParamIDs::ampModel, ampModelParam->choices);
        ampPanel.addKnob (processor.apvts, ParamIDs::ampGain, "GAIN");
        ampPanel.addKnob (processor.apvts, ParamIDs::ampBass, "BASS");
        ampPanel.addKnob (processor.apvts, ParamIDs::ampMid, "MID");
        ampPanel.addKnob (processor.apvts, ParamIDs::ampTreble, "TREBLE");
        ampPanel.addKnob (processor.apvts, ParamIDs::ampPresence, "PRESENCE");
        ampPanel.addKnob (processor.apvts, ParamIDs::ampMaster, "MASTER");

        addAndMakeVisible (cabPanel);
        cabPanel.addBypassToggle (processor.apvts, ParamIDs::cabBypass);
        cabPanel.addToolbarButton ("LOAD IR...", [this] { openIRFileChooser(); });
        cabPanel.setStatusText (processor.getCabinetIRName());
        cabPanel.addKnob (processor.apvts, ParamIDs::cabMix, "MIX");

        addAndMakeVisible (eqPanel);
        eqPanel.addBypassToggle (processor.apvts, ParamIDs::eqBypass);
        eqPanel.addKnob (processor.apvts, ParamIDs::eqLow, "LOW");
        eqPanel.addKnob (processor.apvts, ParamIDs::eqMid, "MID");
        eqPanel.addKnob (processor.apvts, ParamIDs::eqHigh, "HIGH");

        addAndMakeVisible (limiterPanel);
        limiterPanel.addBypassToggle (processor.apvts, ParamIDs::limiterBypass);
        limiterPanel.addKnob (processor.apvts, ParamIDs::limiterCeiling, "CEILING");

        addAndMakeVisible (outputPanel);
        outputPanel.addKnob (processor.apvts, ParamIDs::outputGain, "GAIN");

        for (auto* panel : { &inputPanel, &gatePanel, &compPanel, &ampPanel,
                              &cabPanel, &eqPanel, &limiterPanel, &outputPanel })
            panel->setExplicitLayoutMode (true);

        setResizable (false, false);
        // Window aspect matches the reference photo's crop (704x560)
        // exactly, so it fills the window with zero distortion.
        setSize (1040, 827);
    }

    AuraAudioProcessorEditor::~AuraAudioProcessorEditor()
    {
        setLookAndFeel (nullptr);
    }

    void AuraAudioProcessorEditor::openIRFileChooser()
    {
        irFileChooser = std::make_unique<juce::FileChooser> (
            "Escolhe um ficheiro de IR (WAV/AIFF)", juce::File(), "*.wav;*.aif;*.aiff");

        constexpr auto chooserFlags = juce::FileBrowserComponent::openMode
                                     | juce::FileBrowserComponent::canSelectFiles;

        irFileChooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& chooser)
        {
            const auto file = chooser.getResult();
            if (file != juce::File() && processor.loadCabinetIRFromFile (file))
                cabPanel.setStatusText (processor.getCabinetIRName());
        });
    }

    void AuraAudioProcessorEditor::paint (juce::Graphics& g)
    {
        // Drawn directly in the top-level editor's own paint() (not inside
        // a child component), so a plain drawImage() is safe here - see the
        // knob-frame comment in AuraLookAndFeel.cpp for why that
        // distinction matters on this renderer.
        const auto& photo = UIAssets::getPanelPhoto();
        if (photo.isValid())
            g.drawImage (photo, getLocalBounds().toFloat());

        // A small red power jewel over the plain metal circle already
        // printed in the photo's nameplate strip.
        const auto& jewelImage = UIAssets::getJewelRed();
        if (jewelImage.isValid())
        {
            auto jewel = juce::Rectangle<float> (22.0f, 22.0f).withCentre (scaledPoint (170.0f, 37.0f).toFloat());
            const auto scale = jewel.getWidth() / (float) jewelImage.getWidth();
            g.setFillType (juce::FillType (jewelImage, juce::AffineTransform::scale (scale)
                                                            .translated (jewel.getX(), jewel.getY())));
            g.fillEllipse (jewel);
            g.setFillType (juce::FillType (juce::Colours::black));
        }
    }

    void AuraAudioProcessorEditor::resized()
    {
        // Every position below is measured directly off Resources/UI/
        // panel_photo.png (in its own 704x560 source pixels, via
        // scaledPoint()/scaledRect()) so each real control lands exactly on
        // top of the matching knob hole / label / cutout already drawn in
        // that photo.

        // Placed in the open gap between the power jewel and the
        // instrument selector on the nameplate strip - the reference photo
        // doesn't draw a VU meter itself, so there's no exact spot to match.
        outputMeter.setBounds (juce::Rectangle<int> (90, 54).withCentre (scaledPoint (380.0f, 35.0f)));
        instrumentBox.setBounds (juce::Rectangle<int> (200, 30).withCentre (scaledPoint (620.0f, 37.0f)));

        inputPanel.setBounds (scaledRect (0, 60, 140, 225));
        inputPanel.layoutKnobsExplicit ({ scaledPoint (75, 145) }, standardKnobDiameter);

        gatePanel.setBounds (scaledRect (140, 60, 355, 225));
        gatePanel.layoutKnobsExplicit ({ scaledPoint (173, 145), scaledPoint (243, 145), scaledPoint (313, 145) },
                                        standardKnobDiameter);
        gatePanel.layoutBypassExplicit (scaledPoint (343, 82), jewelDiameter);

        compPanel.setBounds (scaledRect (355, 60, 704, 225));
        compPanel.layoutKnobsExplicit ({ scaledPoint (393, 145), scaledPoint (456, 145), scaledPoint (518, 145),
                                          scaledPoint (582, 145), scaledPoint (645, 145) },
                                        standardKnobDiameter);
        compPanel.layoutBypassExplicit (scaledPoint (693, 82), jewelDiameter);

        ampPanel.setBounds (scaledRect (0, 225, 704, 375));
        ampPanel.layoutKnobsExplicit ({ scaledPoint (78, 305), scaledPoint (187, 305), scaledPoint (296, 305),
                                         scaledPoint (405, 305), scaledPoint (514, 305), scaledPoint (623, 305) },
                                       standardKnobDiameter);
        // Kept in the gap between the amp-model combo box and the knob row
        // (the combo box renders taller than its nominal bounds, so a
        // jewel placed just above it still ends up hidden underneath).
        ampPanel.layoutBypassExplicit (scaledPoint (685, 258), jewelDiameter);
        ampPanel.layoutComboExplicit (scaledRect (15, 228, 695, 248));

        cabPanel.setBounds (scaledRect (0, 375, 180, 540));
        cabPanel.layoutKnobsExplicit ({ scaledPoint (100, 450) }, standardKnobDiameter);
        cabPanel.layoutBypassExplicit (scaledPoint (167, 392), jewelDiameter);
        cabPanel.layoutToolbarExplicit (scaledRect (15, 392, 100, 408), scaledRect (104, 392, 175, 408));

        eqPanel.setBounds (scaledRect (180, 375, 400, 540));
        eqPanel.layoutKnobsExplicit ({ scaledPoint (224, 450), scaledPoint (294, 450), scaledPoint (362, 450) },
                                      standardKnobDiameter);
        eqPanel.layoutBypassExplicit (scaledPoint (388, 392), jewelDiameter);

        limiterPanel.setBounds (scaledRect (400, 375, 565, 540));
        limiterPanel.layoutKnobsExplicit ({ scaledPoint (486, 450) }, bigKnobDiameter);
        limiterPanel.layoutBypassExplicit (scaledPoint (555, 392), jewelDiameter);

        outputPanel.setBounds (scaledRect (565, 375, 704, 540));
        outputPanel.layoutKnobsExplicit ({ scaledPoint (627, 450) }, standardKnobDiameter);
    }
}
