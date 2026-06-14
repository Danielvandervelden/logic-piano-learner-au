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
DanielsPianoHelperEditor::DanielsPianoHelperEditor (DanielsPianoHelperProcessor& p)
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

    barsLabel.setColour (juce::Label::textColourId, colours::text);
    barsLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (barsLabel);

    for (int i = 1; i <= 4; ++i)
        barsSelector.addItem (juce::String (i), i);
    barsSelector.setSelectedId (1);
    barsSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    barsSelector.setColour (juce::ComboBox::textColourId, colours::text);
    barsSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    barsSelector.onChange = [this]
    {
        if (currentTab == Tab::SightRead)
            setSize (540 + (barsSelector.getSelectedId() - 1) * 160,
                     selectedClef() == SheetTrainer::Clef::Grand ? 720 : 600);
    };
    addAndMakeVisible (barsSelector);

    clefLabel.setColour (juce::Label::textColourId, colours::text);
    clefLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (clefLabel);

    clefSelector.addItem ("Treble", 1);
    clefSelector.addItem ("Bass", 2);
    clefSelector.addItem ("Grand", 3);
    clefSelector.setSelectedId (1);
    clefSelector.setColour (juce::ComboBox::backgroundColourId, colours::surface);
    clefSelector.setColour (juce::ComboBox::textColourId, colours::text);
    clefSelector.setColour (juce::ComboBox::outlineColourId, colours::accent);
    clefSelector.onChange = [this]
    {
        if (currentTab == Tab::SightRead)
            setSize (540 + (barsSelector.getSelectedId() - 1) * 160,
                     selectedClef() == SheetTrainer::Clef::Grand ? 720 : 600);
        repaint();
    };
    addAndMakeVisible (clefSelector);

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
                                     modeSelector.getSelectedId() == 1,
                                     barsSelector.getSelectedId(),
                                     selectedClef());
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

DanielsPianoHelperEditor::~DanielsPianoHelperEditor()
{
    stopTimer();
}

//==============================================================================
void DanielsPianoHelperEditor::switchTab (Tab tab)
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
    barsLabel.setVisible (! sr);
    barsSelector.setVisible (! sr);
    clefLabel.setVisible (! sr);
    clefSelector.setVisible (! sr);
    sheetStartStop.setVisible (! sr);

    setSize (sr ? 420 : (540 + (barsSelector.getSelectedId() - 1) * 160),
             (! sr && selectedClef() == SheetTrainer::Clef::Grand) ? 720 : 600);
    resized();
    repaint();
}

