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
  ExtendedTabbedButtonBar();

  bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

/*  first, itemDragEnter is called when the first mouseEvent happens.
     as the mouse is moved, itemDragMove() is called.  
     the tabBarButtons are constrained to the bounds of the ExtendedTabbedButtonBar, 
     so they'll never be dragged outside of it.
     itemDragMove() checks the x coordinate of the item being dragged and compares it with its neighbors.
     tab indexes are swapped if a tab crosses over the middle of another tab.*/
     //all this needs to be before creat tab button
 void itemDragEnter (const SourceDetails& dragSourceDetails) override;
  void itemDragMove (const SourceDetails& dragSourceDetails) override;
void itemDragExit (const SourceDetails& dragSourceDetails) override;
  void itemDropped(const SourceDetails& dragSourceDetails) override;
   void mouseDown(const juce::MouseEvent& e) override;
  juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;

  // Helper functions for drag and drop
  juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
  int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
  juce::Array<juce::TabBarButton*> getTabs();

private:
    juce::Point<int> previousDraggedTabCenterPosition; 
};
//need some kind of horozontal constrrainer (todo)done

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
   
    
    void mouseDown (const juce::MouseEvent& e) override;

    void mouseDrag (const juce::MouseEvent& e) override;


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
