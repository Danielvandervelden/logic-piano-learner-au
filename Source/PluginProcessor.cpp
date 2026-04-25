#include "PluginProcessor.h"
#include "PluginEditor.h"

DanielsPianoHelperProcessor::DanielsPianoHelperProcessor()
    : AudioProcessor (BusesProperties())
{
}

DanielsPianoHelperProcessor::~DanielsPianoHelperProcessor() {}

const juce::String DanielsPianoHelperProcessor::getName() const { return "Daniels Piano Helper"; }
bool DanielsPianoHelperProcessor::acceptsMidi() const { return true; }
bool DanielsPianoHelperProcessor::producesMidi() const { return true; }
bool DanielsPianoHelperProcessor::isMidiEffect() const { return true; }
double DanielsPianoHelperProcessor::getTailLengthSeconds() const { return 0.0; }
int DanielsPianoHelperProcessor::getNumPrograms() { return 1; }
int DanielsPianoHelperProcessor::getCurrentProgram() { return 0; }
void DanielsPianoHelperProcessor::setCurrentProgram (int) {}
const juce::String DanielsPianoHelperProcessor::getProgramName (int) { return {}; }
void DanielsPianoHelperProcessor::changeProgramName (int, const juce::String&) {}

void DanielsPianoHelperProcessor::prepareToPlay (double, int) {}
void DanielsPianoHelperProcessor::releaseResources() {}

void DanielsPianoHelperProcessor::processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer& midi)
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

juce::AudioProcessorEditor* DanielsPianoHelperProcessor::createEditor()
{
    return new DanielsPianoHelperEditor (*this);
}

bool DanielsPianoHelperProcessor::hasEditor() const { return true; }

void DanielsPianoHelperProcessor::getStateInformation (juce::MemoryBlock&) {}
void DanielsPianoHelperProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DanielsPianoHelperProcessor();
}
