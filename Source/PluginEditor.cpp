#include "PluginEditor.h"
#include "Core/ParameterIDs.h"
#include "Core/Instrument.h"

namespace aura
{
    namespace
    {
        constexpr int headerHeight = 76;

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
        setSize (1040, 700);
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

    void AuraAudioProcessorEditor::rebuildBackgroundImage()
    {
        backgroundImage = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics g (backgroundImage);

        juce::ColourGradient baseGradient (juce::Colour (0xff2d2d31), 0.0f, 0.0f,
                                            juce::Colour (0xff141416), 0.0f, (float) getHeight(), false);
        g.setGradientFill (baseGradient);
        g.fillAll();

        // Brushed-metal effect: thin, low-alpha horizontal streaks with a
        // fixed random seed, so the texture is identical on every repaint
        // instead of shimmering.
        juce::Random rng (12345);
        for (int y = 0; y < getHeight(); ++y)
        {
            if (rng.nextFloat() < 0.35f)
            {
                g.setColour (juce::Colours::white.withAlpha (rng.nextFloat() * 0.02f));
                g.drawHorizontalLine (y, 0.0f, (float) getWidth());
            }
        }

        // Vignette so the edges read darker than the centre.
        juce::ColourGradient vignette (juce::Colours::transparentBlack,
                                       getWidth() * 0.5f, getHeight() * 0.5f,
                                       juce::Colours::black.withAlpha (0.4f), 0.0f, 0.0f, true);
        vignette.addColour (0.75, juce::Colours::transparentBlack);
        g.setGradientFill (vignette);
        g.fillAll();
    }

    void AuraAudioProcessorEditor::paint (juce::Graphics& g)
    {
        g.drawImageAt (backgroundImage, 0, 0);

        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawHorizontalLine (headerHeight, 0.0f, (float) getWidth());
        g.setColour (juce::Colours::white.withAlpha (0.04f));
        g.drawHorizontalLine (headerHeight + 1, 0.0f, (float) getWidth());

        g.setColour (juce::Colour (0xff9a9aa0));
        g.setFont (juce::Font (juce::FontOptions (13.0f)).withExtraKerningFactor (0.25f));
        g.drawText ("PREMOLI LABS", juce::Rectangle<int> (22, 14, 300, 20), juce::Justification::centredLeft);

        g.setColour (juce::Colours::whitesmoke);
        g.setFont (juce::Font (juce::FontOptions (30.0f)).boldened().withExtraKerningFactor (0.03f));
        g.drawText ("AURA", juce::Rectangle<int> (20, 32, 220, 36), juce::Justification::centredLeft);

        // Power jewel - lit green, like a real amp's "on" indicator.
        juce::Rectangle<float> powerLed (170.0f, 44.0f, 10.0f, 10.0f);
        g.setColour (juce::Colour (0xff0c0c0e));
        g.fillEllipse (powerLed.expanded (2.0f));
        juce::ColourGradient ledGlow (juce::Colour (0xff7ee08a), powerLed.getCentreX(), powerLed.getY(),
                                       juce::Colour (0xff2f8f3f), powerLed.getCentreX(), powerLed.getBottom(), false);
        g.setGradientFill (ledGlow);
        g.fillEllipse (powerLed);
        g.setColour (juce::Colour (0xff4caf50).withAlpha (0.30f));
        g.fillEllipse (powerLed.expanded (3.0f));

        g.setColour (juce::Colour (0xff9a9aa0));
        g.setFont (juce::Font (juce::FontOptions (10.5f)));
        g.drawText ("INSTRUMENT", instrumentBox.getBounds().translated (0, -16).withHeight (14),
                     juce::Justification::centred);
        g.drawText ("OUTPUT", outputMeter.getBounds().translated (0, -16).withHeight (14),
                     juce::Justification::centred);
    }

    void AuraAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds();

        auto header = bounds.removeFromTop (headerHeight);
        instrumentBox.setBounds (header.removeFromRight (220).reduced (20, 26));
        outputMeter.setBounds (header.removeFromRight (130).reduced (10, 18));

        bounds = bounds.reduced (10);

        const int rowHeight = (bounds.getHeight() - 20) / 3;
        auto row1 = bounds.removeFromTop (rowHeight);
        bounds.removeFromTop (10);
        auto row2 = bounds.removeFromTop (rowHeight);
        bounds.removeFromTop (10);
        auto row3 = bounds;

        layoutWeightedRow (row1, { { &inputPanel, 2 }, { &gatePanel, 4 }, { &compPanel, 6 } });
        layoutWeightedRow (row2, { { &ampPanel, 1 } });
        layoutWeightedRow (row3, { { &cabPanel, 3 }, { &eqPanel, 4 }, { &limiterPanel, 3 }, { &outputPanel, 2 } });
    }
}
