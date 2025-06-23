/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "RotarySliderWithLabels.h"
#include <Utilities.h>
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
static Audio_proAudioProcessor::DSP_Option getDSPOptionFromName( juce::String name)
{
    if( name == "PHASE" )
        return Audio_proAudioProcessor::DSP_Option::Phase;
    if( name == "CHORUS" )
        return Audio_proAudioProcessor::DSP_Option::Chorus;
    if( name == "OVERDRIVE" )
        return Audio_proAudioProcessor::DSP_Option::Overdrive;
    if( name == "LADDERFILTER" )
        return Audio_proAudioProcessor::DSP_Option::LadderFilter;
    if( name == "GEN FILTER" )
        return Audio_proAudioProcessor::DSP_Option::GeneralFilter;
    
    return Audio_proAudioProcessor::DSP_Option::END_OF_LIST;
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

ExtendedTabBarButton::ExtendedTabBarButton
(const juce::String& name, juce::TabbedButtonBar& owner, 
Audio_proAudioProcessor::DSP_Option dspOption)
: juce::TabBarButton(name, owner), option(dspOption)
{
     constrainer = std::make_unique<HorizontalConstrainer>([&owner]()
     { return owner.getLocalBounds(); },
    [this](){ return getBounds(); });
    //this is to prevent the button from being dragged outside the tabbed button bar so giving huge nubmer
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}
void ExtendedTabBarButton::mouseDown (const juce::MouseEvent& e)
{
    toFront(true);
    dragger.startDraggingComponent (this, e);
    juce::TabBarButton::mouseDown(e);
}

void ExtendedTabBarButton::mouseDrag (const juce::MouseEvent& e)
{
    dragger.dragComponent (this, e, constrainer.get());
}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
        auto bestWidth = getLookAndFeel().getTabButtonBestWidth(*this, depth);
    
    auto& bar = getTabbedButtonBar();
   
    // choose whichever value is bigger, the bestWidth, or an equal division of the bar's width based on the number of tabs in the bar.
     
    return juce::jmax(bestWidth,
                      bar.getWidth() / bar.getNumTabs());
}

////////

ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) 
{
  
}
bool ExtendedTabbedButtonBar::isInterestedInDragSource (const SourceDetails& dragSourceDetails) 
{
  //if the ccompkent begin dragged in a button then only return true
        if( dynamic_cast<ExtendedTabBarButton*>( dragSourceDetails.sourceComponent.get() ) )
        return true;
    
    return false;
}

void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails &dragSourceDetails)
{
  DBG("ExtendedTabbedButtonBar::itemDragEnter");
  juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails &dragSourceDetails)
{
    auto tabButtonBeingDragged = findDraggedItem(dragSourceDetails);
    if (tabButtonBeingDragged == nullptr)
        return;
        
    auto idx = findDraggedItemIndex(dragSourceDetails);
    if (idx == -1)
    {
        DBG("failed to find tab being dragged in list of tabs");
        jassertfalse;
        return;
    }
    
    /* the mouseDown function takes a snapshot of the tab order, updating the tabs array(whis is proivate but).
     itemDragMove re-orders this array and reposition the tabs manually.
     the itemDropped() function is where the calls to moveTab() actually occur tho so made sure that we move it before dropping alawys
     
     if the center of the dragged tab transitions from < nextTab.X to >= nextTab.x (from left to right)
        swap(idx, nextTabIndex)
        This handles dragging the tab from left to right.
     if the center of the dragged tab transitions from > previousTab.Right to <= previousTab.right (from right to left)
        swap(idx, previousTabIndex)
        this handles dragging the tab from right to left.*/
    
    auto previousTabIndex = idx - 1;
    auto nextTabIndex = idx + 1;
    auto previousTab = juce::isPositiveAndBelow(previousTabIndex, getNumTabs()) ?
                        getTabButton(previousTabIndex) :
                        nullptr;
    auto nextTab = juce::isPositiveAndBelow(nextTabIndex, getNumTabs()) ? getTabButton(nextTabIndex) : nullptr;
    
    auto centerX = tabButtonBeingDragged->getBounds().getCentreX();
    
    if( centerX > previousDraggedTabCenterPosition.x )
    {
        //transitioning right.
        if( nextTab != nullptr )
        {
            if( previousDraggedTabCenterPosition.x < nextTab->getX() && nextTab->getX() <= centerX )
            {
                DBG( "swapping [" << idx << "] " << tabButtonBeingDragged->getName() << " with [" << nextTabIndex << "] " << nextTab->getName() );
                nextTab->setBounds(nextTab->getBounds().withX(previousTab != nullptr ?
                                                              previousTab->getRight() + 1 :
                                                              0));
                moveTab(idx, nextTabIndex);
            }
        }
    }
    else if( centerX < previousDraggedTabCenterPosition.x )
    {
        //transitioning left
        if( previousTab != nullptr )
        {
            if( previousDraggedTabCenterPosition.x > previousTab->getRight() && centerX <= previousTab->getRight() )
            {
                DBG( "swapping [" << idx << "] " << tabButtonBeingDragged->getName() << " with [" << previousTabIndex << "] " << previousTab->getName() );
                
                previousTab->setBounds(previousTab->getBounds().withX(nextTab != nullptr ?
                                                                      nextTab->getX() - previousTab->getWidth() - 1 :
                                                                      getWidth() - previousTab->getWidth() - 1));
                moveTab(idx, previousTabIndex);
            }
        }
    }

    tabButtonBeingDragged->toFront(true);
    
    previousDraggedTabCenterPosition = tabButtonBeingDragged->getBounds().getCentre();
}
void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails &dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}
void ExtendedTabbedButtonBar::itemDropped (const SourceDetails& dragSourceDetails) 
{
  DBG("ExtendedTabbedButtonBar::itemDropped");
  //dropps correctly in place and bar resized
    resized();
    auto tabs = getTabs();
    Audio_proAudioProcessor::DSP_Order newOrder;
    jassert(newOrder.size() == tabs.size());
    for(int i = 0; i < tabs.size(); i++)
    {
        auto tab = tabs[static_cast<int>(i)];
        if(auto * etbb = dynamic_cast<ExtendedTabBarButton*>(tab))
        {
            newOrder[i] = etbb->getOption();
        }
        else
        {
            jassertfalse;
    }
    }
    //doubt
    listeners.call([newOrder](Listener& l) { l.tabOrderChanged(newOrder); });
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent &e)
{
      DBG( "ExtendedTabbedButtonBar::mouseDown");
    if( auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>( e.originalComponent ) )
    {
   
        
        startDragging(tabButtonBeingDragged->TabBarButton::getTitle(),
                      tabButtonBeingDragged);
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto dspOption = getDSPOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);
    etbb->addMouseListener(this, false);
    
    return etbb.release();
}
 
 

