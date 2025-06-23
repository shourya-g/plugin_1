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

// Custom color scheme for this plugin
namespace PluginColors
{
inline juce::Colour getGainReductionColor() { return juce::Colour(0xff38c19b); }
inline juce::Colour getInputSignalColor() { return juce::Colour(0xff00e7ff); }
inline juce::Colour getOutputSignalColor() { return juce::Colour(0xff0cff00); }
inline juce::Colour getSliderFillColor() { return juce::Colour(0xff96bbc2); }
inline juce::Colour getOrangeBorderColor() { return juce::Colour(255u, 154u, 1u); }
inline juce::Colour getSliderRangeTextColor() { return juce::Colour(0u, 172u, 1u); }
inline juce::Colour getSliderBorderColor() { return juce::Colour(0xff0087a2); }
inline juce::Colour getThresholdColor() { return juce::Colour(0xffe0760c); }
inline juce::Colour getModuleBorderColor() { return juce::Colour(0xff475d9d); }
inline juce::Colour getTitleColor() { return juce::Colour(0xff2c92ca); }
inline juce::Colour getAnalyzerGridColor() { return juce::Colour(0xff262626); }
inline juce::Colour getTickColor() { return juce::Colour(0xff313131); }
inline juce::Colour getMeterLineColor() { return juce::Colour(0xff3c3c3c); }
inline juce::Colour getScaleTextColor() { return juce::Colours::lightgrey; }
}

// Custom LookAndFeel class
struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace juce;
        
        auto bounds = Rectangle<int>(x, y, width, height).toFloat();
        auto enabled = slider.isEnabled();
        
        g.setColour(enabled ? PluginColors::getSliderFillColor() : Colours::darkgrey);
        g.fillEllipse(bounds);
        
        g.setColour(enabled ? PluginColors::getSliderBorderColor() : Colours::grey);
        g.drawEllipse(bounds, 2.f);
        
        if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
        {
            auto center = bounds.getCentre();
            Path p;
            
            Rectangle<float> r;
            r.setLeft(center.getX() - 2);
            r.setRight(center.getX() + 2);
            r.setTop(bounds.getY());
            r.setBottom(jmax(center.getY() - rswl->getTextHeight() * 1.5f,
                            bounds.getY() + 15));
            
            p.addRoundedRectangle(r, 2.f);
            
            jassert(rotaryStartAngle < rotaryEndAngle);
            
            auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
            
            p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
            
            g.fillPath(p);
            
            g.setFont(static_cast<float>(rswl->getTextHeight()));
            auto text = rswl->getDisplayString();
            auto strWidth = g.getCurrentFont().getStringWidth(text);
            
            r.setSize(static_cast<float>(strWidth + 4), 
                      static_cast<float>(rswl->getTextHeight() + 2));
            r.setCentre(bounds.getCentre());
            
            g.setColour(enabled ? PluginColors::getTitleColor() : Colours::lightgrey);
            g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
        }
    }
    
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool /*shouldDrawButtonAsHighlighted*/,
                           bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);
        auto buttonIsOn = toggleButton.getToggleState();
        const int cornerSize = 4;

        g.setColour(buttonIsOn ?
                    PluginColors::getSliderRangeTextColor() :
                    PluginColors::getModuleBorderColor());
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
        
        g.setColour(buttonIsOn ? juce::Colours::white : PluginColors::getTitleColor());
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, 1);
        g.setColour(buttonIsOn ? juce::Colours::white : PluginColors::getTitleColor());
        g.drawFittedText(toggleButton.getButtonText(), bounds, juce::Justification::centred, 1);
    }
};

// Helper functions for drawing module backgrounds
juce::Rectangle<int> getModuleBackgroundArea(juce::Rectangle<int> bounds)
{
    bounds.reduce(3, 3);
    return bounds;
}

void drawModuleBackground(juce::Graphics &g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(PluginColors::getModuleBorderColor());
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3, 3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
}

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

