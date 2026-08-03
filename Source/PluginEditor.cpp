#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"
#include "GUI/UIAssets.h"

namespace aura
{
    namespace
    {
        constexpr int panelMargin  = 14; // uniform edge margin, screws sit inset from here
        constexpr int headerHeight = 92;

        const juce::Colour printedText  (0xffdcd6c8);
        const juce::Colour chromeLight  (0xfff0f0f0);
        const juce::Colour chromeDark   (0xff666666);

        // A small chrome screw head, like the ones holding a real rack
        // panel's faceplate on - drawn once at each of the panel's four
        // corners (not per module - a single continuous panel, matching
        // PLI-1A/PLI-2A/PHATTER, has no per-group hardware).
        void drawScrew (juce::Graphics& g, juce::Point<float> centre)
        {
            constexpr float r = 5.0f;
            juce::Rectangle<float> bounds (r * 2.0f, r * 2.0f);
            bounds.setCentre (centre);

            juce::ColourGradient grad (chromeLight, bounds.getX(), bounds.getY(),
                                        chromeDark, bounds.getRight(), bounds.getBottom(), false);
            g.setGradientFill (grad);
            g.fillEllipse (bounds);
            g.setColour (juce::Colours::black.withAlpha (0.4f));
            g.drawEllipse (bounds, 0.5f);

            juce::Path slot;
            slot.addRoundedRectangle (-r * 0.7f, -0.6f, r * 1.4f, 1.2f, 0.5f);
            g.setColour (juce::Colour (0xff2a2a2a));
            g.fillPath (slot, juce::AffineTransform::rotation (0.5f).translated (centre.x, centre.y));
        }

        // Lays a row of components out left-to-right, each given a width
        // proportional to its weight, so panels with more knobs get more
        // room without any manual pixel arithmetic per layout.
        void layoutWeightedRow (juce::Rectangle<int> area,
                                 std::initializer_list<std::pair<juce::Component*, int>> items)
        {
            int totalWeight = 0;
            for (auto& item : items)
                totalWeight += item.second;

            constexpr int gap = 10;
            const int totalGaps = gap * (int) (items.size() - 1);
            const int availableWidth = area.getWidth() - totalGaps;

            int x = area.getX();
            for (auto& item : items)
            {
                const int w = availableWidth * item.second / totalWeight;
                item.first->setBounds (x, area.getY(), w, area.getHeight());
                x += w + gap;
            }
        }
    }

