cmake_minimum_required(VERSION 3.22)
project(PianoSpeedrun VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")

add_subdirectory(JUCE)

juce_add_plugin(PianoSpeedrun
    COMPANY_NAME "DanielVDV"
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Dvdv
    PLUGIN_CODE Pspr
    FORMATS AU
    PRODUCT_NAME "Piano Speedrun"
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(PianoSpeedrun)

target_sources(PianoSpeedrun
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(PianoSpeedrun
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PianoSpeedrun
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
