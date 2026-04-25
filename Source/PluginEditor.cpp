#include "PluginEditor.h"

namespace colours
{
    static const auto background = juce::Colour (0xff0f0f1a);
    static const auto noteBox    = juce::Colour (0xff16213e);
    static const auto accent     = juce::Colour (0xff0f3460);
    static const auto highlight  = juce::Colour (0xff533483);
    static const auto text       = juce::Colour (0xffe4e4e4);
    static const auto textDim    = juce::Colour (0xff888888);
    static const auto success    = juce::Colour (0xff00b894);
    static const auto surface    = juce::Colour (0xff1a1a2e);
    static const auto btnText    = juce::Colour (0xffffffff);
    static const auto stopBg     = juce::Colour (0xffe17055);
    static const auto error      = juce::Colour (0xffff4757);
}

static constexpr int kTabBarH  = 45;
static constexpr int kButtonH  = 60;

//==============================================================================
PianoSpeedrunEditor::PianoSpeedrunEditor (PianoSpeedrunProcessor& p)
    : AudioProcessorEditor (&p)
{
    setSize (420, 600);

    // --- tab buttons ---
    speedRunTab.setColour (juce::TextButton::buttonColourId, colours::accent);
    speedRunTab.setColour (juce::TextButton::textColourOffId, colours::btnText);
    speedRunTab.onClick = [this] { switchTab (Tab::SpeedRun); };
    addAndMakeVisible (speedRunTab);

    sightReadTab.setColour (juce::TextButton::buttonColourId, colours::surface);
    sightReadTab.setColour (juce::TextButton::textColourOffId, colours::btnText);
    sightReadTab.onClick = [this] { switchTab (Tab::SightRead); };
    addAndMakeVisible (sightReadTab);

    // --- speedrun controls ---
    octaveLabel.setColour (juce::Label::textColourId, colours::text);
    octaveLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (octaveLabel);

    for (int i = 1; i <= 7; ++i)
        octaveSelector.addItem (juce::String (i), i);
    octaveSelector.setSelectedId (2);
    octaveSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    octaveSelector.setColour (juce::ComboBox::textColourId, colours::text);
    octaveSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    addAndMakeVisible (octaveSelector);

    startStopButton.setColour (juce::TextButton::buttonColourId, colours::accent);
    startStopButton.setColour (juce::TextButton::textColourOffId, colours::btnText);
    startStopButton.onClick = [this]
    {
        auto& proc = getProcessor();
        if (isRunning)
        {
            proc.trainer.stop();
            isRunning = false;
            startStopButton.setButtonText ("START");
            startStopButton.setColour (juce::TextButton::buttonColourId, colours::accent);
            stopTimer();
        }
        else
        {
            proc.trainer.start (octaveSelector.getSelectedId());
            isRunning = true;
            startStopButton.setButtonText ("STOP");
            startStopButton.setColour (juce::TextButton::buttonColourId, colours::stopBg);
            startTimer (33);
        }
        repaint();
    };
    addAndMakeVisible (startStopButton);

    exportButton.setColour (juce::TextButton::buttonColourId, colours::surface);
    exportButton.setColour (juce::TextButton::textColourOffId, colours::btnText);
    exportButton.onClick = [this] { exportSession(); };
    addAndMakeVisible (exportButton);

    // --- sight-read controls ---
    notesLabel.setColour (juce::Label::textColourId, colours::text);
    notesLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (notesLabel);

    for (int i = 1; i <= 3; ++i)
        notesSelector.addItem (juce::String (i), i);
    notesSelector.setSelectedId (1);
    notesSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    notesSelector.setColour (juce::ComboBox::textColourId, colours::text);
    notesSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    notesSelector.onChange = [this]
    {
        bool single = (notesSelector.getSelectedId() == 1);
        bool sight = (currentTab == Tab::SightRead);
        modeSelector.setVisible (sight && ! single);
        spreadLabel.setVisible (sight && ! single);
        spreadSelector.setVisible (sight && ! single);
        resized();
        repaint();
    };
    addAndMakeVisible (notesSelector);

    modeSelector.addItem ("Exact", 1);
    modeSelector.addItem ("Up to", 2);
    modeSelector.setSelectedId (1);
    modeSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    modeSelector.setColour (juce::ComboBox::textColourId, colours::text);
    modeSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    addAndMakeVisible (modeSelector);

    spreadLabel.setColour (juce::Label::textColourId, colours::text);
    spreadLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (spreadLabel);

    spreadSelector.addItem ("m3", 3);
    spreadSelector.addItem ("M3", 4);
    spreadSelector.addItem ("P4", 5);
    spreadSelector.addItem ("P5", 7);
    spreadSelector.addItem ("M6", 9);
    spreadSelector.addItem ("Oct", 12);
    spreadSelector.setSelectedId (7);
    spreadSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    spreadSelector.setColour (juce::ComboBox::textColourId, colours::text);
    spreadSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    addAndMakeVisible (spreadSelector);

    sheetStartStop.setColour (juce::TextButton::buttonColourId, colours::accent);
    sheetStartStop.setColour (juce::TextButton::textColourOffId, colours::btnText);
    sheetStartStop.onClick = [this]
    {
        auto& proc = getProcessor();
        if (sheetRunning)
        {
            proc.sheetTrainer.stop();
            sheetRunning = false;
            sheetStartStop.setButtonText ("START");
            sheetStartStop.setColour (juce::TextButton::buttonColourId, colours::accent);
            stopTimer();
        }
        else
        {
            proc.sheetTrainer.start (notesSelector.getSelectedId(),
                                     spreadSelector.getSelectedId(),
                                     modeSelector.getSelectedId() == 1);
            sheetRunning = true;
            sheetStartStop.setButtonText ("STOP");
            sheetStartStop.setColour (juce::TextButton::buttonColourId, colours::stopBg);
            startTimer (33);
        }
        repaint();
    };
    addAndMakeVisible (sheetStartStop);

    switchTab (Tab::SpeedRun);
}

