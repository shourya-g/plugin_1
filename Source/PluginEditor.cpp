/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Audio_proAudioProcessorEditor::Audio_proAudioProcessorEditor (Audio_proAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    dspOrderButton.onClick= [this]()
      {
        juce::Random r;
        Audio_proAudioProcessor::DSP_Order dspOrder;
        auto range= juce::Range<int>
        (static_cast<int>(Audio_proAudioProcessor::DSP_Option::Phase),
         static_cast<int>(Audio_proAudioProcessor::DSP_Option::END_OF_LIST));
     for(auto&val :dspOrder)
     {
      auto entry = r.nextInt(range);
      val= static_cast<Audio_proAudioProcessor::DSP_Option>(entry);
     }
     DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
     jassertfalse;
     audioProcessor.dspOrderFifo.push(dspOrder);
     
      };
      

    addAndMakeVisible(dspOrderButton);
    setSize (400, 300);
}

Audio_proAudioProcessorEditor::~Audio_proAudioProcessorEditor()
{
}

//==============================================================================
void Audio_proAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Audio_proAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    dspOrderButton.setBounds(getLocalBounds().reduced(100));
}
