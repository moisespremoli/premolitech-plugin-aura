cmake_minimum_required(VERSION 3.22)

project(AURA VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# AU só é gerado no macOS; nas demais plataformas o JUCE ignora o formato.
juce_add_plugin(AURA
    COMPANY_NAME "Premoli Labs"
    BUNDLE_ID "com.premolilabs.aura"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Mprm
    PLUGIN_CODE Aura
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Premoli Labs AURA")

target_sources(AURA PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/Modules/NoiseGateModule.cpp
    Source/DSP/Modules/CompressorModule.cpp
    Source/DSP/Modules/EQModule.cpp
    Source/DSP/Modules/LimiterModule.cpp
    Source/DSP/Amp/AmpModule.cpp
    Source/DSP/Amp/JCM800Model.cpp
    Source/DSP/Amp/FenderDeluxeModel.cpp
    Source/DSP/Amp/Peavey5150Model.cpp
    Source/DSP/Cab/CabModule.cpp)

target_include_directories(AURA PRIVATE Source)

# IR de cabinet placeholder, embutida como recurso binário até uma
# biblioteca de IRs real e licenciada ser integrada (Fase 3).
juce_add_binary_data(AURAAssets SOURCES
    Resources/IR/placeholder_cab.wav)

target_compile_definitions(AURA PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AURA
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        AURAAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