PianoSpeedrunEditor::~PianoSpeedrunEditor()
{
    stopTimer();
}

//==============================================================================
void PianoSpeedrunEditor::switchTab (Tab tab)
{
    if (currentTab != tab)
    {
        if (isRunning)
        {
            getProcessor().trainer.stop();
            isRunning = false;
            startStopButton.setButtonText ("START");
            startStopButton.setColour (juce::TextButton::buttonColourId, colours::accent);
        }
        if (sheetRunning)
        {
            getProcessor().sheetTrainer.stop();
            sheetRunning = false;
            sheetStartStop.setButtonText ("START");
            sheetStartStop.setColour (juce::TextButton::buttonColourId, colours::accent);
        }
        stopTimer();
    }

    currentTab = tab;

    speedRunTab.setColour (juce::TextButton::buttonColourId,
        tab == Tab::SpeedRun ? colours::accent : colours::surface);
    sightReadTab.setColour (juce::TextButton::buttonColourId,
        tab == Tab::SightRead ? colours::accent : colours::surface);

    bool sr = (tab == Tab::SpeedRun);
    bool single = (notesSelector.getSelectedId() == 1);

    octaveLabel.setVisible (sr);
    octaveSelector.setVisible (sr);
    startStopButton.setVisible (sr);
    exportButton.setVisible (sr);

    notesLabel.setVisible (! sr);
    notesSelector.setVisible (! sr);
    modeSelector.setVisible (! sr && ! single);
    spreadLabel.setVisible (! sr && ! single);
    spreadSelector.setVisible (! sr && ! single);
    sheetStartStop.setVisible (! sr);

    resized();
    repaint();
}

//==============================================================================
void PianoSpeedrunEditor::timerCallback()
{
    if (currentTab == Tab::SpeedRun)
    {
        if (getProcessor().trainer.getState() == NoteTrainer::State::Idle)
        {
            isRunning = false;
            startStopButton.setButtonText ("START");
            startStopButton.setColour (juce::TextButton::buttonColourId, colours::accent);
            stopTimer();
        }
    }
    else
    {
        getProcessor().sheetTrainer.advanceIfComplete();
    }
    repaint();
}

//==============================================================================
void PianoSpeedrunEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::background);

    auto area = getLocalBounds();
    area.removeFromTop (kTabBarH);
    area.removeFromBottom (kButtonH);

    if (currentTab == Tab::SpeedRun)
        paintSpeedRun (g, area);
    else
        paintSightRead (g, area);
}

