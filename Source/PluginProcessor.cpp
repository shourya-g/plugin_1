/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
auto getPhaserRateName() {return juce::String("Phaser RateHz");}
auto getPhaserDepthName() {return juce::String("Phaser Depth %");}
auto getPhaserCenterFreqName() {return juce::String("Phaser Center FreqHz");}
auto getPhaserFeedbackName() {return juce::String("Phaser Feedback %");}
auto getPhaserMixName() {return juce::String("Phaser Mix %");}
//==============================================================================
Audio_proAudioProcessor::Audio_proAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    auto phaserParams= std::array
    {
        &phaserRateHz,
        &phaserDepthPercent,
        &phaserCenterFreqHz,
        &phaserFeedbackPercent,
        &phaserMixPercent
    };

    auto phaserFuncs= std::array
    {
        &getPhaserRateName,
        &getPhaserDepthName,
        &getPhaserCenterFreqName,
        &getPhaserFeedbackName,
        &getPhaserMixName
    };
    for(size_t i = 0; i < phaserParams.size(); ++i)
    {   
        //get the pointer to the parameter
        auto ptrtoParam = phaserParams[i];
        //get the parameter from the apvts
        *ptrtoParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(phaserFuncs[i]()));
        jassert(*ptrtoParam != nullptr);
    }
}

Audio_proAudioProcessor::~Audio_proAudioProcessor()
{
}

//==============================================================================
const juce::String Audio_proAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Audio_proAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Audio_proAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Audio_proAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Audio_proAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Audio_proAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Audio_proAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Audio_proAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Audio_proAudioProcessor::getProgramName (int index)
{
    return {};
}

void Audio_proAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Audio_proAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Audio_proAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Audio_proAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

juce::AudioProcessorValueTreeState::ParameterLayout 
Audio_proAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const int versionhint = 1;
    //PHASERRATEHZ
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 2.f, 0.01f,1.f), 0.2f,"Hz"));
    
    //PHASERDEPTH 0 TO 1
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f,1.f), 0.05f,"%"));
    
    //PHASERCENTERFREQ 0 TO 1
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(20.f, 20000.f, 1.f,1.f), 1000.f,"Hz"));

    //PHASERFEEDBACK -1 TO 1
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(-1.f, 1.f, 0.01f,1.f), 0.0f,"%"));

    //PHASERMIX 0 TO 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f,1.f), 0.05f,"%"));
     

    return layout;
}

void Audio_proAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //TODO: add apvts[done]
    //TODO: create audio parameters for all the dsp options
    

    auto newDSPOrder= DSP_Order();
    //pulling the order trying actually
    while (dspOrderFifo.pull(newDSPOrder))
    {

    }
    //if pulled,replace
    if(DSP_Order() != newDSPOrder)
    {
        dspOrder = newDSPOrder;

    }
    //array of pointers
    Dsp_pointers dspPointers;
    for(size_t i = 0; i < dspPointers.size(); ++i)
    {
        switch(dspOrder[i])
        {
            case DSP_Option::Phase:
                dspPointers[i] = &phaser;
                break;
            case DSP_Option::Chorus:
                dspPointers[i] = &chorus;
                break;
            case DSP_Option::Overdrive:
                dspPointers[i] = &overdrive;
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i] = &ladderFilter;
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
            default:
                dspPointers[i] = nullptr;
        }
    }

    //now procceess

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for(size_t i = 0; i < dspPointers.size(); ++i)
    {   //need to check if the pointer is not null
        if(dspPointers[i] != nullptr)
        {
            dspPointers[i]->process(context);
        }
    }
   
}

//==============================================================================
bool Audio_proAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Audio_proAudioProcessor::createEditor()
{
    return new Audio_proAudioProcessorEditor (*this);
}

//==============================================================================
void Audio_proAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Audio_proAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Audio_proAudioProcessor();
}
