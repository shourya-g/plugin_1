/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

static juce::String getNameFromDSPOption(Audio_proAudioProcessor::DSP_Option option)
{
    switch (option)
    {
        case Audio_proAudioProcessor::DSP_Option::Phase:
            return "PHASE";
        case Audio_proAudioProcessor::DSP_Option::Chorus:
            return "CHORUS";
        case Audio_proAudioProcessor::DSP_Option::Overdrive:
            return "OVERDRIVE";
        case Audio_proAudioProcessor::DSP_Option::LadderFilter:
            return "LADDERFILTER";
        case Audio_proAudioProcessor::DSP_Option::GeneralFilter:
            return "GEN FILTER";
        case Audio_proAudioProcessor::DSP_Option::END_OF_LIST:
            jassertfalse;
    }
    
    return "NO SELECTION";
}
HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter, 
                                             std::function<juce::Rectangle<int>()> confineeBoundsGetter) :
boundsToConfineToGetter(std::move(confinerBoundsGetter)),
boundsOfConfineeGetter(std::move(confineeBoundsGetter))
{
    
}
void HorizontalConstrainer::checkBounds (juce::Rectangle<int>& bounds,
        const juce::Rectangle<int>& previousBounds,
                       const juce::Rectangle<int>& limits,
                                bool isStretchingTop,
                                         bool isStretchingLeft,
                                    bool isStretchingBottom,
                                      bool isStretchingRight)
{
   // need to fix the y position first most imporant can tlet it go out of the tabbed button bar
    bounds.setY( previousBounds.getY() );
    //now I need to confine the x position
    //i need to know the bounds of the tabbed button bar and the button
    //i can get the bounds of the tabbed button bar
    //i can get the bounds of the button
    // labda getter function
    
    
    if( boundsToConfineToGetter != nullptr &&
       boundsOfConfineeGetter != nullptr )
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();
        //must subtract the width of the button from the right side of the tabbed button bar
        bounds.setX( juce::jlimit(boundsToConfineTo.getX(),
                                  boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
                                  bounds.getX()));
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(),
                                 limits.getY(),
                                 bounds.getX()));
    }
}

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner)
: juce::TabBarButton(name, owner)
{
     constrainer = std::make_unique<HorizontalConstrainer>([&owner](){ return owner.getLocalBounds(); },
                                                          [this](){ return getBounds(); });
    //this is to prevent the button from being dragged outside the tabbed button bar so giving huge nubmer
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
  return new ExtendedTabBarButton(tabName, *this);
}


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


         tabbedComponent.clearTabs();

     for(auto&val :dspOrder)
     {
      auto entry = r.nextInt(range);
      val= static_cast<Audio_proAudioProcessor::DSP_Option>(entry);
      tabbedComponent.addTab(getNameFromDSPOption(val), juce::Colours::white,-1);
     }
     DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
    //  jassertfalse;
     audioProcessor.dspOrderFifo.push(dspOrder);
     
      };
      

    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
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
    auto bounds = getLocalBounds();

    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150,30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));
}