//==============================================================================
void PianoSpeedrunEditor::paintSpeedRun (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& trainer = getProcessor().trainer;

    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (24.0f).withStyle ("Bold")));
    g.drawText ("PIANO SPEEDRUN", area.removeFromTop (50), juce::Justification::centred);

    area.removeFromTop (40); // octave config row

    auto noteArea = area.removeFromTop (160).reduced (60, 10);

    auto now = std::chrono::steady_clock::now();
    auto msSinceSuccess = std::chrono::duration_cast<std::chrono::milliseconds> (
        now - trainer.getLastCompletionTime()).count();
    auto msSinceWrong = std::chrono::duration_cast<std::chrono::milliseconds> (
        now - trainer.getLastWrongNoteTime()).count();

    bool successFlash = msSinceSuccess < 250;
    bool wrongFlash   = msSinceWrong < 400;

    auto boxBg   = colours::noteBox;
    auto boxEdge = colours::accent;

    if (wrongFlash)
    {
        boxBg   = colours::error.withAlpha (0.25f);
        boxEdge = colours::error;
    }
    else if (successFlash)
    {
        boxBg   = colours::success.withAlpha (0.3f);
        boxEdge = colours::success;
    }

    g.setColour (boxBg);
    g.fillRoundedRectangle (noteArea.toFloat(), 12.0f);
    g.setColour (boxEdge);
    g.drawRoundedRectangle (noteArea.toFloat(), 12.0f, 2.0f);

    if (isRunning)
    {
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (80.0f).withStyle ("Bold")));
        g.drawText (trainer.getCurrentNoteName(), noteArea,
                    juce::Justification::centred);

        if (wrongFlash)
        {
            auto wrongLabel = noteArea.removeFromTop (28).removeFromRight (50).reduced (4, 4);
            g.setColour (colours::error);
            g.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
            g.drawText (trainer.getLastWrongNoteName(), wrongLabel,
                        juce::Justification::centredRight);
        }
    }
    else
    {
        g.setColour (colours::textDim);
        g.setFont (juce::Font (juce::FontOptions (26.0f)));
        g.drawText ("Press START", noteArea, juce::Justification::centred);
    }

    area.removeFromTop (10);

    if (isRunning)
    {
        int hit = trainer.getOctavesHit();
        int req = trainer.getOctavesRequired();

        if (req >= 2)
        {
            auto dotRow = area.removeFromTop (30);
            float dotSize = 14.0f;
            float gap = 10.0f;
            float totalW = req * dotSize + (req - 1) * gap;
            float startX = dotRow.getCentreX() - totalW / 2.0f;
            float cy = dotRow.getCentreY() - dotSize / 2.0f;

            for (int i = 0; i < req; ++i)
            {
                float x = startX + (float) i * (dotSize + gap);
                if (i < hit)
                {
                    g.setColour (colours::success);
                    g.fillEllipse (x, cy, dotSize, dotSize);
                }
                else
                {
                    g.setColour (colours::accent.brighter (0.3f));
                    g.drawEllipse (x, cy, dotSize, dotSize, 2.0f);
                }
            }
        }
        else
        {
            area.removeFromTop (30);
        }

        double elapsed = trainer.getCurrentElapsedMs();
        g.setColour (colours::highlight);
        g.setFont (juce::Font (juce::FontOptions (32.0f).withStyle ("Bold")));
        g.drawText (juce::String (elapsed / 1000.0, 3) + "s",
                    area.removeFromTop (40), juce::Justification::centred);
    }
    else
    {
        area.removeFromTop (70);
    }

    area.removeFromTop (10);

    int rounds    = trainer.getRoundCount();
    double last   = trainer.getLastRoundTimeMs();
    double avg    = trainer.getAverageTimeMs();
    int wrongs    = trainer.getTotalWrongNotes();
    auto missed   = trainer.getMostMissedNote();
    int missedN   = trainer.getMostMissedCount();

    g.setColour (colours::textDim);
    g.setFont (juce::Font (juce::FontOptions (15.0f)));

    if (rounds > 0 || wrongs > 0)
    {
        g.drawText ("Last round:    " + (rounds > 0 ? juce::String (last / 1000.0, 3) + "s" : juce::String ("--")),
                    area.removeFromTop (23), juce::Justification::centred);
        g.drawText ("Average:         " + (rounds > 0 ? juce::String (avg / 1000.0, 3) + "s" : juce::String ("--")),
                    area.removeFromTop (23), juce::Justification::centred);
        g.drawText ("Rounds:          " + juce::String (rounds),
                    area.removeFromTop (23), juce::Justification::centred);

        g.setColour (wrongs > 0 ? colours::error.withAlpha (0.8f) : colours::textDim);
        g.drawText ("Wrong notes:  " + juce::String (wrongs),
                    area.removeFromTop (23), juce::Justification::centred);

        if (missedN > 0)
        {
            g.setColour (colours::error.withAlpha (0.8f));
            g.drawText ("Most missed:  " + missed + " (" + juce::String (missedN) + "x)",
                        area.removeFromTop (23), juce::Justification::centred);
        }
    }
    else
    {
        g.drawText ("No rounds completed yet",
                    area.removeFromTop (115), juce::Justification::centred);
    }
}

