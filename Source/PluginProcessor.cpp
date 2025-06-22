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
auto getPhaserBypassName() { return juce::String("Phaser Bypass"); }


auto getChorusRateName() {return juce::String("Chorus RateHz");}
auto getChorusDepthName() {return juce::String("Chorus Depth %");}
auto getChorusCenterDelayName() {return juce::String("Chorus Center DelayMs");}
auto getChorusFeedbackName() {return juce::String("Chorus Feedback %");}
auto getChorusMixName() {return juce::String("Chorus Mix %");}
auto getChorusBypassName() { return juce::String("Chorus Bypass"); }
auto getOverdriveSaturationName() {return juce::String("Overdrive Saturation %");}
auto getOverdriveBypassName() { return juce::String("Overdrive Bypass"); }


auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffName() { return juce::String("Ladder Filter Cutoff Hz"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }
auto getLadderFilterBypassName() { return juce::String("Ladder Filter Bypass"); }
auto getLadderFilterChoices()
{
    return juce::StringArray
    {
        "LPF12",  // low-pass  12 dB/octave
        "HPF12",  // high-pass 12 dB/octave
        "BPF12",  // band-pass 12 dB/octave
        "LPF24",  // low-pass  24 dB/octave
        "HPF24",  // high-pass 24 dB/octave
        "BPF24"   // band-pass 24 dB/octave
    };
}

auto getGeneralFilterChoices()
{
    return juce::StringArray
    {
         "Peak",
        "bandpass",
        "notch",
        "allpass",
    };
}
auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }
auto getGeneralFilterBypassName() { return juce::String("General Filter Bypass"); }

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

    //before it was dspOrder=
    //{{
    //    DSP_Option::Phase,
    //    DSP_Option::Chorus,
    //    DSP_Option::Overdrive,
    //    DSP_Option::LadderFilter,
    //    //enum automatically takes phaser if size difference 
    for( size_t i = 0; i < static_cast<size_t>(DSP_Option::END_OF_LIST); ++i )
    {
        dspOrder[i] = static_cast<DSP_Option>(i);
    }



    //array of pointers to the parameters
    auto floatParams= std::array
    {
        &phaserRateHz,
        &phaserDepthPercent,
        &phaserCenterFreqHz,
        &phaserFeedbackPercent,
        &phaserMixPercent,

        &chorusRateHz,
        &chorusDepthPercent,
        &chorusCenterDelayMs,
        &chorusFeedbackPercent,
        &chorusMixPercent,

        &overdriveSaturationPercent,

        &ladderFilterCutoffHz,
        &ladderFilterResonance,
        &ladderFilterDrive,
        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGaindB
        
    };

    auto floatnameFuncs= std::array
    {
        &getPhaserRateName,
        &getPhaserDepthName,
        &getPhaserCenterFreqName,
        &getPhaserFeedbackName,
        &getPhaserMixName,
        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,

        &getOverdriveSaturationName,

        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,
        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName
    };
    //connects each parameter to its name function for the apvts for float params
    initCachedParams<juce::AudioParameterFloat*>(floatParams, floatnameFuncs);

    //now for choice params 
       auto choiceParams = std::array
    {
        &ladderFilterMode,
        &generalFilterMode,
    };
    
    auto choiceNameFuncs = std::array
    {
        &getLadderFilterModeName,
        &getGeneralFilterModeName,
    };
    
    initCachedParams<juce::AudioParameterChoice*>(choiceParams, choiceNameFuncs);


        auto bypassParams = std::array
    {
        &phaserBypass,
        &chorusBypass,
        &overdriveBypass,
        &ladderFilterBypass,
        &generalFilterBypass,
    };
    
    auto bypassNameFuncs = std::array
    {
        &getPhaserBypassName,
        &getChorusBypassName,
        &getOverdriveBypassName,
        &getLadderFilterBypassName,
        &getGeneralFilterBypassName,
    };

    initCachedParams<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);
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
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
        for( auto smoother : getSmoothers() )
    {
        smoother->reset(sampleRate, 0.005);
    }
    
    updateSmoothersFromParams(1, SmootherUpdateMode::initialize);
  
    
}
void Audio_proAudioProcessor::updateSmoothersFromParams(int numSamplesToSkip, SmootherUpdateMode init)
{   
    //refernce?? doubt
    auto paramsNeedingSmoothing = std::vector
    {
        phaserRateHz,
        phaserDepthPercent,
        phaserCenterFreqHz, 
        phaserFeedbackPercent,
        phaserMixPercent,
        chorusRateHz,
        chorusDepthPercent,
        chorusCenterDelayMs,
        chorusFeedbackPercent,
        chorusMixPercent,
        overdriveSaturationPercent,
        ladderFilterCutoffHz,
        ladderFilterResonance,
        ladderFilterDrive,
        generalFilterFreqHz,
        generalFilterQuality,
        generalFilterGaindB,
  
    };
    
    auto smoothers = getSmoothers();
    jassert( smoothers.size() == paramsNeedingSmoothing.size() );
    
    for( size_t i = 0; i < smoothers.size(); ++i )
    {
        auto smoother = smoothers[i];
        auto param = paramsNeedingSmoothing[i];
        
        if( init == SmootherUpdateMode::initialize )
            smoother->setCurrentAndTargetValue( param->get() );
        else
            smoother->setTargetValue(param->get());
        
        smoother->skip(numSamplesToSkip);
    }
    
    
}



