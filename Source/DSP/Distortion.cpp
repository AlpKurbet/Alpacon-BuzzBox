/*
  ==============================================================================

    Distortion.cpp
    Created: 12 Mar 2023 12:36:54pm
    Author:  Alperen Kurbetci

  ==============================================================================
*/

#include "Distortion.h"

//JUCE example dsp folders have this line for the SampleType. It must be used just above every time typename is called
template <typename SampleType>

Distortion<SampleType>::Distortion()
{
    
}

template <typename SampleType>

//Setting up the specs
void Distortion<SampleType>::prepare(juce::dsp::ProcessSpec &spec)
{
   
    _sampleRate = spec.sampleRate;
    
    dcFilter.prepare(spec);
    dcFilter.setCutoffFrequency(10.0);
    dcFilter.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    
    
    reset();
    
}

template <typename SampleType>

//Reaching to the parameters from the ./Parameters/Globals.h
void Distortion<SampleType>::reset()
{
    if(_sampleRate <= 0) return;
    
    _input.reset(_sampleRate, 0.02);
    _input.setTargetValue(0.0);
    
    _output.reset(_sampleRate, 0.02);
    _output.setTargetValue(1.0);
    
    _mix.reset(_sampleRate, 0.02);
    _mix.setTargetValue(0.0);
}

template <typename SampleType>

//Getting the values from the user inputs
void Distortion<SampleType>::setDrive(SampleType newDrive)
{
    _input.setTargetValue(newDrive);
}

template <typename SampleType>
void Distortion<SampleType>::setMix(SampleType newMix)
{
    
    _mix.setTargetValue(newMix);
}

template <typename SampleType>
void Distortion<SampleType>::setOutput(SampleType nexOutput)
{
    _output.setTargetValue(nexOutput);
}

//Switching betwen choices
template <typename SampleType>
void Distortion<SampleType>::setDistortionModel(DistortionModel newModel)
{
    switch(newModel)
    {
        case DistortionModel::cHard:
    {
        _model = newModel;
        break;
    }
    
    
        case DistortionModel::cSoft:
    {
        _model = newModel;
        break;
    }
    
    
        case DistortionModel::cSaturation:
    {
        _model = newModel;
        break;
    }

    }
     
}

//Setting up the types of variables that the typename template can have
template class Distortion<float>;
template class Distortion<double>;