//==============================================================================
void PianoSpeedrunEditor::paintSightRead (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& trainer = getProcessor().sheetTrainer;

    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (24.0f).withStyle ("Bold")));
    g.drawText ("SIGHT READ", area.removeFromTop (50), juce::Justification::centred);

    area.removeFromTop (40); // config controls row

    // staff background
    auto staffRect = area.removeFromTop (200).reduced (10, 15);
    g.setColour (colours::noteBox);
    g.fillRoundedRectangle (staffRect.toFloat().expanded (6), 10.0f);

    if (sheetRunning)
    {
        auto notes  = trainer.getBarNotes();
        auto done   = trainer.getCompleted();
        auto hits   = trainer.getNoteHit();
        auto counts = trainer.getBeatNoteCount();
        int idx     = trainer.getCurrentNoteIndex();

        auto now = std::chrono::steady_clock::now();
        auto msSinceWrong = std::chrono::duration_cast<std::chrono::milliseconds> (
            now - trainer.getLastWrongTime()).count();

        if (msSinceWrong < 300)
        {
            g.setColour (colours::error.withAlpha (0.12f));
            g.fillRoundedRectangle (staffRect.toFloat().expanded (6), 10.0f);
        }

        drawStaff (g, staffRect.toFloat(), &notes, &done, &hits, &counts, idx);
    }
    else
    {
        drawStaff (g, staffRect.toFloat(), nullptr, nullptr, nullptr, nullptr, -1);

        g.setColour (colours::textDim);
        g.setFont (juce::Font (juce::FontOptions (22.0f)));
        g.drawText ("Press START", staffRect, juce::Justification::centred);
    }

    area.removeFromTop (20);

    int bars   = trainer.getBarsCompleted();
    int wrongs = trainer.getTotalWrongNotes();

    if (bars > 0 || wrongs > 0 || sheetRunning)
    {
        g.setColour (colours::textDim);
        g.setFont (juce::Font (juce::FontOptions (15.0f)));
        g.drawText ("Bars completed:  " + juce::String (bars),
                    area.removeFromTop (23), juce::Justification::centred);

        g.setColour (wrongs > 0 ? colours::error.withAlpha (0.8f) : colours::textDim);
        g.drawText ("Wrong notes:     " + juce::String (wrongs),
                    area.removeFromTop (23), juce::Justification::centred);
    }
}

