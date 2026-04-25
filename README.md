# Daniels Piano Helper

A Logic Pro AU (Audio Unit) MIDI plugin for practicing piano note recognition and sight reading.

## Features

### SpeedRun Mode
Rapid-fire note recognition trainer. A random note name appears on screen and you play it on your MIDI keyboard.

- **Octave requirement** — configure how many octaves you must play each note in (1–7)
- **Weighted randomization** — notes you struggle with or play slowly appear more often
- **Live timer** — see elapsed time per round and running average
- **Wrong note tracking** — counts mistakes and highlights your most-missed note
- **Session export** — export round-by-round results to CSV

### Sight Read Mode
Read notes from a 4/4 staff and play them in order. Generates random bars of natural notes in the treble clef range (C4–A5).

- **1, 2, or 3 notes per beat** — practice single notes, intervals, or chords
- **Exact or "Up to" mode** — exact always shows N notes per beat; "up to" randomly varies between 1 and N
- **Complementary note selection** — multi-note beats use consonant intervals (3rds, 4ths, 5ths, 6ths) so chords sound musical
- **Configurable max spread** — control the widest interval between notes in a chord (minor 3rd through octave)
- **Wrong note resets the beat** — miss one note in a chord and you replay the whole chord
- **Auto-advance** — the next bar generates automatically after completing the current one
- **Wrong note flash** — visual feedback on mistakes

## Installation

Download the latest `.zip` from [Releases](https://github.com/Danielvandervelden/logic-piano-learner-au/releases), unzip, and copy `Daniels Piano Helper.component` to:

```
~/Library/Audio/Plug-Ins/Components/
```

Restart Logic Pro, then add "Daniels Piano Helper" as a MIDI effect on a software instrument track.

## Building from Source

Requires CMake 3.22+ and a C++17 compiler. JUCE is included as a git submodule.

```bash
git submodule update --init
mkdir build && cd build
cmake ..
cmake --build .
```

The AU plugin is automatically copied to `~/Library/Audio/Plug-Ins/Components/` after building.

## Tech

- **Framework**: JUCE 7
- **Format**: AU (Audio Unit)
- **Language**: C++17
- **Platform**: macOS 13+
