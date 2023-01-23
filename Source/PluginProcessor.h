/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "C:\Users\pedro\Desktop\OSC\OSC\RnboExport\rnbo\RNBO.h"

//==============================================================================
/**
*/
class OSCAudioProcessor : public juce::AudioProcessor,
                           public juce::AudioProcessorValueTreeState::Listener
    
{
public:
    //==============================================================================
    OSCAudioProcessor();
    ~OSCAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:

    juce::AudioProcessorValueTreeState apvts;
    juce::UndoManager undoManager;

    RNBO::CoreObject rnboObject;

    std::unordered_map<juce::String, RNBO::ParameterIndex> apvtsParamIdToRnboParamIndex;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCAudioProcessor)
};
