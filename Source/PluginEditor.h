#pragma once
#include "PluginProcessor.h"

class PianoSpeedrunEditor : public juce::AudioProcessorEditor,
                            private juce::Timer
{
public:
    explicit PianoSpeedrunEditor (PianoSpeedrunProcessor&);
    ~PianoSpeedrunEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum class Tab { SpeedRun, SightRead };

    void timerCallback() override;
    void switchTab (Tab);
    void exportSession();
    void paintSpeedRun (juce::Graphics&, juce::Rectangle<int>);
    void paintSightRead (juce::Graphics&, juce::Rectangle<int>);
    void drawStaff (juce::Graphics&, juce::Rectangle<float>,
                    const std::array<std::array<int, 3>, 4>*,
                    const std::array<bool, 4>*,
                    const std::array<std::array<bool, 3>, 4>*,
                    const std::array<int, 4>*, int);

    PianoSpeedrunProcessor& getProcessor()
    {
        return static_cast<PianoSpeedrunProcessor&> (processor);
    }

    Tab currentTab = Tab::SpeedRun;

    juce::TextButton speedRunTab  { "SPEEDRUN" };
    juce::TextButton sightReadTab { "SIGHT READ" };

    juce::TextButton startStopButton { "START" };
    juce::TextButton exportButton    { "EXPORT" };
    juce::ComboBox   octaveSelector;
    juce::Label      octaveLabel     { {}, "Octaves:" };
    bool isRunning = false;

    juce::TextButton sheetStartStop { "START" };
    juce::ComboBox   notesSelector;
    juce::Label      notesLabel     { {}, "Notes:" };
    juce::ComboBox   modeSelector;
    juce::ComboBox   spreadSelector;
    juce::Label      spreadLabel    { {}, "Spread:" };
    bool sheetRunning = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PianoSpeedrunEditor)
};
