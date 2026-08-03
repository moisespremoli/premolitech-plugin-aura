#include "UIAssets.h"
#include "BinaryData.h"

namespace aura::UIAssets
{
    namespace
    {
        const juce::Image& loadOnce (juce::Image& cache, const void* data, int size)
        {
            if (! cache.isValid())
                cache = juce::ImageCache::getFromMemory (data, size);
            return cache;
        }
    }

    const juce::Image& getKnobFrame (int frameIndex)
    {
        struct FrameData { const char* data; int size; };
        static const FrameData frames[numKnobFrames] = {
        { BinaryData::knob_00_png, BinaryData::knob_00_pngSize },
        { BinaryData::knob_01_png, BinaryData::knob_01_pngSize },
        { BinaryData::knob_02_png, BinaryData::knob_02_pngSize },
        { BinaryData::knob_03_png, BinaryData::knob_03_pngSize },
        { BinaryData::knob_04_png, BinaryData::knob_04_pngSize },
        { BinaryData::knob_05_png, BinaryData::knob_05_pngSize },
        { BinaryData::knob_06_png, BinaryData::knob_06_pngSize },
        { BinaryData::knob_07_png, BinaryData::knob_07_pngSize },
        { BinaryData::knob_08_png, BinaryData::knob_08_pngSize },
        { BinaryData::knob_09_png, BinaryData::knob_09_pngSize },
        { BinaryData::knob_10_png, BinaryData::knob_10_pngSize },
        { BinaryData::knob_11_png, BinaryData::knob_11_pngSize },
        { BinaryData::knob_12_png, BinaryData::knob_12_pngSize },
        { BinaryData::knob_13_png, BinaryData::knob_13_pngSize },
        { BinaryData::knob_14_png, BinaryData::knob_14_pngSize },
        { BinaryData::knob_15_png, BinaryData::knob_15_pngSize },
        { BinaryData::knob_16_png, BinaryData::knob_16_pngSize },
        { BinaryData::knob_17_png, BinaryData::knob_17_pngSize },
        { BinaryData::knob_18_png, BinaryData::knob_18_pngSize },
        { BinaryData::knob_19_png, BinaryData::knob_19_pngSize },
        { BinaryData::knob_20_png, BinaryData::knob_20_pngSize },
        { BinaryData::knob_21_png, BinaryData::knob_21_pngSize },
        { BinaryData::knob_22_png, BinaryData::knob_22_pngSize },
        { BinaryData::knob_23_png, BinaryData::knob_23_pngSize },
        { BinaryData::knob_24_png, BinaryData::knob_24_pngSize },
        { BinaryData::knob_25_png, BinaryData::knob_25_pngSize },
        { BinaryData::knob_26_png, BinaryData::knob_26_pngSize },
        { BinaryData::knob_27_png, BinaryData::knob_27_pngSize },
        { BinaryData::knob_28_png, BinaryData::knob_28_pngSize },
        { BinaryData::knob_29_png, BinaryData::knob_29_pngSize },
        { BinaryData::knob_30_png, BinaryData::knob_30_pngSize },
        { BinaryData::knob_31_png, BinaryData::knob_31_pngSize },
        { BinaryData::knob_32_png, BinaryData::knob_32_pngSize },
        { BinaryData::knob_33_png, BinaryData::knob_33_pngSize },
        { BinaryData::knob_34_png, BinaryData::knob_34_pngSize },
        { BinaryData::knob_35_png, BinaryData::knob_35_pngSize },
        { BinaryData::knob_36_png, BinaryData::knob_36_pngSize },
        { BinaryData::knob_37_png, BinaryData::knob_37_pngSize },
        { BinaryData::knob_38_png, BinaryData::knob_38_pngSize },
        { BinaryData::knob_39_png, BinaryData::knob_39_pngSize },
        { BinaryData::knob_40_png, BinaryData::knob_40_pngSize },
        { BinaryData::knob_41_png, BinaryData::knob_41_pngSize },
        { BinaryData::knob_42_png, BinaryData::knob_42_pngSize },
        { BinaryData::knob_43_png, BinaryData::knob_43_pngSize },
        { BinaryData::knob_44_png, BinaryData::knob_44_pngSize },
        { BinaryData::knob_45_png, BinaryData::knob_45_pngSize },
        { BinaryData::knob_46_png, BinaryData::knob_46_pngSize },
        { BinaryData::knob_47_png, BinaryData::knob_47_pngSize },
        { BinaryData::knob_48_png, BinaryData::knob_48_pngSize },
        { BinaryData::knob_49_png, BinaryData::knob_49_pngSize },
        { BinaryData::knob_50_png, BinaryData::knob_50_pngSize },
        { BinaryData::knob_51_png, BinaryData::knob_51_pngSize },
        { BinaryData::knob_52_png, BinaryData::knob_52_pngSize },
        { BinaryData::knob_53_png, BinaryData::knob_53_pngSize },
        { BinaryData::knob_54_png, BinaryData::knob_54_pngSize },
        { BinaryData::knob_55_png, BinaryData::knob_55_pngSize },
        { BinaryData::knob_56_png, BinaryData::knob_56_pngSize },
        { BinaryData::knob_57_png, BinaryData::knob_57_pngSize },
        { BinaryData::knob_58_png, BinaryData::knob_58_pngSize },
        { BinaryData::knob_59_png, BinaryData::knob_59_pngSize },
        { BinaryData::knob_60_png, BinaryData::knob_60_pngSize },
        };

        static juce::Image cache[numKnobFrames];
        frameIndex = juce::jlimit (0, numKnobFrames - 1, frameIndex);
        return loadOnce (cache[frameIndex], frames[frameIndex].data, frames[frameIndex].size);
    }

    const juce::Image& getJewelRed()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::jewel_red_png, BinaryData::jewel_red_pngSize);
    }

    const juce::Image& getJewelGreen()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::jewel_green_png, BinaryData::jewel_green_pngSize);
    }

    const juce::Image& getHandle()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::handle_png, BinaryData::handle_pngSize);
    }

    const juce::Image& getGrille()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::grille_png, BinaryData::grille_pngSize);
    }

    const juce::Image& getSteelPanel()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::steel_panel_png, BinaryData::steel_panel_pngSize);
    }

    const juce::Image& getTolexPanel()
    {
        static juce::Image image;
        return loadOnce (image, BinaryData::tolex_panel_png, BinaryData::tolex_panel_pngSize);
    }
}
