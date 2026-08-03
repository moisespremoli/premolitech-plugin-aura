#pragma once

#include <juce_graphics/juce_graphics.h>

namespace aura::UIAssets
{
    // Real photoreal art (processed from AI-generated reference images -
    // background removed, cropped, rotation-pivot-corrected where needed),
    // embedded as BinaryData. Loaded once and cached; every accessor is
    // cheap after the first call. Only called from the message thread
    // (paint/drawX callbacks), so the function-local statics need no locking.
    // Pre-rotated knob frames, one whole standalone image per frame,
    // covering the slider's full sweep from sliderPos 0 to 1. Drawn as a
    // plain whole-image scaled blit - no runtime rotation, no sub-region
    // sampling from a shared atlas. Getting a solid, correctly opaque
    // render out of this renderer took ruling out two other approaches
    // first: (1) rotating the image live (via drawImageTransformed or a
    // context-level addTransform) visibly washes it out even though the
    // source PNG is fully opaque, confirmed against an offline Python
    // re-sample of the same file staying solid black; and (2) drawing a
    // sub-rectangle out of a larger shared atlas washes it out too,
    // regardless of the atlas's aspect ratio (tried both an extreme
    // 19520x320 strip and a near-square 2560x2560 grid, both identical) -
    // whereas an isolated whole-image draw of a single frame always
    // rendered correctly. So: one frame, one standalone embedded file,
    // drawn whole with no source rect and no transform.
    constexpr int numKnobFrames = 61;
    const juce::Image& getKnobFrame (int frameIndex);
    const juce::Image& getJewelRed();
    const juce::Image& getJewelGreen();
    const juce::Image& getHandle();
    const juce::Image& getGrille();
    const juce::Image& getCreamPanel();
}
