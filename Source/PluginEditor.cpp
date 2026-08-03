#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"
#include "GUI/UIAssets.h"

namespace aura
{
    namespace
    {
        constexpr int tolexMargin    = 18;
        constexpr int handleClearance = 34; // top gap left for the leather handle arch
        constexpr int grilleHeight   = 46;
        constexpr int headerHeight   = 92;

        const juce::Colour creamTop     (0xfffdfbf7);
        const juce::Colour creamBottom  (0xffe6dbc4);
        const juce::Colour tanBorder    (0xff9c8a5c);
        const juce::Colour engravedText (0xffece3ca);
        const juce::Colour chromeLight  (0xfff0f0f0);
        const juce::Colour chromeDark   (0xff666666);

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
        return { tolexMargin, handleClearance,
                 getWidth() - 2 * tolexMargin,
                 getHeight() - handleClearance - grilleHeight - tolexMargin - 8 };
    }

    juce::Rectangle<int> AuraAudioProcessorEditor::getGrilleBounds() const
    {
        auto panel = getPanelBounds();
        return { panel.getX(), panel.getBottom() + 8, panel.getWidth(), grilleHeight };
    }

    void AuraAudioProcessorEditor::rebuildBackgroundImage()
    {
        backgroundImage = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics g (backgroundImage);

        // Real photographed black pebbled tolex/leatherette, tiled to cover
        // the whole cabinet body (replaces the old procedural gradient +
        // synthetic weave noise).
        {
            const auto& tolexTexture = UIAssets::getTolexPanel();
            if (tolexTexture.isValid())
            {
                g.setFillType (juce::FillType (tolexTexture, juce::AffineTransform()));
                g.fillAll();
                g.setFillType (juce::FillType (juce::Colours::black));
            }
            else
            {
                juce::ColourGradient tolexGradient (juce::Colour (0xff2a2a2a), getWidth() * 0.5f, getHeight() * 0.5f,
                                                     juce::Colour (0xff0d0d0d), 0.0f, 0.0f, true);
                g.setGradientFill (tolexGradient);
                g.fillAll();
            }

            // Subtle vignette so the tiled texture reads as one large panel
            // rather than an obviously repeating swatch.
            juce::ColourGradient vignette (juce::Colours::transparentBlack, getWidth() * 0.5f, getHeight() * 0.5f,
                                            juce::Colours::black.withAlpha (0.35f), 0.0f, 0.0f, true);
            g.setGradientFill (vignette);
            g.fillAll();
        }

        // Stitched seam just inside the outer edge.
        {
            juce::Path seam;
            seam.addRoundedRectangle (10.0f, 10.0f, (float) getWidth() - 20.0f, (float) getHeight() - 20.0f, 14.0f);
            juce::Path dashedSeam;
            float dashLengths[] = { 4.0f, 4.0f };
            juce::PathStrokeType (1.2f).createDashedStroke (dashedSeam, seam, dashLengths, 2);
            g.setColour (juce::Colour (0xff554433).withAlpha (0.5f));
            g.fillPath (dashedSeam);
        }

        // Chrome corner brackets.
        auto drawCorner = [&] (float cx, float cy, float startAngle)
        {
            constexpr float size = 30.0f;
            juce::Path corner;
            corner.addPieSegment (cx - size, cy - size, size * 2.0f, size * 2.0f,
                                   startAngle, startAngle + juce::MathConstants<float>::halfPi, 0.5f);
            juce::ColourGradient grad (chromeLight, cx, cy, chromeDark, cx + size, cy + size, false);
            g.setGradientFill (grad);
            g.fillPath (corner);
        };
        drawCorner (18.0f, 18.0f, juce::MathConstants<float>::pi);
        drawCorner ((float) getWidth() - 18.0f, 18.0f, juce::MathConstants<float>::halfPi);
        drawCorner (18.0f, (float) getHeight() - 18.0f, juce::MathConstants<float>::pi + juce::MathConstants<float>::halfPi);
        drawCorner ((float) getWidth() - 18.0f, (float) getHeight() - 18.0f, 0.0f);

        // Real photographed leather carrying handle, arched over the top edge.
        {
            const auto& handleImage = UIAssets::getHandle();
            if (handleImage.isValid())
            {
                const float handleWidth = 200.0f;
                const float aspect = (float) handleImage.getHeight() / (float) handleImage.getWidth();
                const float handleHeight = handleWidth * aspect;
                juce::Rectangle<float> target (0.0f, 0.0f, handleWidth, handleHeight);
                target.setCentre ({ getWidth() * 0.5f, 18.0f });
                g.drawImage (handleImage, target, juce::RectanglePlacement::centred);
            }
        }

        // Real photographed sparkle grille cloth, tiled to fill the grille
        // strip - baked into the cached background since it's static (only
        // the VU needle above it animates).
        {
            auto grille = getGrilleBounds().toFloat();
            const auto& grilleImage = UIAssets::getGrille();
            if (grilleImage.isValid())
            {
                g.setFillType (juce::FillType (grilleImage, juce::AffineTransform::translation (
                    grille.getX(), grille.getY())));
                g.fillRoundedRectangle (grille, 6.0f);
                g.setFillType (juce::FillType (juce::Colours::black));
            }
            else
            {
                g.setColour (juce::Colour (0xff2b261f));
                g.fillRoundedRectangle (grille, 6.0f);
            }

            g.setColour (juce::Colour (0xff0d0d0d));
            g.drawRoundedRectangle (grille, 6.0f, 2.0f);
        }
    }