    AuraAudioProcessorEditor::AuraAudioProcessorEditor (AuraAudioProcessor& processorToEdit)
        : AudioProcessorEditor (&processorToEdit), processor (processorToEdit),
          outputMeter ([this] { return processor.getOutputLevel(); })
    {
        setLookAndFeel (&lookAndFeel);

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

        setResizable (false, false);
        setSize (1040, 800);
        rebuildBackgroundImage();
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

    juce::Rectangle<int> AuraAudioProcessorEditor::getPanelBounds() const
    {
        return getLocalBounds().reduced (panelMargin);
    }

    void AuraAudioProcessorEditor::rebuildBackgroundImage()
    {
        backgroundImage = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics g (backgroundImage);

        // One continuous real photographed weathered steel panel, tiled to
        // cover the whole window - no separate cabinet/tolex body and
        // control-panel plate; the other Premoli Labs plug-ins are a single
        // painted panel front to back.
        const auto& steelTexture = UIAssets::getSteelPanel();
        if (steelTexture.isValid())
        {
            g.setFillType (juce::FillType (steelTexture, juce::AffineTransform()));
            g.fillAll();
            g.setFillType (juce::FillType (juce::Colours::black));
        }
        else
        {
            g.setColour (juce::Colour (0xff3a4148));
            g.fillAll();
        }

        // Subtle vignette so the tiled texture reads as one large panel
        // rather than an obviously repeating swatch.
        juce::ColourGradient vignette (juce::Colours::transparentBlack, getWidth() * 0.5f, getHeight() * 0.5f,
                                        juce::Colours::black.withAlpha (0.3f), 0.0f, 0.0f, true);
        g.setGradientFill (vignette);
        g.fillAll();

        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawRect (getLocalBounds().toFloat(), 2.0f);

        for (auto corner : { juce::Point<float> ((float) panelMargin + 8.0f, (float) panelMargin + 8.0f),
                              juce::Point<float> ((float) getWidth() - panelMargin - 8.0f, (float) panelMargin + 8.0f),
                              juce::Point<float> ((float) panelMargin + 8.0f, (float) getHeight() - panelMargin - 8.0f),
                              juce::Point<float> ((float) getWidth() - panelMargin - 8.0f, (float) getHeight() - panelMargin - 8.0f) })
            drawScrew (g, corner);
    }

    void AuraAudioProcessorEditor::paint (juce::Graphics& g)
    {
        // The single steel panel (with its corner screws) is already fully
        // baked into backgroundImage by rebuildBackgroundImage() - nothing
        // else to fill here, matching the other Premoli Labs plug-ins'
        // one-continuous-panel construction instead of a floating inset box.
        g.drawImageAt (backgroundImage, 0, 0);

        auto panel = getPanelBounds();

        // Header: bold sans-serif wordmark (no italics/script), like PLI-1A/
        // PLI-2A's "PREMOLI labs" print, plus a soft dark drop shadow so it
        // reads clearly against the steel.
        auto printLabel = [&] (const juce::String& text, juce::Rectangle<int> area, juce::Justification j)
        {
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.drawText (text, area.translated (0, 1), j);
            g.setColour (printedText);
            g.drawText (text, area, j);
        };

        g.setFont (juce::Font (juce::FontOptions (22.0f)).boldened().withExtraKerningFactor (0.06f));
        printLabel ("PREMOLI LABS", { panel.getX() + 4, panel.getY() + 14, 260, 26 }, juce::Justification::centredLeft);

        g.setFont (juce::Font (juce::FontOptions (13.0f)).withExtraKerningFactor (0.14f));
        printLabel ("AURA", { panel.getX() + 6, panel.getY() + 42, 220, 18 }, juce::Justification::centredLeft);

        {
            juce::Rectangle<float> jewel (26.0f, 26.0f);
            jewel.setCentre ({ (float) panel.getX() + 232.0f, (float) panel.getY() + (float) headerHeight * 0.5f });

            const auto& jewelImage = UIAssets::getJewelRed();
            if (jewelImage.isValid())
            {
                const auto scale = jewel.getWidth() / (float) jewelImage.getWidth();
                g.setFillType (juce::FillType (jewelImage, juce::AffineTransform::scale (scale)
                                                                .translated (jewel.getX(), jewel.getY())));
                g.fillEllipse (jewel);
                g.setFillType (juce::FillType (juce::Colours::black));
            }
        }

        g.setFont (juce::Font (juce::FontOptions (10.0f)).boldened().withExtraKerningFactor (0.05f));
        printLabel ("INSTRUMENT", instrumentBox.getBounds().translated (0, -16).withHeight (14),
                    juce::Justification::centred);
        printLabel ("OUTPUT LEVEL", outputMeter.getBounds().translated (0, -16).withHeight (14),
                    juce::Justification::centred);
    }

    void AuraAudioProcessorEditor::resized()
    {
        auto panel = getPanelBounds();
        auto header = panel.removeFromTop (headerHeight);

        instrumentBox.setBounds (header.removeFromRight (190).reduced (14, 30));
        // Sized close to the real vu_frame.png aspect ratio (~1.68:1) so
        // the bezel image doesn't stretch noticeably.
        outputMeter.setBounds (header.removeFromRight (106).reduced (8, 19));

        auto body = panel.reduced (16, 10);

        const int rowHeight = (body.getHeight() - 20) / 3;
        auto row1 = body.removeFromTop (rowHeight);
        body.removeFromTop (10);
        auto row2 = body.removeFromTop (rowHeight);
        body.removeFromTop (10);
        auto row3 = body;

        layoutWeightedRow (row1, { { &inputPanel, 2 }, { &gatePanel, 4 }, { &compPanel, 6 } });
        layoutWeightedRow (row2, { { &ampPanel, 1 } });
        layoutWeightedRow (row3, { { &cabPanel, 3 }, { &eqPanel, 4 }, { &limiterPanel, 3 }, { &outputPanel, 2 } });
    }
}
