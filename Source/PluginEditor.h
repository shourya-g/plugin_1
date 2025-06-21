/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

/*
 "You can create a subclass of TabbedButtonBar which is also a DragAndDropTarget. And you can create custom tab buttons which allow themselves to be dragged."
 */
struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget, juce::DragAndDropContainer
{
  ExtendedTabbedButtonBar(): juce::TabbedButtonBar
  (juce::TabbedButtonBar::Orientation::TabsAtTop)
  {}

  bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override
  {
    return false;
  }

  void itemDropped(const SourceDetails& dragSourceDetails) override
  {}
  juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;

};
//we need some kind of horozontal constrrainer (todo)done

struct HorizontalConstrainer : juce::ComponentBoundsConstrainer
{
  //the thing that is doing the confinding and the things that is being confined 
    HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                          std::function<juce::Rectangle<int>()> confineeBoundsGetter);
    
    void checkBounds (juce::Rectangle<int>& bounds,
                      const juce::Rectangle<int>& previousBounds,
                      const juce::Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight) override;
    
private:
    std::function<juce::Rectangle<int>()> boundsToConfineToGetter;
    std::function<juce::Rectangle<int>()> boundsOfConfineeGetter;
};

struct ExtendedTabBarButton: juce::TabBarButton
{
      ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner);
    juce::ComponentDragger dragger;
    std::unique_ptr<HorizontalConstrainer> constrainer;
   
    
    void mouseDown (const juce::MouseEvent& e){
      dragger.startDraggingComponent(this, e);
    }

    void mouseDrag (const juce::MouseEvent& e){
      //can add paramterof juce::ractangle here to confine the drag
      dragger.dragComponent(this, e, constrainer.get());
    }


};



class Audio_proAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Audio_proAudioProcessorEditor (Audio_proAudioProcessor&);
    ~Audio_proAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Audio_proAudioProcessor& audioProcessor;
   juce::TextButton dspOrderButton{ "DSP Order" };
   ExtendedTabbedButtonBar tabbedComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Audio_proAudioProcessorEditor)
};