// Helper functions for drag and drop
juce::TabBarButton* ExtendedTabbedButtonBar::findDraggedItem(const SourceDetails& dragSourceDetails)
{
    return dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get());
}

int ExtendedTabbedButtonBar::findDraggedItemIndex(const SourceDetails& dragSourceDetails)
{
    if (auto draggedItem = findDraggedItem(dragSourceDetails))
        return indexOfTabButton(draggedItem);
    return -1;
}

juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    juce::Array<juce::TabBarButton*> tabs;
    auto numTabs = getNumTabs();
    tabs.resize(numTabs);
    
    for (int i = 0; i < numTabs; ++i)
    {
        tabs.getReference(i) = getTabButton(i);
    }
    
    return tabs;
}

void ExtendedTabbedButtonBar::addListener(Listener* l) 
{ 
    listeners.add(l); 
}

void ExtendedTabbedButtonBar::removeListener(Listener* l) 
{ 
    listeners.remove(l); 
}

DSP_Gui::DSP_Gui(Audio_proAudioProcessor& proc) : processor(proc)
{
    
}
void DSP_Gui::resized()
{

   // to fit the DSP_Gui component's bounds. It lays out buttons at the top, comboBoxes on the left,
   // and sliders in the remaining area. The logic checks if each vector is empty before laying out
   // the corresponding controls, and divides the available space evenly among the controls of each type.
   auto bounds = getLocalBounds();
    if( buttons.empty() == false )
    {
        auto buttonArea = bounds.removeFromTop(30);
        auto w = buttonArea.getWidth() / buttons.size();
        for( auto& button : buttons )
        {
            button->setBounds(buttonArea.removeFromLeft( static_cast<int>(w) ));
        }
    }
    
    if( comboBoxes.empty() == false )
    {
        auto comboBoxArea = bounds.removeFromLeft(bounds.getWidth() / 4);
        auto h = juce::jmin(comboBoxArea.getHeight() / static_cast<int>(comboBoxes.size()), 30);
        for( auto& cb : comboBoxes )
        {
            cb->setBounds(comboBoxArea.removeFromTop( static_cast<int>(h) ));
        }
    }
    
    if( sliders.empty() == false )
    {
        auto w = bounds.getWidth() / sliders.size();
        for( auto& slider : sliders )
        {
            slider->setBounds(bounds.removeFromLeft( static_cast<int>(w) ));
        }
    }
}
void DSP_Gui::paint( juce::Graphics& g )
{
    g.fillAll(juce::Colours::black);
}
void DSP_Gui::rebuildInterface( std::vector< juce::RangedAudioParameter* > params )
{
    //float is sliders
    //bool is checkboxes
    //int is comboboxes
        sliderAttachments.clear();
    comboBoxAttachments.clear();
    buttonAttachments.clear();
    
    sliders.clear();
    comboBoxes.clear();
    buttons.clear();
    for(size_t i = 0; i < params.size(); i++)
    {

        auto p = params[i];

                if( auto* choice = dynamic_cast<juce::AudioParameterChoice*>(params[i]) )
        {
            //make a combobox
            comboBoxes.push_back( std::make_unique<juce::ComboBox>());
            auto& cb = *comboBoxes.back();
            cb.addItemList(choice->choices, 1);
            comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, p->getName(100), cb));
        }
        else if( auto* toggle = dynamic_cast<juce::AudioParameterBool*>(params[i]) )
        {

          
            buttons.push_back(std::make_unique<juce::ToggleButton>("Bypass"));
            auto& btn = *buttons.back();
            buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, p->getName(100), btn));
        }
        else
        {
                   //sliders are used for float and choice params
            sliders.push_back(std::make_unique<RotarySliderWithLabels>(p, p->label, p->getName(100)));
            auto& slider = *sliders.back();
            SimpleMBComp::addLabelPairs(slider.labels, *p, p->label);
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, p->getName(100), slider));
        }
    }
     for( auto& slider : sliders )
        addAndMakeVisible(slider.get());
    for( auto& cb : comboBoxes)
        addAndMakeVisible(cb.get());
    for( auto& btn : buttons )
        addAndMakeVisible(btn.get());
    
    resized();

}



