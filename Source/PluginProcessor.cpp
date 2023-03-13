/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BuzzBoxAudioProcessor::BuzzBoxAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, _treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    _treeState.addParameterListener(disModelID, this);
    _treeState.addParameterListener(inputID, this);
    _treeState.addParameterListener(outputID, this);
    _treeState.addParameterListener(mixID, this);
}

BuzzBoxAudioProcessor::~BuzzBoxAudioProcessor()
{
    _treeState.removeParameterListener(disModelID, this);
    _treeState.removeParameterListener(inputID, this);
    _treeState.removeParameterListener(outputID, this);
    _treeState.removeParameterListener(mixID, this);
}

juce::AudioProcessorValueTreeState::ParameterLayout BuzzBoxAudioProcessor::createParameterLayout()

{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::StringArray disMods = {"Hard", "Soft", "Saturation"};
    
    auto DriveModel = std::make_unique<juce::AudioParameterChoice>(disModelID,disModelName,disMods,0);
    auto paramDrive = std::make_unique<juce::AudioParameterFloat>(inputID, inputName, 0.0f, 24.0f, 0.0f);
    auto paramOutput = std::make_unique<juce::AudioParameterFloat>(outputID, outputName, -24.0f, 24.0f, 0.0f);
    auto paramMix= std::make_unique<juce::AudioParameterFloat>(mixID, mixName, 0.0f, 1.0f, 1.0f);
    
    params.push_back(std::move(DriveModel));
    params.push_back(std::move(paramDrive));
    params.push_back(std::move(paramOutput));
    params.push_back(std::move(paramMix));
    
    return {params.begin(), params.end()};
}

void BuzzBoxAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)

{
    updateParameters();
    
}

void BuzzBoxAudioProcessor::updateParameters()
{
    auto model = static_cast<int>(_treeState.getRawParameterValue(disModelID)->load());
    switch (model)
    {
        case 0: _myDistortion.setDistortionModel(Distortion<float>::DistortionModel::cHard); break;
        case 1: _myDistortion.setDistortionModel(Distortion<float>::DistortionModel::cSoft); break;
        case 2: _myDistortion.setDistortionModel(Distortion<float>::DistortionModel::cSaturation); break;
            
    }
        
        
    _myDistortion.setDrive(_treeState.getRawParameterValue(inputID) -> load());
    _myDistortion.setMix(_treeState.getRawParameterValue(mixID) -> load());
    _myDistortion.setOutput(_treeState.getRawParameterValue(outputID) -> load());
    
}

//==============================================================================
const juce::String BuzzBoxAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BuzzBoxAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BuzzBoxAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BuzzBoxAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BuzzBoxAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BuzzBoxAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BuzzBoxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BuzzBoxAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BuzzBoxAudioProcessor::getProgramName (int index)
{
    return {};
}

void BuzzBoxAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BuzzBoxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    _myDistortion.prepare(spec);
    
    updateParameters();
}

void BuzzBoxAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BuzzBoxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BuzzBoxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    const int numSamples = buffer.getNumSamples();
    MaxVal = buffer.getMagnitude(0, numSamples);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block {buffer};
    _myDistortion.process(juce::dsp::ProcessContextReplacing<float>(block));

}

//==============================================================================
bool BuzzBoxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BuzzBoxAudioProcessor::createEditor()
{
        //    return new BuzzBoxAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
    
}

//==============================================================================
void BuzzBoxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BuzzBoxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BuzzBoxAudioProcessor();
}
