/*
  ==============================================================================

    Distortion.cpp
    Created: 12 Mar 2023 12:36:54pm
    Author:  Alperen Kurbetci

  ==============================================================================
*/

#include "Distortion.h"

template <typename SampleType>

Distortion<SampleType>::Distortion()
{
    
}

template <typename SampleType>

void Distortion<SampleType>::prepare(juce::dsp::ProcessSpec &spec)
{
   
    _sampleRate = spec.sampleRate;
    
    dcFilter.prepare(spec);
    dcFilter.setCutoffFrequency(10.0);
    dcFilter.setType(juce::dsp::LinkwitzRileyFilter<float>::Type::highpass);
    
    
    reset();
    
}

template <typename SampleType>

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


template class Distortion<float>;
template class Distortion<double>;