Audio_proAudioProcessorEditor::Audio_proAudioProcessorEditor (Audio_proAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGui);
    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize (600, 400);
}

Audio_proAudioProcessorEditor::~Audio_proAudioProcessorEditor()
{   
    setLookAndFeel(nullptr);
    tabbedComponent.removeListener(this);
    
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

    // dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150,30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGui.setBounds(bounds);
}
void Audio_proAudioProcessorEditor::tabOrderChanged( Audio_proAudioProcessor::DSP_Order newOrder )
{
    DBG("Audio_proAudioProcessorEditor::tabOrderChanged");
    
    // Print the new DSP order
    juce::String orderString = "New DSP Order: ";
    for (int i = 0; i < newOrder.size(); ++i)
    {
        orderString += getNameFromDSPOption(newOrder[i]);
        if (i < newOrder.size() - 1)
            orderString += " -> ";
    }
    DBG(orderString);
    
    // Also print to console for guaranteed visibility
    std::cout << "DSP ORDER CHANGED: " << orderString << std::endl;
    rebuildInterface();
    // Only push to processor when user actually changes the order, not during initialization
    audioProcessor.dspOrderFifo.push(newOrder);
}

void Audio_proAudioProcessorEditor::selectedTabChanged(int newCurrentTabIndex)
{
    DBG("Audio_proAudioProcessorEditor::selectedTabChanged: " << newCurrentTabIndex);
    // Handle tab selection change if needed
}
void Audio_proAudioProcessorEditor::addTabsFromDSPOrder(Audio_proAudioProcessor::DSP_Order newOrder)
{
        tabbedComponent.clearTabs();
    for( auto v : newOrder ) 
    {
        tabbedComponent.addTab(getNameFromDSPOption(v), juce::Colours::white, -1);
    }
 
    rebuildInterface();
    // Don't push back to processor - it already has the correct state

}
void Audio_proAudioProcessorEditor::rebuildInterface()
{
   auto currenttabindex = tabbedComponent.getCurrentTabIndex();
   auto currenttab = tabbedComponent.getTabButton(currenttabindex);
   if( auto etbb = dynamic_cast<ExtendedTabBarButton*>(currenttab) )
   {
        auto dspoption = etbb->getOption();
        auto params = audioProcessor.getparamsforoption(dspoption);
        jassert(params.size() > 0);
        dspGui.rebuildInterface(params);
   }
}
void Audio_proAudioProcessorEditor::timerCallback()
{
    if(audioProcessor.restoredDspOrderFifo.getNumAvailableForReading() ==0)
    return;

    
        using T = Audio_proAudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(Audio_proAudioProcessor::DSP_Option::END_OF_LIST);
    auto empty = newOrder;
    while( audioProcessor.restoredDspOrderFifo.pull(newOrder) )
    {
        ; //do nothing   you'll do something with the most recently pulled order next.
    }
    
    if( newOrder != empty ) //if you pulled nothing, neworder will be filled with END_OF_LIST
    {
        //don't create tabs if neworder is filled with END_OF_LIST
        addTabsFromDSPOrder(newOrder);
    }
    

    

}