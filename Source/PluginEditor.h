#pragma once
#include "PluginProcessor.h"

class DanielsPianoHelperEditor : public juce::AudioProcessorEditor,
                            private juce::Timer
{
public:
    explicit DanielsPianoHelperEditor (DanielsPianoHelperProcessor&);
    ~DanielsPianoHelperEditor() override;

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
                    const std::array<std::array<int, 3>, 16>*,
                    const std::array<bool, 16>*,
                    const std::array<std::array<bool, 3>, 16>*,
                    const std::array<int, 16>*, int, int,
                    SheetTrainer::Clef);

    DanielsPianoHelperProcessor& getProcessor()
    {
        return static_cast<DanielsPianoHelperProcessor&> (processor);
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
    juce::ComboBox   barsSelector;
    juce::Label      barsLabel      { {}, "Bars:" };
    juce::ComboBox   clefSelector;
    juce::Label      clefLabel      { {}, "Clef:" };
    bool sheetRunning = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DanielsPianoHelperEditor)
};
