#pragma once
#include <JuceHeader.h>
#include <array>
#include <chrono>

class SheetTrainer
{
public:
    enum class State { Idle, Active };
    static constexpr int kNotesPerBar = 4;
    static constexpr int kMaxNotesPerBeat = 3;

    void start (int numNotesPerBeat = 1, int spreadSemitones = 12, bool exact = true)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        notesPerBeat = juce::jlimit (1, 3, numNotesPerBeat);
        maxSpread = juce::jlimit (3, 21, spreadSemitones);
        exactMode = exact;
        state = State::Active;
        barsCompleted = 0;
        totalWrongNotes = 0;
        barComplete = false;
        generateBar();
    }

    void stop()
    {
        juce::SpinLock::ScopedLockType sl (lock);
        state = State::Idle;
    }

    bool handleNoteOn (int midiNote)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (state != State::Active || barComplete)
            return false;

        int count = beatNoteCount[(size_t) currentNoteIndex];

        bool found = false;
        for (int n = 0; n < count; ++n)
        {
            if (midiNote == barNotes[(size_t) currentNoteIndex][(size_t) n]
                && ! noteHit[(size_t) currentNoteIndex][(size_t) n])
            {
                noteHit[(size_t) currentNoteIndex][(size_t) n] = true;
                lastCorrectTime = std::chrono::steady_clock::now();
                found = true;
                break;
            }
        }

        if (! found)
        {
            totalWrongNotes++;
            lastWrongTime = std::chrono::steady_clock::now();
            for (int n = 0; n < count; ++n)
                noteHit[(size_t) currentNoteIndex][(size_t) n] = false;
            return false;
        }

        bool beatDone = true;
        for (int n = 0; n < count; ++n)
            if (! noteHit[(size_t) currentNoteIndex][(size_t) n])
            {
                beatDone = false;
                break;
            }

        if (beatDone)
        {
            completed[(size_t) currentNoteIndex] = true;
            currentNoteIndex++;

            if (currentNoteIndex >= kNotesPerBar)
            {
                barsCompleted++;
                barComplete = true;
                barCompleteTime = std::chrono::steady_clock::now();
            }
        }
        return true;
    }

    void advanceIfComplete()
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (! barComplete)
            return;

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now() - barCompleteTime).count();

        if (ms > 700)
        {
            barComplete = false;
            generateBar();
        }
    }

    State getState() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return state;
    }

    std::array<std::array<int, kMaxNotesPerBeat>, kNotesPerBar> getBarNotes() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return barNotes;
    }

    std::array<bool, kNotesPerBar> getCompleted() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return completed;
    }

    std::array<std::array<bool, kMaxNotesPerBeat>, kNotesPerBar> getNoteHit() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return noteHit;
    }

    int getNotesPerBeat() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return notesPerBeat;
    }

    std::array<int, kNotesPerBar> getBeatNoteCount() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return beatNoteCount;
    }

    int getCurrentNoteIndex() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return currentNoteIndex;
    }

    int getBarsCompleted() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return barsCompleted;
    }

    int getTotalWrongNotes() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return totalWrongNotes;
    }

    bool isBarComplete() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return barComplete;
    }

    std::chrono::steady_clock::time_point getLastCorrectTime() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastCorrectTime;
    }

    std::chrono::steady_clock::time_point getLastWrongTime() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastWrongTime;
    }

    static int staffPosition (int midiNote)
    {
        switch (midiNote)
        {
            case 60: return -2;  // C4
            case 62: return -1;  // D4
            case 64: return  0;  // E4 (bottom line)
            case 65: return  1;  // F4
            case 67: return  2;  // G4
            case 69: return  3;  // A4
            case 71: return  4;  // B4 (middle line)
            case 72: return  5;  // C5
            case 74: return  6;  // D5
            case 76: return  7;  // E5
            case 77: return  8;  // F5 (top line)
            case 79: return  9;  // G5
            case 81: return 10;  // A5
            default: return  0;
        }
    }