//==============================================================================
void PianoSpeedrunEditor::drawStaff (juce::Graphics& g, juce::Rectangle<float> bounds,
                                      const std::array<std::array<int, 3>, 4>* notes,
                                      const std::array<bool, 4>* completed,
                                      const std::array<std::array<bool, 3>, 4>* noteHit,
                                      const std::array<int, 4>* beatCounts,
                                      int currentIndex)
{
    const float lineSpacing = 14.0f;
    const float staffHeight = 4.0f * lineSpacing;
    const float staffCY     = bounds.getCentreY();
    const float staffTop    = staffCY - staffHeight / 2.0f;
    const float bottomLineY = staffTop + staffHeight;
    const float staffL      = bounds.getX() + 8.0f;
    const float staffR      = bounds.getRight() - 8.0f;

    // five staff lines
    g.setColour (colours::textDim.withAlpha (0.5f));
    for (int i = 0; i < 5; ++i)
    {
        float y = staffTop + (float) i * lineSpacing;
        g.drawLine (staffL, y, staffR, y, 1.0f);
    }

    // barlines
    g.drawLine (staffL, staffTop, staffL, bottomLineY, 1.5f);
    g.drawLine (staffR, staffTop, staffR, bottomLineY, 1.5f);

    // treble clef
    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (48.0f)));
    auto clefRect = juce::Rectangle<float> (staffL + 2, staffTop - lineSpacing * 1.5f,
                                             30.0f, staffHeight + lineSpacing * 3.0f);
    g.drawText (juce::String::fromUTF8 ("\xF0\x9D\x84\x9E"),
                clefRect.toNearestInt(), juce::Justification::centred);

    // 4/4 time signature
    float tsX = staffL + 36.0f;
    g.setFont (juce::Font (juce::FontOptions (22.0f).withStyle ("Bold")));
    g.drawText ("4",
                juce::Rectangle<float> (tsX, staffTop, 18.0f, staffHeight / 2.0f).toNearestInt(),
                juce::Justification::centred);
    g.drawText ("4",
                juce::Rectangle<float> (tsX, staffTop + staffHeight / 2.0f, 18.0f, staffHeight / 2.0f).toNearestInt(),
                juce::Justification::centred);

    if (notes == nullptr)
        return;

    // note layout
    const float notesStart = staffL + 65.0f;
    const float notesEnd   = staffR - 15.0f;
    const float noteGap    = (notesEnd - notesStart) / 4.0f;
    const float nhW        = lineSpacing * 1.4f;
    const float nhH        = lineSpacing * 0.9f;
    const float stemLen    = lineSpacing * 3.5f;

    for (int i = 0; i < 4; ++i)
    {
        float cx = notesStart + noteGap * ((float) i + 0.5f);
        int bpc = beatCounts != nullptr ? (*beatCounts)[(size_t) i] : 1;

        // collect staff positions for all notes in this beat
        int spArr[3] {};
        int spMin = 999, spMax = -999;
        for (int n = 0; n < bpc; ++n)
        {
            spArr[n] = SheetTrainer::staffPosition ((*notes)[(size_t) i][(size_t) n]);
            spMin = juce::jmin (spMin, spArr[n]);
            spMax = juce::jmax (spMax, spArr[n]);
        }

        // ledger lines below staff (draw once for the range)
        if (spMin < 0)
        {
            g.setColour (colours::textDim.withAlpha (0.5f));
            for (int lp = -2; lp >= spMin; lp -= 2)
            {
                float ly = bottomLineY - (float) lp * (lineSpacing / 2.0f);
                g.drawLine (cx - nhW * 0.8f, ly, cx + nhW * 0.8f, ly, 1.0f);
            }
        }

        // ledger lines above staff
        if (spMax > 8)
        {
            g.setColour (colours::textDim.withAlpha (0.5f));
            for (int lp = 10; lp <= spMax; lp += 2)
            {
                float ly = bottomLineY - (float) lp * (lineSpacing / 2.0f);
                g.drawLine (cx - nhW * 0.8f, ly, cx + nhW * 0.8f, ly, 1.0f);
            }
        }

        // draw noteheads with per-note coloring
        for (int n = 0; n < bpc; ++n)
        {
            juce::Colour col;
            if ((*completed)[(size_t) i])
                col = colours::success;
            else if (i == currentIndex && noteHit != nullptr)
                col = (*noteHit)[(size_t) i][(size_t) n] ? colours::success : juce::Colours::white;
            else if (i == currentIndex)
                col = juce::Colours::white;
            else
                col = colours::text.withAlpha (0.55f);

            float cy = bottomLineY - (float) spArr[n] * (lineSpacing / 2.0f);
            g.setColour (col);
            g.fillEllipse (cx - nhW / 2.0f, cy - nhH / 2.0f, nhW, nhH);
        }

        // stem color (per-beat)
        juce::Colour stemCol;
        if ((*completed)[(size_t) i])
            stemCol = colours::success;
        else if (i == currentIndex)
            stemCol = juce::Colours::white;
        else
            stemCol = colours::text.withAlpha (0.55f);
        g.setColour (stemCol);

        if (bpc == 1)
        {
            // single note — original stem logic
            float cy = bottomLineY - (float) spArr[0] * (lineSpacing / 2.0f);
            if (spArr[0] < 4)
            {
                float sx = cx + nhW / 2.0f - 1.0f;
                g.drawLine (sx, cy, sx, cy - stemLen, 2.0f);
            }
            else
            {
                float sx = cx - nhW / 2.0f + 1.0f;
                g.drawLine (sx, cy, sx, cy + stemLen, 2.0f);
            }
        }
        else
        {
            // chord — shared stem spanning all noteheads
            float cyLow  = bottomLineY - (float) spMin * (lineSpacing / 2.0f);
            float cyHigh = bottomLineY - (float) spMax * (lineSpacing / 2.0f);

            float avgSp = 0.0f;
            for (int n = 0; n < bpc; ++n)
                avgSp += (float) spArr[n];
            avgSp /= (float) bpc;

            if (avgSp < 4.0f)
            {
                // stem up (right side)
                float sx = cx + nhW / 2.0f - 1.0f;
                g.drawLine (sx, cyLow, sx, cyHigh - stemLen, 2.0f);
            }
            else
            {
                // stem down (left side)
                float sx = cx - nhW / 2.0f + 1.0f;
                g.drawLine (sx, cyHigh, sx, cyLow + stemLen, 2.0f);
            }
        }
    }
}