std::vector<juce::SmoothedValue<float>*> Audio_proAudioProcessor::getSmoothers()
{
    auto smoothers = std::vector
    {
        //alwasy change do depht is befroe centerfreq in phaser also
        &phaserRateHzSmoother,
         &phaserDepthPercentSmoother,
        &phaserCenterFreqHzSmoother,
        &phaserFeedbackPercentSmoother,
        &phaserMixPercentSmoother,
        &chorusRateHzSmoother,
        &chorusDepthPercentSmoother,
        &chorusCenterDelayMsSmoother,
        &chorusFeedbackPercentSmoother,
        &chorusMixPercentSmoother,
        &overdriveSaturationSmoother,
        &ladderFilterCutoffHzSmoother,
        &ladderFilterResonanceSmoother,
        &ladderFilterDriveSmoother,
        &generalFilterFreqHzSmoother,
        &generalFilterQualitySmoother,
        &generalFilterGainSmoother,
  
    };
    
    return smoothers;
}

void Audio_proAudioProcessor::MonoChannelDSP::prepare(const juce::dsp::ProcessSpec &spec)
{
    jassert(spec.numChannels == 1);
    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter
    };
    
    for( auto p : dsp )
    {
        p->prepare(spec);
        p->reset();
    }
    
    overdrive.dsp.setCutoffFrequencyHz(20000.f);
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

    //PHASERBYPASS
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionhint}, name, false));

    //CHORUSRATEHZ
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 100.f, 0.01f,1.f), 0.2f,"Hz"));

    //CHORUSDEPTH 0 TO 1
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f,1.f), 0.05f,"%"));
    
    //CHORUSCENTERDELAYMS
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 100.f, 0.01f,1.f), 7.5f,"ms"));
    
    //CHORUSFEEDBACK -1 TO 1
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(-1.f, 1.f, 0.01f,1.f), 0.0f,"%"));

    //CHORUSMIX 0 TO 1
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f,1.f), 0.05f,"%"));

    //CHORUSBYPASS
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionhint}, name, false));

    //OVERDRIVESATURATION 0 TO 1
    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name, versionhint},
    name, juce::NormalisableRange<float>(1.f, 100.f, 0.1f,1.f), 1.f,"%"));

    //OVERDRIVEBYPASS
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionhint}, name, false));

    /*
    //LADDERFILTERMODE
    mode: ladder filter mode
    cutoff: ladder filter cutoff hz
    resonance: ladder filter resonance
    drive: ladder filter drive
    mode: ladder filter mode
    cutoff: ladder filter cutoff hz
    resonance: ladder filter resonance
    drive: ladder filter drive
    
    */

   name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{name, versionhint}, name, choices, 0));
    
    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       juce::ParameterID{name, versionhint},
       name,
       juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f),
       20000.f,
       "Hz"));
    
    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       juce::ParameterID{name, versionhint},
       name,
       juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f),
       0.f,
       "%"));
    
    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
       juce::ParameterID{name, versionhint},
       name,
       juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
       1.f,
       ""));
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionhint}, name, false));

    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{name, versionhint}, name, choices, 0));

    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{name, versionhint},
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        750.f,
        "Hz"));
