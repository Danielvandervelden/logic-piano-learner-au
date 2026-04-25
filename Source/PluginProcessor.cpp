#include "PluginProcessor.h"
#include "PluginEditor.h"

PianoSpeedrunProcessor::PianoSpeedrunProcessor()
    : AudioProcessor (BusesProperties())
{
}

PianoSpeedrunProcessor::~PianoSpeedrunProcessor() {}

const juce::String PianoSpeedrunProcessor::getName() const { return "Piano Speedrun"; }
bool PianoSpeedrunProcessor::acceptsMidi() const { return true; }
bool PianoSpeedrunProcessor::producesMidi() const { return true; }
bool PianoSpeedrunProcessor::isMidiEffect() const { return true; }
double PianoSpeedrunProcessor::getTailLengthSeconds() const { return 0.0; }
int PianoSpeedrunProcessor::getNumPrograms() { return 1; }
int PianoSpeedrunProcessor::getCurrentProgram() { return 0; }
void PianoSpeedrunProcessor::setCurrentProgram (int) {}
const juce::String PianoSpeedrunProcessor::getProgramName (int) { return {}; }
void PianoSpeedrunProcessor::changeProgramName (int, const juce::String&) {}

void PianoSpeedrunProcessor::prepareToPlay (double, int) {}
void PianoSpeedrunProcessor::releaseResources() {}

void PianoSpeedrunProcessor::processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            trainer.handleNoteOn (msg.getNoteNumber());
            sheetTrainer.handleNoteOn (msg.getNoteNumber());
        }
    }
}

juce::AudioProcessorEditor* PianoSpeedrunProcessor::createEditor()
{
    return new PianoSpeedrunEditor (*this);
}

bool PianoSpeedrunProcessor::hasEditor() const { return true; }

void PianoSpeedrunProcessor::getStateInformation (juce::MemoryBlock&) {}
void PianoSpeedrunProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PianoSpeedrunProcessor();
}
