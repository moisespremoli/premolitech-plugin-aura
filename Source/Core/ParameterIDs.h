#pragma once

// Central registry of every automatable parameter ID in the plugin.
// Keeping these in one place means a module and the editor component that
// controls it can never disagree on a string literal.
namespace aura::ParamIDs
{
    // Global
    constexpr auto instrument   = "instrument";
    constexpr auto inputGain    = "inputGain";
    constexpr auto outputGain   = "outputGain";

    // Noise gate
    constexpr auto gateBypass    = "gateBypass";
    constexpr auto gateThreshold = "gateThreshold";
    constexpr auto gateAttack    = "gateAttack";
    constexpr auto gateRelease   = "gateRelease";

    // Compressor
    constexpr auto compBypass    = "compBypass";
    constexpr auto compThreshold = "compThreshold";
    constexpr auto compRatio     = "compRatio";
    constexpr auto compAttack    = "compAttack";
    constexpr auto compRelease   = "compRelease";
    constexpr auto compMakeup    = "compMakeup";

    // Amp
    constexpr auto ampBypass   = "ampBypass";
    constexpr auto ampModel    = "ampModel";
    constexpr auto ampGain     = "ampGain";
    constexpr auto ampBass     = "ampBass";
    constexpr auto ampMid      = "ampMid";
    constexpr auto ampTreble   = "ampTreble";
    constexpr auto ampPresence = "ampPresence";
    constexpr auto ampMaster   = "ampMaster";

    // Cab / IR
    constexpr auto cabBypass = "cabBypass";
    constexpr auto cabMix    = "cabMix";

    // EQ (post-cab tone shaping)
    constexpr auto eqBypass = "eqBypass";
    constexpr auto eqLow    = "eqLow";
    constexpr auto eqMid    = "eqMid";
    constexpr auto eqHigh   = "eqHigh";

    // Limiter
    constexpr auto limiterBypass  = "limiterBypass";
    constexpr auto limiterCeiling = "limiterCeiling";
}