LevelMeter::LevelMeter(std::function<float()> levelGetter) : getLevelFunc(std::move(levelGetter))
{
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background with rounded corners
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(bounds, 2.0f);
    
    // Border with color scheme
    g.setColour(PluginColors::getMeterLineColor());
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    
    // Get current level
    auto currentLevel = getLevelFunc ? getLevelFunc() : level.get();
    
    // Convert to dB if needed
    auto levelDb = currentLevel > 0.0f ? juce::Decibels::gainToDecibels(currentLevel) : NEGATIVE_INFINITY;
    
    // Clamp to our range
    levelDb = juce::jlimit(NEGATIVE_INFINITY, MAX_DECIBELS, levelDb);
    
    // Calculate meter fill height
    auto normalizedLevel = juce::jmap(levelDb, NEGATIVE_INFINITY, MAX_DECIBELS, 0.0f, 1.0f);
    auto meterHeight = bounds.getHeight() * normalizedLevel;
    
    // Draw the meter with gradient effect
    if (meterHeight > 0)
    {
        auto meterRect = bounds.withHeight(meterHeight).withBottomY(bounds.getBottom()).reduced(1.0f);
        
        // Color based on level using color scheme
        if (levelDb > 0.0f)
        {
            g.setColour(PluginColors::getThresholdColor());  // Over 0dB = threshold color
        }
        else if (levelDb > -6.0f)
        {
            g.setColour(PluginColors::getGainReductionColor());  // -6dB to 0dB = gain reduction color
        }
        else
        {
            g.setColour(PluginColors::getInputSignalColor());   // Below -6dB = input signal color
        }
        
        g.fillRoundedRectangle(meterRect, 1.0f);
    }
    
    // Draw scale marks
    g.setColour(PluginColors::getScaleTextColor());
    g.setFont(8.0f);
    
    auto drawScaleMark = [&](float dbLevel, const juce::String& text)
    {
        auto y = juce::jmap(dbLevel, NEGATIVE_INFINITY, MAX_DECIBELS, bounds.getBottom(), bounds.getY());
        g.drawLine(bounds.getX(), y, bounds.getX() + 3, y);
        g.drawText(text, bounds.getX() + 4, y - 4, 15, 8, juce::Justification::left);
    };
    
    drawScaleMark(0.0f, "0");
    drawScaleMark(-6.0f, "-6");
    drawScaleMark(-12.0f, "-12");
    drawScaleMark(-18.0f, "-18");
}

void LevelMeter::setLevel(float newLevel)
{
    level.set(newLevel);
    repaint();
}

////////

ExtendedTabbedButtonBar::ExtendedTabbedButtonBar(Audio_proAudioProcessor& proc) : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop), processor(proc) 
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
    
    auto* button = etbb.release();
    
    // We'll add the bypass button in a timer callback since the tab needs to be added first
    juce::Timer::callAfterDelay(1, [this, tabIndex, dspOption]() {
        addBypassButtonToTab(tabIndex, dspOption);
    });
    
    return button;
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

void ExtendedTabbedButtonBar::addBypassButtonToTab(int tabIndex, Audio_proAudioProcessor::DSP_Option option)
{
    auto* tabButton = getTabButton(tabIndex);
    if (!tabButton) return;
    
    // Create bypass button
    auto bypassButton = std::make_unique<juce::ToggleButton>();
    bypassButton->setButtonText("X");
    bypassButton->setTooltip("Bypass " + getNameFromDSPOption(option));
    bypassButton->setSize(16, 16);
    
    // Get the bypass parameter for this DSP option
    juce::AudioParameterBool* bypassParam = nullptr;
    switch (option)
    {
        case Audio_proAudioProcessor::DSP_Option::Phase:
            bypassParam = processor.phaserBypass;
            break;
        case Audio_proAudioProcessor::DSP_Option::Chorus:
            bypassParam = processor.chorusBypass;
            break;
        case Audio_proAudioProcessor::DSP_Option::Overdrive:
            bypassParam = processor.overdriveBypass;
            break;
        case Audio_proAudioProcessor::DSP_Option::LadderFilter:
            bypassParam = processor.ladderFilterBypass;
            break;
        case Audio_proAudioProcessor::DSP_Option::GeneralFilter:
            bypassParam = processor.generalFilterBypass;
            break;
        case Audio_proAudioProcessor::DSP_Option::END_OF_LIST:
        default:
            return; // No bypass parameter for this option
    }
    
    if (bypassParam)
    {
        // Create attachment to connect button to parameter
        auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processor.apvts, bypassParam->getName(100), *bypassButton);
        
        // Store the attachment so it doesn't get destroyed
        bypassAttachments.push_back(std::move(attachment));
        
        // Set the bypass button as extra component on the tab
        tabButton->setExtraComponent(bypassButton.release(), juce::TabBarButton::afterText);
    }
}