//GENERALFILTERQUALITY 0.1 TO 10
    name = getGeneralFilterQualityName();
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{name, versionhint},
        name,
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f,
        ""));
    //GENERALFILTERGAIN -24dB TO 24dB
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{name, versionhint},
        name,
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f,
        "dB"));
          
       name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{name, versionhint}, name, false));

    return layout;
}

void Audio_proAudioProcessor::MonoChannelDSP::updateDSPFromParams()
{
       //might have to see the layout to confirm that the rnages of all paramters are correct
    phaser.dsp.setRate(p.phaserRateHzSmoother.getCurrentValue());
    phaser.dsp.setDepth(p.phaserDepthPercentSmoother.getCurrentValue());
    phaser.dsp.setCentreFrequency(p.phaserCenterFreqHzSmoother.getCurrentValue());
    phaser.dsp.setFeedback(p.phaserFeedbackPercentSmoother.getCurrentValue());
    phaser.dsp.setMix(p.phaserMixPercentSmoother.getCurrentValue());

    chorus.dsp.setRate(p.chorusRateHzSmoother.getCurrentValue());
    chorus.dsp.setDepth(p.chorusDepthPercentSmoother.getCurrentValue());
    chorus.dsp.setCentreDelay(p.chorusCenterDelayMsSmoother.getCurrentValue());
    chorus.dsp.setFeedback(p.chorusFeedbackPercentSmoother.getCurrentValue());
    chorus.dsp.setMix(p.chorusMixPercentSmoother.getCurrentValue());

    overdrive.dsp.setDrive(p.overdriveSaturationSmoother.getCurrentValue());
    
    ladderFilter.dsp.setMode(
    static_cast<juce::dsp::LadderFilterMode>(p.ladderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(p.ladderFilterCutoffHzSmoother.getCurrentValue());
    ladderFilter.dsp.setResonance(p.ladderFilterResonanceSmoother.getCurrentValue());
    ladderFilter.dsp.setDrive(p.ladderFilterDriveSmoother.getCurrentValue());

    // need to update genral coeeffs here
    auto sampleRate = p.getSampleRate();
    //update generalFilter coefficients
    //choices: peak, bandpass, notch, allpass
    auto genMode = p.generalFilterMode->getIndex();
    auto genHz = p.generalFilterFreqHzSmoother.getCurrentValue();
    auto genQ = p.generalFilterQualitySmoother.getCurrentValue();
    auto genGain = p.generalFilterGainSmoother.getCurrentValue();
    
    bool filterChanged = false;
    filterChanged |= (filterFreq != genHz);
    filterChanged |= (filterQ != genQ);
    filterChanged |= (filterGain != genGain);
    
    auto updatedMode = static_cast<GeneralFilterMode>(genMode);
    filterChanged |= (filterMode != updatedMode);
    
    if( filterChanged )
    {
        filterMode = updatedMode;
        filterFreq = genHz;
        filterQ = genQ;
        filterGain = genGain;
        
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        switch(filterMode)
        {
            case GeneralFilterMode::Peak:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                   filterFreq,
                                                                                   filterQ,
                                                                                   juce::Decibels::decibelsToGain(filterGain));
                break;
            }
            case GeneralFilterMode::Bandpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 
                                                                                 filterFreq,
                                                                                 filterQ);
                break;
            }
            case GeneralFilterMode::Notch:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate,
                                                                              filterFreq,
                                                                              filterQ);
                break;
            }
            case GeneralFilterMode::Allpass:
            {
                coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate,
                                                                                filterFreq,
                                                                                filterQ);
                break;
            }
            case GeneralFilterMode::END_OF_LIST:
            { //very imp to have this case shoudk never be hit 
                jassertfalse;
                break;
            }
        }
        
        if( coefficients != nullptr )
        {

            //DONT WORRY IF THIS JAASSERT HITS JUST COMMENT STHIS OUT LOL 
           //if( generalFilter.dsp.coefficients->coefficients.size() != coefficients->coefficients.size() )
           //{
           //    jassertfalse;    
           //}
            //this is the actual update of the coefficients
            *generalFilter.dsp.coefficients = *coefficients;
            generalFilter.reset();
        }
    }    
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
    


    
    

    leftChannel.updateDSPFromParams();
    rightChannel.updateDSPFromParams();

    auto newDSPOrder= DSP_Order();
    //pulling the order trying actually
    while (dspOrderFifo.pull(newDSPOrder))
    {
        #if VERIFY_BYPASS_FUNCTIONALITY
        jassertfalse;
        #endif
    }
    //if pulled,replace
    if(newDSPOrder != DSP_Order())
    {
        dspOrder = newDSPOrder;
        

    }


    const auto numSamples = buffer.getNumSamples(); // (1)
    auto samplesRemaining = numSamples;
    auto maxSamplesToProcess = juce::jmin(samplesRemaining, 64); // (2)

    auto block = juce::dsp::AudioBlock<float>(buffer);
      size_t startSample = 0; 
          while( samplesRemaining > 0 ) // (3)
    {
        /*
         The first time through this loop samplesToProcess will be 64, because maxSmplesToProcess is set to 64, and samplesRamaining is 72.
         the 2nd time this loop runs, samplesToProcess will be 8, because the previous loop consumed 64 of the 72 samples.
         */

        //figure out how many samples to actually process.
        auto samplesToProcess = juce::jmin(samplesRemaining, maxSamplesToProcess); // (4)
        //advance each smoother 'samplesToProcess' samples
        updateSmoothersFromParams(samplesToProcess, SmootherUpdateMode::liveInRealtime); // (5)
        
        //update the DSP
        leftChannel.updateDSPFromParams();  // (6)
        rightChannel.updateDSPFromParams();
        
        //create a sub block from the buffer, and
        auto subBlock = block.getSubBlock(startSample, samplesToProcess); // (7)
        
        //now process
        leftChannel.process(subBlock.getSingleChannelBlock(0), dspOrder); // (8)
        rightChannel.process(subBlock.getSingleChannelBlock(1), dspOrder);
        
        startSample += samplesToProcess; // (9)
        samplesRemaining -= samplesToProcess;
    }



   
   
}


