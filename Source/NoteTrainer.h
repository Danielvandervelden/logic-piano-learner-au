#pragma once
#include <JuceHeader.h>
#include <set>
#include <map>
#include <vector>
#include <chrono>

struct RoundResult
{
    juce::String noteName;
    int octavesRequired;
    double timeMs;
};

class NoteTrainer
{
public:
    enum class State { Idle, Active };

    void start (int numOctaves)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        results.clear();
        octavesRequired = juce::jlimit (1, 7, numOctaves);
        roundCount = 0;
        totalTimeMs = 0.0;
        lastRoundTimeMs = 0.0;
        totalWrongNotes = 0;
        wrongCountByTarget.clear();
        noteWeight.clear();
        state = State::Active;
        pickNextNote();
    }

    void stop()
    {
        juce::SpinLock::ScopedLockType sl (lock);
        state = State::Idle;
    }

    bool handleNoteOn (int midiNoteNumber)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (state != State::Active)
            return false;

        int noteClass = midiNoteNumber % 12;
        if (noteClass != targetNoteClass)
        {
            lastWrongNoteName = chromaticNoteName (noteClass);
            lastWrongNoteTime = std::chrono::steady_clock::now();
            totalWrongNotes++;
            wrongCountByTarget[targetNoteClass]++;
            noteWeight[targetNoteClass]++;
            return false;
        }

        int octave = midiNoteNumber / 12;
        octavesHit.insert (octave);

        if ((int) octavesHit.size() >= octavesRequired)
        {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double, std::milli> (now - roundStartTime).count();

            double avgBefore = roundCount > 0 ? totalTimeMs / roundCount : 0.0;
            bool wasSlow = roundCount >= 3 && elapsed > avgBefore * 1.5;

            noteWeight[targetNoteClass] /= 2;
            if (wasSlow)
                noteWeight[targetNoteClass]++;

            results.push_back ({ noteNameForClass (targetNoteClass), octavesRequired, elapsed });
            roundCount++;
            totalTimeMs += elapsed;
            lastRoundTimeMs = elapsed;
            lastCompletionTime = now;

            pickNextNote();
            return true;
        }
        return false;
    }

    State getState() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return state;
    }

    juce::String getCurrentNoteName() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return noteNameForClass (targetNoteClass);
    }

    int getOctavesHit() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return (int) octavesHit.size();
    }

    int getOctavesRequired() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return octavesRequired;
    }

    double getCurrentElapsedMs() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (state != State::Active)
            return 0.0;
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli> (now - roundStartTime).count();
    }

    double getLastRoundTimeMs() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastRoundTimeMs;
    }

    double getAverageTimeMs() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return roundCount > 0 ? totalTimeMs / roundCount : 0.0;
    }

    int getRoundCount() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return roundCount;
    }

    std::chrono::steady_clock::time_point getLastCompletionTime() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastCompletionTime;
    }

    std::vector<RoundResult> getResults() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return results;
    }

    int getTotalWrongNotes() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return totalWrongNotes;
    }

    juce::String getMostMissedNote() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (wrongCountByTarget.empty())
            return {};

        int worstClass = 0;
        int worstCount = 0;
        for (auto& [noteClass, count] : wrongCountByTarget)
        {
            if (count > worstCount)
            {
                worstClass = noteClass;
                worstCount = count;
            }
        }
        return noteNameForClass (worstClass);
    }

    int getMostMissedCount() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        int worst = 0;
        for (auto& [_, count] : wrongCountByTarget)
            worst = juce::jmax (worst, count);
        return worst;
    }

    juce::String getLastWrongNoteName() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastWrongNoteName;
    }

    std::chrono::steady_clock::time_point getLastWrongNoteTime() const
    {
        juce::SpinLock::ScopedLockType sl (lock);
        return lastWrongNoteTime;
    }

    static juce::String chromaticNoteName (int noteClass)
    {
        static const char* names[] = {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };
        return names[noteClass % 12];
    }

    static juce::String noteNameForClass (int noteClass)
    {
        switch (noteClass)
        {
            case 0:  return "C";
            case 2:  return "D";
            case 4:  return "E";
            case 5:  return "F";
            case 7:  return "G";
            case 9:  return "A";
            case 11: return "B";
            default: return "?";
        }
    }

private:
    void pickNextNote()
    {
        static constexpr int naturalNotes[] = { 0, 2, 4, 5, 7, 9, 11 };

        int totalWeight = 0;
        int weights[7] {};

        for (int i = 0; i < 7; ++i)
        {
            if (naturalNotes[i] == targetNoteClass && !results.empty())
            {
                weights[i] = 0;
            }
            else
            {
                int w = 1;
                auto it = noteWeight.find (naturalNotes[i]);
                if (it != noteWeight.end())
                    w += it->second;
                weights[i] = w;
            }
            totalWeight += weights[i];
        }

        int roll = random.nextInt (totalWeight);
        int cumulative = 0;

        for (int i = 0; i < 7; ++i)
        {
            cumulative += weights[i];
            if (roll < cumulative)
            {
                targetNoteClass = naturalNotes[i];
                break;
            }
        }

        octavesHit.clear();
        roundStartTime = std::chrono::steady_clock::now();
    }

    mutable juce::SpinLock lock;
    State state = State::Idle;
    int targetNoteClass = 0;
    int octavesRequired = 2;
    std::set<int> octavesHit;
    std::chrono::steady_clock::time_point roundStartTime;
    std::chrono::steady_clock::time_point lastCompletionTime;
    double lastRoundTimeMs = 0.0;
    double totalTimeMs = 0.0;
    int roundCount = 0;
    int totalWrongNotes = 0;
    std::map<int, int> wrongCountByTarget;
    std::map<int, int> noteWeight;
    juce::String lastWrongNoteName;
    std::chrono::steady_clock::time_point lastWrongNoteTime;
    std::vector<RoundResult> results;
    juce::Random random;
};