DSP_Gui::DSP_Gui(Audio_proAudioProcessor& proc) : processor(proc),
    leftInputMeter([&proc]() { return proc.leftPreRMS.get(); }),
    rightInputMeter([&proc]() { return proc.rightPreRMS.get(); }),
    leftOutputMeter([&proc]() { return proc.leftPostRMS.get(); }),
    rightOutputMeter([&proc]() { return proc.rightPostRMS.get(); })
{
    addAndMakeVisible(leftInputMeter);
    addAndMakeVisible(rightInputMeter);
    addAndMakeVisible(leftOutputMeter);
    addAndMakeVisible(rightOutputMeter);
}
void DSP_Gui::resized()
{
    auto bounds = getModuleBackgroundArea(getLocalBounds());
    
    // Reserve space for meters at the left and right with better spacing
    static constexpr int meterWidth = 32;
    static constexpr int meterPadding = 4;
    
    auto leftMeterArea = bounds.removeFromLeft(meterWidth).reduced(meterPadding);
    auto rightMeterArea = bounds.removeFromRight(meterWidth).reduced(meterPadding);
    
    // Add some horizontal spacing
    bounds.removeFromLeft(8);
    bounds.removeFromRight(8);
    
    // Split meter areas vertically for input/output meters with gap
    auto leftInputArea = leftMeterArea.removeFromTop((leftMeterArea.getHeight() - 4) / 2);
    leftMeterArea.removeFromTop(4); // gap
    auto leftOutputArea = leftMeterArea;
    
    auto rightInputArea = rightMeterArea.removeFromTop((rightMeterArea.getHeight() - 4) / 2);
    rightMeterArea.removeFromTop(4); // gap  
    auto rightOutputArea = rightMeterArea;
    
    leftInputMeter.setBounds(leftInputArea);
    leftOutputMeter.setBounds(leftOutputArea);
    rightInputMeter.setBounds(rightInputArea);
    rightOutputMeter.setBounds(rightOutputArea);

   // to fit the DSP_Gui component's bounds. It lays out buttons at the top, comboBoxes on the left,
   // and sliders in the remaining area. The logic checks if each vector is empty before laying out
   // the corresponding controls, and divides the available space evenly among the controls of each type.
    if( buttons.empty() == false )
    {
        auto buttonArea = bounds.removeFromTop(35).reduced(4);
        auto w = buttonArea.getWidth() / buttons.size();
        for( auto& button : buttons )
        {
            button->setBounds(buttonArea.removeFromLeft( static_cast<int>(w) ).reduced(2));
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
    // Draw modern module background
    drawModuleBackground(g, getLocalBounds());
}

void DSP_Gui::updateMeters()
{
    leftInputMeter.repaint();
    rightInputMeter.repaint();
    leftOutputMeter.repaint();
    rightOutputMeter.repaint();
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
    : AudioProcessorEditor (&p), audioProcessor (p), customLookAndFeel(std::make_unique<CustomLookAndFeel>())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be
    setLookAndFeel(customLookAndFeel.get());
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGui);
    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize (770, 400);
}

Audio_proAudioProcessorEditor::~Audio_proAudioProcessorEditor()
{   
    setLookAndFeel(nullptr);
    tabbedComponent.removeListener(this);
    customLookAndFeel.reset();
}

//==============================================================================
void Audio_proAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Modern gradient background
    auto bounds = getLocalBounds().toFloat();
    
    juce::ColourGradient gradient(PluginColors::getAnalyzerGridColor(), 
                                  bounds.getTopLeft(),
                                  juce::Colours::black, 
                                  bounds.getBottomRight(), 
                                  false);
    
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Optional: Add a subtle border
    g.setColour(PluginColors::getModuleBorderColor());
    g.drawRect(getLocalBounds(), 1);
}

void Audio_proAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto leftmeterarea= bounds.removeFromLeft(meterWidth);
    auto rightmeterarea= bounds.removeFromRight(meterWidth);
    juce::ignoreUnused(leftmeterarea, rightmeterarea);
    // dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150,30));
   
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
    // Update meters
    dspGui.updateMeters();
    
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
    

    
   repaint();
}