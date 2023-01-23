/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParamIDs.h"


static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;


    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ ParamIDs::volume, 1 },
        ParamIDs::volume,
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.0f, 2.0f),
        1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (-10.0f < value && value < 10.0f)
            return juce::String(value, 1) + " dB";
            else
                return juce::String(std::roundf(value), 0) + " dB"; },
        nullptr));

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ ParamIDs::freq, 1 },
        ParamIDs::freq,
        juce::NormalisableRange<float>(200.0f, 1000.0f, 200.0f, 1000.0f),
        440.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value, 1) + " st"; },
            nullptr));





    return layout;
}

//==============================================================================
OSCAudioProcessor::OSCAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    apvts(*this, &undoManager, "Parameters", createParameterLayout())


{
    for (RNBO::ParameterIndex i = 0; i < rnboObject.getNumParameters(); ++i)
    {
        RNBO::ParameterInfo info;
        rnboObject.getParameterInfo(i, &info);

        if (info.visible)
        {
            auto paramID = juce::String(rnboObject.getParameterId(i));

            // Each apvts parameter id and range must be the same as the rnbo param object's.
            // If you hit this assertion then you need to fix the incorrect id in ParamIDs.h.
            jassert(apvts.getParameter(paramID) != nullptr);

            // If you hit these assertions then you need to fix the incorrect apvts 
            // parameter range in createParameterLayout().
            jassert(info.min == apvts.getParameterRange(paramID).start);
            jassert(info.max == apvts.getParameterRange(paramID).end);

            apvtsParamIdToRnboParamIndex[paramID] = i;

            apvts.addParameterListener(paramID, this);
            rnboObject.setParameterValue(i, apvts.getRawParameterValue(paramID)->load());
        }
    }
}

OSCAudioProcessor::~OSCAudioProcessor()
{
}

//==============================================================================
const juce::String OSCAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OSCAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OSCAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OSCAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OSCAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OSCAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OSCAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OSCAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String OSCAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void OSCAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);

}

//==============================================================================
void OSCAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    rnboObject.prepareToProcess(sampleRate, static_cast<size_t> (samplesPerBlock));
}

void OSCAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OSCAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OSCAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    auto bufferSize = buffer.getNumSamples();

    rnboObject.prepareToProcess(getSampleRate(),
        static_cast<size_t> (bufferSize));

    rnboObject.process(buffer.getArrayOfWritePointers(),
        static_cast<RNBO::Index> (buffer.getNumChannels()),
        buffer.getArrayOfWritePointers(),
        static_cast<RNBO::Index> (buffer.getNumChannels()),
        static_cast<RNBO::Index> (bufferSize));
}

//==============================================================================
bool OSCAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OSCAudioProcessor::createEditor()
{
    //return new JRGranularAudioProcessorEditor(*this, apvts, undoManager);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void OSCAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void OSCAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, static_cast<size_t> (sizeInBytes));

    if (tree.isValid())
        apvts.replaceState(tree);
}

void OSCAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    rnboObject.setParameterValue(apvtsParamIdToRnboParamIndex[parameterID], newValue);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OSCAudioProcessor();
}