//==============================================================================
void DanielsPianoHelperEditor::timerCallback()
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
void DanielsPianoHelperEditor::paint (juce::Graphics& g)
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
void DanielsPianoHelperEditor::paintSpeedRun (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& trainer = getProcessor().trainer;

    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (24.0f).withStyle ("Bold")));
    g.drawText ("SPEEDRUN", area.removeFromTop (50), juce::Justification::centred);

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
void DanielsPianoHelperEditor::paintSightRead (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& trainer = getProcessor().sheetTrainer;

    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (24.0f).withStyle ("Bold")));
    g.drawText ("SIGHT READ", area.removeFromTop (50), juce::Justification::centred);

    area.removeFromTop (40); // config controls row

    // staff background (grand staff needs more vertical room for two staves)
    bool grand = (sheetRunning ? trainer.getClef() == SheetTrainer::Clef::Grand
                               : selectedClef() == SheetTrainer::Clef::Grand);
    auto staffRect = area.removeFromTop (grand ? 340 : 200).reduced (10, 15);
    g.setColour (colours::noteBox);
    g.fillRoundedRectangle (staffRect.toFloat().expanded (6), 10.0f);

    if (sheetRunning)
    {
        auto notes  = trainer.getBarNotes();
        auto done   = trainer.getCompleted();
        auto hits   = trainer.getNoteHit();
        auto counts = trainer.getBeatNoteCount();
        auto nclef  = trainer.getNoteClef();
        int idx     = trainer.getCurrentNoteIndex();

        auto now = std::chrono::steady_clock::now();
        auto msSinceWrong = std::chrono::duration_cast<std::chrono::milliseconds> (
            now - trainer.getLastWrongTime()).count();

        if (msSinceWrong < 300)
        {
            g.setColour (colours::error.withAlpha (0.12f));
            g.fillRoundedRectangle (staffRect.toFloat().expanded (6), 10.0f);
        }

        int nb = trainer.getNumBars();
        drawStaff (g, staffRect.toFloat(), &notes, &done, &hits, &counts, &nclef, idx, nb,
                   trainer.getClef());
    }
    else
    {
        drawStaff (g, staffRect.toFloat(), nullptr, nullptr, nullptr, nullptr, nullptr, -1,
                   barsSelector.getSelectedId(), selectedClef());

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
void DanielsPianoHelperEditor::drawStaff (juce::Graphics& g, juce::Rectangle<float> bounds,
                                      const std::array<std::array<int, 6>, 16>* notes,
                                      const std::array<bool, 16>* completed,
                                      const std::array<std::array<bool, 6>, 16>* noteHit,
                                      const std::array<int, 16>* beatCounts,
                                      const std::array<std::array<SheetTrainer::Clef, 6>, 16>* noteClef,
                                      int currentIndex, int numBars,
                                      SheetTrainer::Clef clef)
{
    const float ls      = 14.0f;
    const float centreY = bounds.getCentreY();
    const float staffL  = bounds.getX() + 8.0f;
    const float staffR  = bounds.getRight() - 8.0f;

    const bool grand     = (clef == SheetTrainer::Clef::Grand);
    const int  numStaves = grand ? 2 : 1;

    // bottom-line Y and clef for each staff layer
    float bottomLineY[2];
    SheetTrainer::Clef staffClef[2];
    if (grand)
    {
        bottomLineY[0] = centreY - 3.0f * ls;   // treble staff (right hand)
        staffClef[0]   = SheetTrainer::Clef::Treble;
        bottomLineY[1] = centreY + 3.0f * ls;   // bass staff (left hand)
        staffClef[1]   = SheetTrainer::Clef::Bass;
    }
    else
    {
        bottomLineY[0] = centreY + 2.0f * ls;
        staffClef[0]   = clef;
    }

    const float systemTop    = bottomLineY[0] - 4.0f * ls;
    const float systemBottom = bottomLineY[numStaves - 1];

    // horizontal layout shared by both staves so beat columns line up
    const float notesStart = staffL + 65.0f;
    const float notesEnd   = staffR - 15.0f;
    const float barWidth   = (notesEnd - notesStart) / (float) numBars;

    // barlines span the whole system (joins the staves in grand mode)
    g.setColour (colours::textDim.withAlpha (0.5f));
    g.drawLine (staffL, systemTop, staffL, systemBottom, 1.5f);
    g.drawLine (staffR, systemTop, staffR, systemBottom, 1.5f);
    for (int b = 1; b < numBars; ++b)
    {
        float bx = notesStart + barWidth * (float) b;
        g.drawLine (bx, systemTop, bx, systemBottom, 1.5f);
    }

    // bracket joining the two staves (grand staff only)
    if (grand)
    {
        float xb = staffL - 4.0f;
        g.setColour (colours::text);
        g.drawLine (xb, systemTop, xb, systemBottom, 2.5f);
        g.drawLine (xb, systemTop, xb + 6.0f, systemTop, 2.5f);
        g.drawLine (xb, systemBottom, xb + 6.0f, systemBottom, 2.5f);
    }

    for (int s = 0; s < numStaves; ++s)
        drawStaffLayer (g, bottomLineY[s], ls, staffL, staffR, notesStart, barWidth, numBars,
                        staffClef[s], notes, completed, noteHit, beatCounts, noteClef, currentIndex);
}

//==============================================================================
void DanielsPianoHelperEditor::drawStaffLayer (juce::Graphics& g, float bottomLineY, float ls,
                                      float staffL, float staffR, float notesStart, float barWidth, int numBars,
                                      SheetTrainer::Clef layerClef,
                                      const std::array<std::array<int, 6>, 16>* notes,
                                      const std::array<bool, 16>* completed,
                                      const std::array<std::array<bool, 6>, 16>* noteHit,
                                      const std::array<int, 16>* beatCounts,
                                      const std::array<std::array<SheetTrainer::Clef, 6>, 16>* noteClef,
                                      int currentIndex)
{
    const float staffHeight = 4.0f * ls;
    const float staffTop    = bottomLineY - staffHeight;

    // five staff lines
    g.setColour (colours::textDim.withAlpha (0.5f));
    for (int i = 0; i < 5; ++i)
    {
        float y = staffTop + (float) i * ls;
        g.drawLine (staffL, y, staffR, y, 1.0f);
    }

    // clef glyph (treble U+1D11E, bass U+1D122)
    g.setColour (colours::text);
    g.setFont (juce::Font (juce::FontOptions (48.0f)));
    if (layerClef == SheetTrainer::Clef::Bass)
    {
        auto clefRect = juce::Rectangle<float> (staffL + 2, staffTop - ls * 0.5f,
                                                 30.0f, staffHeight + ls);
        g.drawText (juce::String::fromUTF8 ("\xF0\x9D\x84\xA2"),
                    clefRect.toNearestInt(), juce::Justification::centred);
    }
    else
    {
        auto clefRect = juce::Rectangle<float> (staffL + 2, staffTop - ls * 1.5f,
                                                 30.0f, staffHeight + ls * 3.0f);
        g.drawText (juce::String::fromUTF8 ("\xF0\x9D\x84\x9E"),
                    clefRect.toNearestInt(), juce::Justification::centred);
    }

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

    const int totalBeats = numBars * SheetTrainer::kNotesPerBar;
    const float beatGap  = barWidth / (float) SheetTrainer::kNotesPerBar;
    const float nhW      = ls * 1.4f;
    const float nhH      = ls * 0.9f;
    const float stemLen  = ls * 3.5f;

    for (int i = 0; i < totalBeats; ++i)
    {
        int bar = i / SheetTrainer::kNotesPerBar;
        int beatInBar = i % SheetTrainer::kNotesPerBar;
        float cx = notesStart + barWidth * (float) bar + beatGap * ((float) beatInBar + 0.5f);
        int bpc = (*beatCounts)[(size_t) i];

        // collect only this staff's notes (in grand mode the other hand lives on the other staff)
        int spArr[6] {};
        int origIdx[6] {};
        int localCount = 0;
        int spMin = 999, spMax = -999;
        for (int n = 0; n < bpc; ++n)
        {
            if ((*noteClef)[(size_t) i][(size_t) n] != layerClef)
                continue;
            int sp = SheetTrainer::staffPosition ((*notes)[(size_t) i][(size_t) n], layerClef);
            spArr[localCount]   = sp;
            origIdx[localCount] = n;
            spMin = juce::jmin (spMin, sp);
            spMax = juce::jmax (spMax, sp);
            ++localCount;
        }

        if (localCount == 0)
            continue;

        // ledger lines below staff (draw once for the range)
        if (spMin < 0)
        {
            g.setColour (colours::textDim.withAlpha (0.5f));
            for (int lp = -2; lp >= spMin; lp -= 2)
            {
                float ly = bottomLineY - (float) lp * (ls / 2.0f);
                g.drawLine (cx - nhW * 0.8f, ly, cx + nhW * 0.8f, ly, 1.0f);
            }
        }

        // ledger lines above staff
        if (spMax > 8)
        {
            g.setColour (colours::textDim.withAlpha (0.5f));
            for (int lp = 10; lp <= spMax; lp += 2)
            {
                float ly = bottomLineY - (float) lp * (ls / 2.0f);
                g.drawLine (cx - nhW * 0.8f, ly, cx + nhW * 0.8f, ly, 1.0f);
            }
        }

        // draw noteheads with per-note coloring
        for (int k = 0; k < localCount; ++k)
        {
            int n = origIdx[k];
            juce::Colour col;
            if ((*completed)[(size_t) i])
                col = colours::success;
            else if (i == currentIndex && noteHit != nullptr)
                col = (*noteHit)[(size_t) i][(size_t) n] ? colours::success : juce::Colours::white;
            else if (i == currentIndex)
                col = juce::Colours::white;
            else
                col = colours::text.withAlpha (0.55f);

            float cy = bottomLineY - (float) spArr[k] * (ls / 2.0f);
            g.setColour (col);
            g.fillEllipse (cx - nhW / 2.0f, cy - nhH / 2.0f, nhW, nhH);
        }

        // stem color (per beat, per staff)
        juce::Colour stemCol;
        if ((*completed)[(size_t) i])
            stemCol = colours::success;
        else if (i == currentIndex)
            stemCol = juce::Colours::white;
        else
            stemCol = colours::text.withAlpha (0.55f);
        g.setColour (stemCol);

        if (localCount == 1)
        {
            // single note
            float cy = bottomLineY - (float) spArr[0] * (ls / 2.0f);
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
            // chord — shared stem spanning this staff's noteheads
            float cyLow  = bottomLineY - (float) spMin * (ls / 2.0f);
            float cyHigh = bottomLineY - (float) spMax * (ls / 2.0f);

            float avgSp = 0.0f;
            for (int k = 0; k < localCount; ++k)
                avgSp += (float) spArr[k];
            avgSp /= (float) localCount;

            if (avgSp < 4.0f)
            {
                float sx = cx + nhW / 2.0f - 1.0f;
                g.drawLine (sx, cyLow, sx, cyHigh - stemLen, 2.0f);
            }
            else
            {
                float sx = cx - nhW / 2.0f + 1.0f;
                g.drawLine (sx, cyHigh, sx, cyLow + stemLen, 2.0f);
            }
        }
    }
}

//==============================================================================
void DanielsPianoHelperEditor::resized()
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
        barsLabel.setBounds (config.removeFromLeft (36));
        config.removeFromLeft (4);
        barsSelector.setBounds (config.removeFromLeft (50));
        config.removeFromLeft (10);
        notesLabel.setBounds (config.removeFromLeft (42));
        config.removeFromLeft (4);
        notesSelector.setBounds (config.removeFromLeft (50));
        config.removeFromLeft (10);
        clefLabel.setBounds (config.removeFromLeft (36));
        config.removeFromLeft (4);
        clefSelector.setBounds (config.removeFromLeft (62));

        if (notesSelector.getSelectedId() > 1)
        {
            config.removeFromLeft (6);
            modeSelector.setBounds (config.removeFromLeft (65));
            config.removeFromLeft (8);
            spreadLabel.setBounds (config.removeFromLeft (48));
            config.removeFromLeft (4);
            spreadSelector.setBounds (config.removeFromLeft (58));
        }

        sheetStartStop.setBounds (buttons.reduced ((buttons.getWidth() - 140) / 2, 0));
    }
}

//==============================================================================
void DanielsPianoHelperEditor::exportSession()
{
    auto results = getProcessor().trainer.getResults();
    if (results.empty())
        return;

    auto chooser = std::make_shared<juce::FileChooser> (
        "Export Session",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
            .getChildFile ("daniels-piano-helper-session.csv"),
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
