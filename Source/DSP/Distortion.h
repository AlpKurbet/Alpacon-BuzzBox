/*
  ==============================================================================

    Distortion.h
    Created: 12 Mar 2023 12:36:54pm
    Author:  Alperen Kurbetci

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

template <typename SampleType>

class Distortion

{
public:
    
    Distortion();
    
    void prepare(juce::dsp::ProcessSpec& spec);
    
    void reset();
    
    /*
     The process block is for to handle different distortion processes like har/soft clipping
     */
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        //Coppied from juce_Compressor.h
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                
            {
                outputSamples[i] = dcFilter.processSample(channel,outputSamples[i] );
                outputSamples[i] = processSample(inputSamples[i], channel);
            }
                
        }
        
    }
    
    /*
    
     */
    SampleType processSample(SampleType inputSample, int channel) noexcept
    {
        switch(_model)
        {
            case DistortionModel::cHard:
        {
            return processHardClip (inputSample);
            break;
        }
        
        
            case DistortionModel::cSoft:
        {
            return processSoftClip (inputSample);
            break;
        }
        
        
            case DistortionModel::cSaturation:
        {
            return processSaturation(inputSample, channel);
            break;
        }

        }
    }
    
    SampleType processHardClip (SampleType inputSample)
    {
        // To drive the signal we multiply
        // getNextValue() funtion is due to _input being a smoothed value
        
        auto wetSignal = inputSample * juce::Decibels::decibelsToGain(_input.getNextValue());
        
        if (std::abs(wetSignal) > 0.99)
        
        {
            wetSignal *= 0.99 / std::abs(wetSignal);
        }
        
        auto mix = ((1.0 - _mix.getNextValue()) * inputSample) + wetSignal * _mix.getNextValue();
        
        //Here, we can use the wetSignal instead of the mix, programmer's choice

        return mix * juce::Decibels::decibelsToGain(_output.getNextValue());
        
    }
    
    SampleType processSoftClip (SampleType inputSample)
    {
        auto wetSignal = inputSample * juce::Decibels::decibelsToGain(_input.getNextValue());
        
        //tan, atan, tanh, attanh
        wetSignal = piDi * std::atan(wetSignal);
        
        wetSignal *= 2.0;
        
        wetSignal *= juce::Decibels::decibelsToGain(_input.getNextValue() * -0.25);
        
        if (std::abs(wetSignal) > 0.99)
        
        {
            wetSignal *= 0.99 / std::abs(wetSignal);
        }
        
        auto mix = (1.0 - _mix.getNextValue()) * inputSample + wetSignal * _mix.getNextValue();
        
        //Here, we can use the wetSignal instead of the mix, programmer's choice

        return mix * juce::Decibels::decibelsToGain(_output.getNextValue());
    }
    
    SampleType processSaturation(SampleType inputSample , int channel)
    {
        auto drive = juce::jmap(_input.getNextValue(), 0.0f, 24.0f, 0.0f, 6.0f);
        
        auto wetSignal = inputSample * juce::Decibels::decibelsToGain(drive);
        
        if(wetSignal >= 0.0)
        {
            wetSignal = std::tanh(wetSignal);
        }
        else
        {
            wetSignal = std::tanh(std::sinh(wetSignal)) - 0.2 * wetSignal * std::sin(juce::MathConstants<float>::pi * wetSignal);
        }
        
        wetSignal *= 1.15;
        wetSignal *= juce::Decibels::decibelsToGain(_input.getNextValue() * -0.05);
        
        auto mix = (1.0 - _mix.getNextValue()) * inputSample + wetSignal * _mix.getNextValue();

        return mix * juce::Decibels::decibelsToGain(_output.getNextValue());

    }
    
    enum class DistortionModel
    {
        cHard,
        cSoft,
        cSaturation
    };
    
    
    
    void setDrive(SampleType newDrive);
    void setMix(SampleType newMix);
    void setOutput(SampleType newOutput);
    
    void setDistortionModel(DistortionModel newModel);
    
    
    
private:
    juce::SmoothedValue<float> _input;
    juce::SmoothedValue<float> _mix;
    juce::SmoothedValue<float> _output;
    
    
    juce::dsp::LinkwitzRileyFilter<float> dcFilter;
    
    float piDi =  2.0 / juce::MathConstants<float>::pi;
    
    float _sampleRate = 48000.0f;
    DistortionModel _model = DistortionModel::cHard;
};
