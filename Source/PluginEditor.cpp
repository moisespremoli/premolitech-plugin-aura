#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"
#include "GUI/UIAssets.h"

namespace aura
{
    namespace
    {
        // The reference photo (Resources/UI/panel_photo.png) is a
        // user-supplied mock-up, 1402x1122 source pixels, scaled down to
        // fill this editor's window exactly (no distortion, since the
        // window is sized to the same aspect ratio in the constructor
        // below).
        constexpr float photoScale = 1040.0f / 1402.0f;

        juce::Point<int> scaledPoint (float sourceX, float sourceY)
        {
            return { juce::roundToInt (sourceX * photoScale), juce::roundToInt (sourceY * photoScale) };
        }

        juce::Rectangle<int> scaledRect (float x0, float y0, float x1, float y1)
        {
            return { juce::roundToInt (x0 * photoScale), juce::roundToInt (y0 * photoScale),
                     juce::roundToInt ((x1 - x0) * photoScale), juce::roundToInt ((y1 - y0) * photoScale) };
        }

        // Sized to stay inside the gap between the printed 3/7 (and 1/9)
        // digits around each knob hole in the new flat background - at 78
        // the drawn dial's rim overlapped those numbers.
        constexpr int standardKnobDiameter = 54;
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
        // Window aspect matches the reference photo (1402x1122) exactly,
        // so it fills the window with zero distortion.
        setSize (1040, 832);
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
            auto jewel = juce::Rectangle<float> (22.0f, 22.0f).withCentre (scaledPoint (340.0f, 80.0f).toFloat());
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
        // panel_photo.png (in its own 1402x1122 source pixels, via
        // scaledPoint()/scaledRect()) so each real control lands exactly on
        // top of the matching knob hole / label already drawn in that
        // photo. Every bypass jewel sits on the small screw already
        // printed at that panel's own boundary (one screw per section,
        // marking its right-hand divider) - the CABINET box has no knob
        // art of its own though, so its toolbar/MIX knob are placed by eye.

        // Placed in the open gap between the power jewel and the
        // "PREMOLI LABS" title on the nameplate strip.
        outputMeter.setBounds (juce::Rectangle<int> (170, 26).withCentre (scaledPoint (460.0f, 80.0f)));
        // Sits inside the empty pill outline already printed top-right.
        instrumentBox.setBounds (juce::Rectangle<int> (165, 28).withCentre (scaledPoint (1278.0f, 81.0f)));

        inputPanel.setBounds (scaledRect (0, 145, 270, 460));
        inputPanel.layoutKnobsExplicit ({ scaledPoint (160, 305) }, standardKnobDiameter);

        gatePanel.setBounds (scaledRect (270, 145, 715, 460));
        gatePanel.layoutKnobsExplicit ({ scaledPoint (352, 305), scaledPoint (495, 305), scaledPoint (625, 305) },
                                        standardKnobDiameter);
        gatePanel.layoutBypassExplicit (scaledPoint (692, 172), jewelDiameter);

        compPanel.setBounds (scaledRect (715, 145, 1360, 460));
        compPanel.layoutKnobsExplicit ({ scaledPoint (795, 305), scaledPoint (915, 305), scaledPoint (1030, 305),
                                          scaledPoint (1170, 305), scaledPoint (1290, 305) },
                                        standardKnobDiameter);
        compPanel.layoutBypassExplicit (scaledPoint (1350, 170), jewelDiameter);

        ampPanel.setBounds (scaledRect (0, 460, 1360, 770));
        ampPanel.layoutKnobsExplicit ({ scaledPoint (160, 630), scaledPoint (385, 630), scaledPoint (610, 630),
                                         scaledPoint (825, 630), scaledPoint (1035, 630), scaledPoint (1255, 630) },
                                       standardKnobDiameter);
        ampPanel.layoutBypassExplicit (scaledPoint (1350, 485), jewelDiameter);
        // Thin strip right under the "AMPLIFIER" title, above the knob row.
        ampPanel.layoutComboExplicit (scaledRect (20, 485, 1340, 515));

        // No knob art at all is printed inside this box (it's the one
        // section left blank in the reference photo), so the toggle,
        // LOAD IR button and MIX knob are placed by eye instead of traced.
        cabPanel.setBounds (scaledRect (0, 770, 365, 1122));
        cabPanel.layoutKnobsExplicit ({ scaledPoint (182, 950) }, standardKnobDiameter);
        cabPanel.layoutBypassExplicit (scaledPoint (350, 792), jewelDiameter);
        cabPanel.layoutToolbarExplicit (scaledRect (20, 845, 110, 870), scaledRect (115, 845, 345, 868));

        eqPanel.setBounds (scaledRect (365, 770, 800, 1122));
        eqPanel.layoutKnobsExplicit ({ scaledPoint (460, 905), scaledPoint (595, 905), scaledPoint (720, 905) },
                                      standardKnobDiameter);
        eqPanel.layoutBypassExplicit (scaledPoint (783, 790), jewelDiameter);

        // Widened past the usual 1060 boundary so its bypass jewel - on
        // the screw printed just before "OUTPUT", at x=1095 - has room
        // without falling outside this panel's own clip region.
        limiterPanel.setBounds (scaledRect (800, 770, 1110, 1122));
        limiterPanel.layoutKnobsExplicit ({ scaledPoint (975, 905) }, bigKnobDiameter);
        limiterPanel.layoutBypassExplicit (scaledPoint (1095, 795), jewelDiameter);

        outputPanel.setBounds (scaledRect (1060, 770, 1360, 1122));
        outputPanel.layoutKnobsExplicit ({ scaledPoint (1250, 905) }, standardKnobDiameter);
    }
}