//==============================================================================
void PianoSpeedrunEditor::resized()
{
    auto area = getLocalBounds();

    // tab bar
    auto tabBar = area.removeFromTop (kTabBarH).reduced (40, 8);
    int tw = tabBar.getWidth() / 2;
    speedRunTab.setBounds (tabBar.removeFromLeft (tw).reduced (4, 0));
    sightReadTab.setBounds (tabBar.reduced (4, 0));

    // bottom button area
    auto buttons = getLocalBounds().removeFromBottom (kButtonH).reduced (50, 10);

    if (currentTab == Tab::SpeedRun)
    {
        // skip title area (painted), then config row
        area.removeFromTop (50);
        auto config = area.removeFromTop (40).reduced (100, 5);
        octaveLabel.setBounds (config.removeFromLeft (80));
        config.removeFromLeft (10);
        octaveSelector.setBounds (config.removeFromLeft (70));

        int bw = (buttons.getWidth() - 20) / 2;
        startStopButton.setBounds (buttons.removeFromLeft (bw));
        buttons.removeFromLeft (20);
        exportButton.setBounds (buttons.removeFromLeft (bw));
    }
    else
    {
        // skip title area (painted), then config row
        area.removeFromTop (50);
        auto config = area.removeFromTop (40).reduced (10, 5);
        notesLabel.setBounds (config.removeFromLeft (48));
        config.removeFromLeft (4);
        notesSelector.setBounds (config.removeFromLeft (55));

        if (notesSelector.getSelectedId() > 1)
        {
            config.removeFromLeft (6);
            modeSelector.setBounds (config.removeFromLeft (72));
            config.removeFromLeft (10);
            spreadLabel.setBounds (config.removeFromLeft (50));
            config.removeFromLeft (4);
            spreadSelector.setBounds (config.removeFromLeft (65));
        }

        sheetStartStop.setBounds (buttons.reduced ((buttons.getWidth() - 140) / 2, 0));
    }
}

//==============================================================================
void PianoSpeedrunEditor::exportSession()
{
    auto results = getProcessor().trainer.getResults();
    if (results.empty())
        return;

    auto chooser = std::make_shared<juce::FileChooser> (
        "Export Session",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile ("piano-speedrun-session.csv"),
        "*.csv");

    chooser->launchAsync (
        juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles,
        [results, chooser] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File())
                return;

            juce::String csv;
            csv << "Round,Note,Octaves,Time (ms),Time (s)\n";

            for (int i = 0; i < (int) results.size(); ++i)
            {
                csv << juce::String (i + 1) << ","
                    << results[(size_t) i].noteName << ","
                    << juce::String (results[(size_t) i].octavesRequired) << ","
                    << juce::String (results[(size_t) i].timeMs, 1) << ","
                    << juce::String (results[(size_t) i].timeMs / 1000.0, 3) << "\n";
            }

            file.replaceWithText (csv);
        });
}
