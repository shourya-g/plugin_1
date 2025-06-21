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
//we need some kind of horozontal constrrainer (todo)


struct ExtendedTabBarButton: juce::TabBarButton
{
      ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner);
    juce::ComponentDragger dragger;
   
    
    void mouseDown (const juce::MouseEvent& e){
      dragger.startDraggingComponent(this, e);
    }

    void mouseDrag (const juce::MouseEvent& e){
      dragger.dragComponent(this, e, nullptr);
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