void Audio_proAudioProcessor::MonoChannelDSP::process(juce::dsp::AudioBlock<float> block, const DSP_Order &dspOrder)
{
    Dsp_pointers dspPointers;
    dspPointers.fill({}); //this was previously dspPointers.fill(nullptr);
    
    for( size_t i = 0; i < dspPointers.size(); ++i )
    {
        switch (dspOrder[i])
        {
            case DSP_Option::Phase:
                dspPointers[i].processor = &phaser;
                dspPointers[i].bypass = p.phaserBypass->get();
                break;
            case DSP_Option::Chorus:
                dspPointers[i].processor = &chorus;
                dspPointers[i].bypass = p.chorusBypass->get();
                break;
            case DSP_Option::Overdrive:
                dspPointers[i].processor = &overdrive;
                dspPointers[i].bypass = p.overdriveBypass->get();
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i].processor = &ladderFilter;
                dspPointers[i].bypass = p.ladderFilterBypass->get();
                break;
            case DSP_Option::GeneralFilter:
                dspPointers[i].processor = &generalFilter;
                dspPointers[i].bypass = p.generalFilterBypass->get();
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
        }
    }
    
    //now process:
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    for( size_t i = 0; i < dspPointers.size(); ++i )
    {
        if( dspPointers[i].processor != nullptr )
        {
            juce::ScopedValueSetter<bool> svs(context.isBypassed,
                                              dspPointers[i].bypass);
#if VERIFY_BYPASS_FUNCTIONALITY
            if( context.isBypassed )
            {
                jassertfalse;
            }
            
            if( dspPointers[i].processor == &generalFilter )
            {
                continue;
            }
#endif
            dspPointers[i].processor->process(context);
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
    // return new juce::GenericAudioProcessorEditor(*this);

}
template<>
struct juce::VariantConverter<Audio_proAudioProcessor::DSP_Order>
{
    static Audio_proAudioProcessor::DSP_Order fromVar(const juce::var& v)
    {
     //reading from memeory bloack and casting that int to dsp order enumeration value
     using T= Audio_proAudioProcessor::DSP_Order;
     T dspOrder;
     jassert(v.isBinaryData());
     if(v.isBinaryData()==false)
     {
        dspOrder.fill(Audio_proAudioProcessor::DSP_Option::END_OF_LIST);
        //want to hit jasssert later
        return dspOrder;
     }
     else{
        auto mb= *v.getBinaryData();
        juce::MemoryInputStream mis(mb, false);
        
        std::vector<int> vec;
        while(!mis.isExhausted()){
            vec.push_back(mis.readInt());
        }
        jassert(vec.size()==dspOrder.size());
        for(size_t i = 0; i < vec.size(); ++i)
        {
            dspOrder[i]= static_cast<Audio_proAudioProcessor::DSP_Option>(vec[i]);
        }
        return dspOrder;
     }
    }
    static juce::var toVar(const Audio_proAudioProcessor::DSP_Order& order)
    {
        juce::MemoryBlock mb;
        //juce mos uses scoping properly to write to the memory block
        {
            juce::MemoryOutputStream mos(mb, false);
            for(const auto&val :order)
            {
                mos.writeInt(static_cast<int>(val));
            }
        }
        return mb;
    }
};
//==============================================================================
void Audio_proAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    //this is the same as the getStateInformation in the editor
    apvts.state.setProperty("DSP_Order",
     juce::VariantConverter<Audio_proAudioProcessor::DSP_Order>::toVar(dspOrder), nullptr);

    juce::MemoryOutputStream mos(destData, false);
    //write the state of the apvts to the memory block that  init before meaning its peerpspectie of daw
    apvts.state.writeToStream(mos);
}

void Audio_proAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    //read the state of the apvts from the memory block that  init before meaning its peerpspectie of daw
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid())
    {
        apvts.replaceState(tree);
        if(apvts.state.hasProperty("DSP_Order"))
        {
            auto order= juce::VariantConverter<Audio_proAudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("DSP_Order"));
            dspOrderFifo.push(order);
            //for gui 
            restoredDspOrderFifo.push(order);
        }
        //debugging the apvts state
        DBG( apvts.state.toXmlString() );


        //debugging the bypass functionality of the plugin
#if VERIFY_BYPASS_FUNCTIONALITY
        juce::Timer::callAfterDelay(1000, [this]()
        {
            DSP_Order order;
            order.fill(DSP_Option::LadderFilter);
            order[0] = DSP_Option::Chorus;
            
            chorusBypass->setValueNotifyingHost(1.f);
            dspOrderFifo.push(order);
        });
#endif
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Audio_proAudioProcessor();
}