    void AuraAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.drawImageAt (backgroundImage, 0, 0);

        auto panel = getPanelBounds();

        // Worn steel backplate behind every module panel - the modules' own
        // gaps show this through, so the panel reads as one continuous aged
        // metal control plate rather than separate floating tiles. Tile
        // origin is the editor's own (0,0) - ModulePanel uses the matching
        // -getX()/-getY() offset so its own steel fill lines up exactly
        // with this one instead of restarting at each panel's corner.
        const auto& steelTexture = UIAssets::getSteelPanel();
        if (steelTexture.isValid())
        {
            g.setFillType (juce::FillType (steelTexture, juce::AffineTransform()));
            g.fillRoundedRectangle (panel.toFloat(), 10.0f);
            g.setFillType (juce::FillType (juce::Colours::black));
        }
        else
        {
            juce::ColourGradient panelGradient (creamTop, 0.0f, (float) panel.getY(),
                                                 creamBottom, 0.0f, (float) panel.getBottom(), false);
            g.setGradientFill (panelGradient);
            g.fillRoundedRectangle (panel.toFloat(), 10.0f);
        }
        g.setColour (tanBorder.withAlpha (0.7f));
        g.drawRoundedRectangle (panel.toFloat().reduced (1.0f), 10.0f, 1.5f);

        // Header: brand wordmark, jewel power light. Engraved-metal look -
        // pale parchment ink with a soft dark shadow so it reads clearly
        // against the worn steel instead of the old dark-ink-on-cream look.
        auto drawEngraved = [&] (const juce::String& text, juce::Rectangle<int> area, juce::Justification j)
        {
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.drawText (text, area.translated (0, 1), j);
            g.setColour (engravedText);
            g.drawText (text, area, j);
        };

        g.setFont (juce::Font (juce::FontOptions (15.0f)).boldened().withExtraKerningFactor (0.18f));
        drawEngraved ("PREMOLI LABS", { panel.getX() + 24, panel.getY() + 16, 260, 20 }, juce::Justification::centredLeft);

        g.setFont (juce::Font (juce::FontOptions (36.0f)).italicised().boldened());
        drawEngraved ("AURA", { panel.getX() + 22, panel.getY() + 34, 220, 44 }, juce::Justification::centredLeft);

        {
            juce::Rectangle<float> jewel (30.0f, 30.0f);
            jewel.setCentre ({ (float) panel.getX() + 250.0f, (float) panel.getY() + (float) headerHeight * 0.5f });

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
        drawEngraved ("INSTRUMENT", instrumentBox.getBounds().translated (0, -16).withHeight (14),
                      juce::Justification::centred);
        drawEngraved ("OUTPUT LEVEL", outputMeter.getBounds().translated (0, -16).withHeight (14),
                      juce::Justification::centred);
    }

    void AuraAudioProcessorEditor::resized()
    {
        auto panel = getPanelBounds();
        auto header = panel.removeFromTop (headerHeight);

        instrumentBox.setBounds (header.removeFromRight (190).reduced (14, 30));
        outputMeter.setBounds (header.removeFromRight (130).reduced (8, 18));

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