private:
    static bool isConsonant (int a, int b)
    {
        if (a == b)
            return false;
        int interval = std::abs (a - b) % 12;
        return interval == 0 || interval == 3 || interval == 4 || interval == 5
            || interval == 7 || interval == 8 || interval == 9;
    }

    void generateBar()
    {
        static constexpr int pool[] = { 60, 62, 64, 65, 67, 69, 71,
                                        72, 74, 76, 77, 79, 81 };
        static constexpr int poolSize = 13;

        for (int i = 0; i < kNotesPerBar; ++i)
        {
            int count = notesPerBeat;
            if (! exactMode && notesPerBeat > 1)
                count = 1 + random.nextInt (notesPerBeat);

            beatNoteCount[(size_t) i] = count;

            if (count == 1)
            {
                barNotes[(size_t) i][0] = pool[(size_t) random.nextInt (poolSize)];
            }
            else
            {
                pickChord (i, pool, poolSize, count);
            }

            completed[(size_t) i] = false;
            for (int n = 0; n < kMaxNotesPerBeat; ++n)
                noteHit[(size_t) i][(size_t) n] = false;
        }
        currentNoteIndex = 0;
    }

    void pickChord (int beatIndex, const int* pool, int poolSize, int count)
    {
        barNotes[(size_t) beatIndex][0] = pool[(size_t) random.nextInt (poolSize)];

        for (int n = 1; n < count; ++n)
        {
            int candidates[13];
            int candidateCount = 0;

            for (int p = 0; p < poolSize; ++p)
            {
                int candidate = pool[(size_t) p];

                bool used = false;
                for (int j = 0; j < n; ++j)
                    if (barNotes[(size_t) beatIndex][(size_t) j] == candidate)
                        used = true;
                if (used)
                    continue;

                bool consonant = true;
                for (int j = 0; j < n; ++j)
                    if (! isConsonant (candidate, barNotes[(size_t) beatIndex][(size_t) j]))
                    {
                        consonant = false;
                        break;
                    }
                if (! consonant)
                    continue;

                int lo = candidate, hi = candidate;
                for (int j = 0; j < n; ++j)
                {
                    lo = juce::jmin (lo, barNotes[(size_t) beatIndex][(size_t) j]);
                    hi = juce::jmax (hi, barNotes[(size_t) beatIndex][(size_t) j]);
                }
                if (hi - lo > maxSpread)
                    continue;

                candidates[candidateCount++] = candidate;
            }

            if (candidateCount > 0)
                barNotes[(size_t) beatIndex][(size_t) n] =
                    candidates[(size_t) random.nextInt (candidateCount)];
            else
                barNotes[(size_t) beatIndex][(size_t) n] =
                    barNotes[(size_t) beatIndex][0];
        }

        // sort notes low to high for display
        for (int a = 0; a < count - 1; ++a)
            for (int b = a + 1; b < count; ++b)
                if (barNotes[(size_t) beatIndex][(size_t) a]
                    > barNotes[(size_t) beatIndex][(size_t) b])
                    std::swap (barNotes[(size_t) beatIndex][(size_t) a],
                               barNotes[(size_t) beatIndex][(size_t) b]);
    }

    mutable juce::SpinLock lock;
    State state = State::Idle;
    std::array<std::array<int, kMaxNotesPerBeat>, kNotesPerBar> barNotes {};
    std::array<std::array<bool, kMaxNotesPerBeat>, kNotesPerBar> noteHit {};
    std::array<bool, kNotesPerBar> completed {};
    std::array<int, kNotesPerBar> beatNoteCount {};
    int notesPerBeat = 1;
    int maxSpread = 12;
    bool exactMode = true;
    int currentNoteIndex = 0;
    int barsCompleted = 0;
    int totalWrongNotes = 0;
    bool barComplete = false;
    std::chrono::steady_clock::time_point barCompleteTime;
    std::chrono::steady_clock::time_point lastCorrectTime;
    std::chrono::steady_clock::time_point lastWrongTime;
    juce::Random random;
};
