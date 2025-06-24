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

// Modern color scheme for this plugin
namespace PluginColors
{
// Primary brand colors - deeper, more professional
inline juce::Colour getPrimaryBlue() { return juce::Colour(0xff2196f3); }
inline juce::Colour getPrimaryBlueLight() { return juce::Colour(0xff64b5f6); }
inline juce::Colour getPrimaryBlueDark() { return juce::Colour(0xff1976d2); }
inline juce::Colour getAccentCyan() { return juce::Colour(0xff00bcd4); }
inline juce::Colour getAccentOrange() { return juce::Colour(0xffff5722); }

// Background colors - more sophisticated gradients
inline juce::Colour getWindowBackground() { return juce::Colour(0xff1a1a1a); }
inline juce::Colour getModuleBackground() { return juce::Colour(0xff2d2d2d); }
inline juce::Colour getModuleBackgroundLight() { return juce::Colour(0xff3a3a3a); }
inline juce::Colour getCardBackground() { return juce::Colour(0xff424242); }

// Control colors
inline juce::Colour getSliderTrack() { return juce::Colour(0xff4a4a4a); }
inline juce::Colour getSliderThumb() { return getPrimaryBlue(); }
inline juce::Colour getSliderThumbHover() { return getPrimaryBlueLight(); }
inline juce::Colour getSliderValue() { return getAccentCyan(); }

// Meter colors with more distinct levels
inline juce::Colour getMeterGreen() { return juce::Colour(0xff4caf50); }
inline juce::Colour getMeterYellow() { return juce::Colour(0xffff9800); }
inline juce::Colour getMeterOrange() { return juce::Colour(0xffff5722); }
inline juce::Colour getMeterRed() { return juce::Colour(0xfff44336); }

// Text colors
inline juce::Colour getTextPrimary() { return juce::Colour(0xffffffff); }
inline juce::Colour getTextSecondary() { return juce::Colour(0xffb0b0b0); }
inline juce::Colour getTextDisabled() { return juce::Colour(0xff757575); }

// Border and outline colors
inline juce::Colour getBorderLight() { return juce::Colour(0xff616161); }
inline juce::Colour getBorderDark() { return juce::Colour(0xff2a2a2a); }
inline juce::Colour getFocusRing() { return getPrimaryBlueLight().withAlpha(0.6f); }

// Legacy color getters for compatibility
inline juce::Colour getGainReductionColor() { return getMeterYellow(); }
inline juce::Colour getInputSignalColor() { return getAccentCyan(); }
inline juce::Colour getOutputSignalColor() { return getMeterGreen(); }
inline juce::Colour getSliderFillColor() { return getSliderThumb(); }
inline juce::Colour getOrangeBorderColor() { return getAccentOrange(); }
inline juce::Colour getSliderRangeTextColor() { return getSliderValue(); }
inline juce::Colour getSliderBorderColor() { return getBorderLight(); }
inline juce::Colour getThresholdColor() { return getMeterOrange(); }
inline juce::Colour getModuleBorderColor() { return getBorderDark(); }
inline juce::Colour getTitleColor() { return getPrimaryBlue(); }
inline juce::Colour getAnalyzerGridColor() { return getModuleBackground(); }
inline juce::Colour getTickColor() { return getBorderLight(); }
inline juce::Colour getMeterLineColor() { return getBorderLight(); }
inline juce::Colour getScaleTextColor() { return getTextSecondary(); }
}

// Enhanced CustomLookAndFeel class with modern styling
struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    CustomLookAndFeel()
    {
        // Set up modern color scheme
        setColour(juce::ResizableWindow::backgroundColourId, PluginColors::getWindowBackground());
        setColour(juce::Slider::trackColourId, PluginColors::getSliderTrack());
        setColour(juce::Slider::thumbColourId, PluginColors::getSliderThumb());
        setColour(juce::Slider::rotarySliderFillColourId, PluginColors::getSliderThumb());
        setColour(juce::Slider::rotarySliderOutlineColourId, PluginColors::getSliderTrack());
        setColour(juce::ComboBox::backgroundColourId, PluginColors::getCardBackground());
        setColour(juce::ComboBox::outlineColourId, PluginColors::getBorderLight());
        setColour(juce::TextButton::buttonColourId, PluginColors::getCardBackground());
        setColour(juce::TextButton::buttonOnColourId, PluginColors::getPrimaryBlue());
        setColour(juce::TextButton::textColourOffId, PluginColors::getTextPrimary());
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }
    
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace juce;
        
        auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(4);
        auto enabled = slider.isEnabled();
        auto isHover = slider.isMouseOverOrDragging();
        
        // Simplified - removed shadow for performance
        
        // Main knob background with gradient
        juce::ColourGradient knobGradient(
            enabled ? PluginColors::getCardBackground().brighter(0.1f) : PluginColors::getCardBackground().darker(0.2f),
            bounds.getCentreX(), bounds.getY(),
            enabled ? PluginColors::getCardBackground().darker(0.1f) : PluginColors::getCardBackground().darker(0.4f),
            bounds.getCentreX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(knobGradient);
        g.fillEllipse(bounds);
        
        // Inner highlight ring
        auto innerBounds = bounds.reduced(2);
        g.setColour(enabled ? PluginColors::getBorderLight().withAlpha(0.3f) : PluginColors::getBorderDark());
        g.drawEllipse(innerBounds, 1.0f);
        
        // Outer border
        g.setColour(enabled ? (isHover ? PluginColors::getFocusRing() : PluginColors::getBorderLight()) 
                            : PluginColors::getTextDisabled());
        g.drawEllipse(bounds, enabled ? 2.0f : 1.0f);
        
        // Value arc
        if (enabled)
        {
            auto center = bounds.getCentre();
            auto radius = bounds.getWidth() * 0.4f;
            auto lineThickness = 3.0f;
            
            Path valueArc;
            valueArc.addCentredArc(center.x, center.y, radius, radius, 0.0f,
                                 rotaryStartAngle, rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle),
                                 true);
            
            // Value arc with gradient
            auto valueColor = isHover ? PluginColors::getSliderThumbHover() : PluginColors::getSliderThumb();
            g.setColour(valueColor);
            g.strokePath(valueArc, PathStrokeType(lineThickness, PathStrokeType::curved, PathStrokeType::rounded));
            
            // Track arc (remaining portion)
            Path trackArc;
            trackArc.addCentredArc(center.x, center.y, radius, radius, 0.0f,
                                 rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle),
                                 rotaryEndAngle,
                                 true);
            
            g.setColour(PluginColors::getSliderTrack());
            g.strokePath(trackArc, PathStrokeType(lineThickness * 0.7f, PathStrokeType::curved, PathStrokeType::rounded));
        }
        
        // Simple center dot indicator
        if (enabled)
        {
            auto center = bounds.getCentre();
            auto dotRadius = 3.0f;
            auto indicatorRadius = bounds.getWidth() * 0.3f;
            
            auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
            auto dotX = center.x + std::cos(angle - juce::MathConstants<float>::halfPi) * indicatorRadius;
            auto dotY = center.y + std::sin(angle - juce::MathConstants<float>::halfPi) * indicatorRadius;
            
            // Simple dot without shadow
            g.setColour(isHover ? PluginColors::getSliderThumbHover() : PluginColors::getSliderThumb());
            g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);
        }
        
        // Draw label text with better styling
        if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
        {
            g.setFont(juce::Font(juce::FontOptions(static_cast<float>(rswl->getTextHeight()))).boldened());
            auto text = rswl->getDisplayString();
            auto textBounds = bounds.withY(bounds.getBottom() + 5).withHeight(static_cast<float>(rswl->getTextHeight() + 4));
            
            // Simple text without shadow
            g.setColour(enabled ? PluginColors::getTextPrimary() : PluginColors::getTextDisabled());
            g.drawFittedText(text, textBounds.toNearestInt(), juce::Justification::centred, 1);
        }
    }
    
    void drawToggleButton (juce::Graphics &g,
                           juce::ToggleButton & toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool /*shouldDrawButtonAsDown*/) override
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);
        auto buttonIsOn = toggleButton.getToggleState();
        auto isHover = shouldDrawButtonAsHighlighted || toggleButton.isMouseOverOrDragging();
        const float cornerSize = 6.0f;
        
        // Button shadow
        if (buttonIsOn || isHover)
        {
            auto shadowBounds = bounds.expanded(1).toFloat();
            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.fillRoundedRectangle(shadowBounds, cornerSize);
        }
        
        // Button background with gradient
        auto bgColor = buttonIsOn ? PluginColors::getPrimaryBlue() : PluginColors::getCardBackground();
        if (isHover)
            bgColor = bgColor.brighter(0.1f);
            
        juce::ColourGradient buttonGradient(
            bgColor.brighter(0.05f), bounds.getX(), bounds.getY(),
            bgColor.darker(0.05f), bounds.getX(), bounds.getBottom(),
            false
        );
        g.setGradientFill(buttonGradient);
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
        
        // Button border
        auto borderColor = buttonIsOn ? PluginColors::getPrimaryBlueDark() : PluginColors::getBorderLight();
        if (isHover)
            borderColor = PluginColors::getFocusRing();
            
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds.toFloat(), cornerSize, isHover ? 2.0f : 1.0f);
        
        // Button text with better contrast
        g.setFont(juce::Font(juce::FontOptions(bounds.getHeight() * 0.5f)).boldened());
        auto textColor = buttonIsOn ? juce::Colours::white : PluginColors::getTextPrimary();
        
        // Text shadow for better readability
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawFittedText(toggleButton.getButtonText(), bounds.translated(1, 1), juce::Justification::centred, 1);
        
        g.setColour(textColor);
        g.drawFittedText(toggleButton.getButtonText(), bounds, juce::Justification::centred, 1);
    }
    
    void drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        auto isHover = box.isMouseOverOrDragging();
        const float cornerSize = 4.0f;
        
        // Background gradient
        juce::ColourGradient bgGradient(
            PluginColors::getCardBackground().brighter(0.05f), 0, 0,
            PluginColors::getCardBackground().darker(0.05f), 0, height,
            false
        );
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // Border
        g.setColour(isHover ? PluginColors::getFocusRing() : PluginColors::getBorderLight());
        g.drawRoundedRectangle(bounds, cornerSize, isHover ? 2.0f : 1.0f);
        
        // Arrow button area
        auto buttonBounds = juce::Rectangle<int>(buttonX, buttonY, buttonW, buttonH).toFloat();
        
        // Draw arrow
        auto arrowSize = juce::jmin(buttonBounds.getWidth(), buttonBounds.getHeight()) * 0.3f;
        auto arrowBounds = buttonBounds.withSizeKeepingCentre(arrowSize, arrowSize * 0.6f);
        
        juce::Path arrow;
        arrow.startNewSubPath(arrowBounds.getX(), arrowBounds.getY());
        arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getBottom());
        arrow.lineTo(arrowBounds.getRight(), arrowBounds.getY());
        
        g.setColour(PluginColors::getTextSecondary());
        g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};

// Enhanced helper functions for drawing modern module backgrounds
juce::Rectangle<int> getModuleBackgroundArea(juce::Rectangle<int> bounds)
{
    bounds.reduce(6, 6);
    return bounds;
}

void drawModuleBackground(juce::Graphics &g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    auto fbounds = bounds.toFloat();
    const float cornerRadius = 8.0f;
    
    // Outer shadow for depth
    auto shadowBounds = fbounds.expanded(2);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(shadowBounds, cornerRadius + 2);
    
    // Main background with gradient
    juce::ColourGradient bgGradient(
        PluginColors::getModuleBackgroundLight(), fbounds.getCentreX(), fbounds.getY(),
        PluginColors::getModuleBackground(), fbounds.getCentreX(), fbounds.getBottom(),
        false
    );
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(fbounds, cornerRadius);
    
    // Inner highlight
    auto innerBounds = fbounds.reduced(1);
    g.setColour(PluginColors::getBorderLight().withAlpha(0.1f));
    g.drawRoundedRectangle(innerBounds, cornerRadius - 1, 1.0f);
    
    // Outer border
    g.setColour(PluginColors::getBorderDark());
    g.drawRoundedRectangle(fbounds, cornerRadius, 1.5f);
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
    const float cornerRadius = 4.0f;
    
    // Background shadow for depth
    auto shadowBounds = bounds.expanded(1);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(shadowBounds, cornerRadius + 1);
    
    // Background with modern gradient
    juce::ColourGradient bgGradient(
        PluginColors::getModuleBackground().brighter(0.1f), bounds.getCentreX(), bounds.getY(),
        juce::Colours::black, bounds.getCentreX(), bounds.getBottom(),
        false
    );
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, cornerRadius);
    
    // Get current level
    auto currentLevel = getLevelFunc ? getLevelFunc() : level.get();
    
    // Convert to dB if needed
    auto levelDb = currentLevel > 0.0f ? juce::Decibels::gainToDecibels(currentLevel) : NEGATIVE_INFINITY;
    levelDb = juce::jlimit(NEGATIVE_INFINITY, MAX_DECIBELS, levelDb);
    
    // Calculate meter fill height
    auto normalizedLevel = juce::jmap(levelDb, NEGATIVE_INFINITY, MAX_DECIBELS, 0.0f, 1.0f);
    auto meterHeight = bounds.getHeight() * normalizedLevel;
    
    // Draw the meter with segmented LED style
    if (meterHeight > 0)
    {
        auto meterBounds = bounds.reduced(2.0f);
        auto fillHeight = meterBounds.getHeight() * normalizedLevel;
        
        // Number of segments
        const int numSegments = 8;  // Reduced for better performance
        const float segmentHeight = meterBounds.getHeight() / numSegments;
        const float segmentGap = 1.0f;
        
        for (int i = 0; i < numSegments; ++i)
        {
            auto segmentY = meterBounds.getBottom() - (i + 1) * segmentHeight;
            auto segmentRect = juce::Rectangle<float>(
                meterBounds.getX(),
                segmentY + segmentGap * 0.5f,
                meterBounds.getWidth(),
                segmentHeight - segmentGap
            );
            
            // Only draw segments that are within the current level
            if (segmentY + segmentHeight >= meterBounds.getBottom() - fillHeight)
            {
                // Color based on segment position
                float segmentLevel = (float)i / (float)(numSegments - 1);
                juce::Colour segmentColor;
                
                if (segmentLevel > 0.85f)
                    segmentColor = PluginColors::getMeterRed();       // Top 15% - Red
                else if (segmentLevel > 0.7f)
                    segmentColor = PluginColors::getMeterOrange();    // 70-85% - Orange  
                else if (segmentLevel > 0.5f)
                    segmentColor = PluginColors::getMeterYellow();    // 50-70% - Yellow
                else
                    segmentColor = PluginColors::getMeterGreen();     // Bottom 50% - Green
                
                // Add glow effect for active segments
                g.setColour(segmentColor.withAlpha(0.3f));
                g.fillRoundedRectangle(segmentRect.expanded(1), 1.0f);
                
                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentRect, 1.0f);
            }
        }
    }
    
    // Modern border
    g.setColour(PluginColors::getBorderLight());
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
    
    // Scale marks with better styling
    g.setColour(PluginColors::getTextSecondary());
    g.setFont(juce::Font(juce::FontOptions(7.0f)).boldened());
    
    auto drawScaleMark = [&](float dbLevel, const juce::String& text, bool major = false)
    {
        auto y = juce::jmap(dbLevel, NEGATIVE_INFINITY, MAX_DECIBELS, bounds.getBottom() - 2, bounds.getY() + 2);
        auto lineLength = major ? 4.0f : 2.0f;
        
        // Scale line
        g.setColour(PluginColors::getBorderLight().withAlpha(major ? 0.8f : 0.5f));
        g.drawLine(bounds.getRight() - lineLength, y, bounds.getRight(), y, major ? 1.0f : 0.5f);
        
        // Scale text for major marks
        if (major && bounds.getWidth() > 25)
        {
            g.setColour(PluginColors::getTextSecondary());
            g.drawText(text, bounds.getRight() + 2, y - 6, 20, 12, juce::Justification::left);
        }
    };
    
    // Major scale marks
    drawScaleMark(0.0f, "0", true);
    drawScaleMark(-6.0f, "-6", true);
    drawScaleMark(-12.0f, "-12", true);
    drawScaleMark(-18.0f, "-18", true);
    drawScaleMark(-24.0f, "-24", true);
    
    // Minor scale marks
    drawScaleMark(-3.0f, "-3");
    drawScaleMark(-9.0f, "-9");
    drawScaleMark(-15.0f, "-15");
    drawScaleMark(-21.0f, "-21");
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

    // Enhanced meter layout with better proportions
    static constexpr int meterWidth = 36;
    static constexpr int meterPadding = 6;
    static constexpr int meterGap = 6;
    
    auto leftMeterArea = bounds.removeFromLeft(meterWidth).reduced(meterPadding);
    auto rightMeterArea = bounds.removeFromRight(meterWidth).reduced(meterPadding);
    
    // More generous horizontal spacing for better visual separation
    bounds.removeFromLeft(12);
    bounds.removeFromRight(12);
    
    // Split meter areas with labels
    auto leftInputArea = leftMeterArea.removeFromTop((leftMeterArea.getHeight() - meterGap) / 2);
    leftMeterArea.removeFromTop(meterGap);
    auto leftOutputArea = leftMeterArea;
    
    auto rightInputArea = rightMeterArea.removeFromTop((rightMeterArea.getHeight() - meterGap) / 2);
    rightMeterArea.removeFromTop(meterGap);
    auto rightOutputArea = rightMeterArea;
    
    leftInputMeter.setBounds(leftInputArea);
    leftOutputMeter.setBounds(leftOutputArea);
    rightInputMeter.setBounds(rightInputArea);
    rightOutputMeter.setBounds(rightOutputArea);

    // Enhanced control layout with better visual hierarchy
    
    // Buttons at top with improved styling
    if (!buttons.empty())
    {
        auto buttonArea = bounds.removeFromTop(40).reduced(6);
        auto buttonWidth = juce::jmin(120, buttonArea.getWidth() / static_cast<int>(buttons.size()));
        auto totalButtonWidth = buttonWidth * static_cast<int>(buttons.size());
        auto buttonStartX = (buttonArea.getWidth() - totalButtonWidth) / 2;
        
        auto currentX = buttonStartX;
        for (auto& button : buttons)
        {
            button->setBounds(juce::Rectangle<int>(currentX, buttonArea.getY(), buttonWidth - 4, buttonArea.getHeight()));
            currentX += buttonWidth;
        }
        
        bounds.removeFromTop(8); // Spacing after buttons
    }
    
    // ComboBoxes in left column with better proportions
    if (!comboBoxes.empty())
    {
        auto comboBoxArea = bounds.removeFromLeft(juce::jmin(180, bounds.getWidth() / 3));
        auto comboHeight = juce::jmin(32, comboBoxArea.getHeight() / static_cast<int>(comboBoxes.size()));
        
        for (auto& cb : comboBoxes)
        {
            auto cbBounds = comboBoxArea.removeFromTop(comboHeight).reduced(4);
            cb->setBounds(cbBounds);
            comboBoxArea.removeFromTop(4); // Small gap between combo boxes
        }
        
        bounds.removeFromLeft(8); // Spacing after combo boxes
    }
    
    // Sliders with improved layout and spacing
    if (!sliders.empty())
    {
        // Calculate optimal slider width with proper spacing
        auto totalSpacing = (static_cast<int>(sliders.size()) - 1) * 8; // 8px between sliders
        auto availableWidth = bounds.getWidth() - totalSpacing;
        auto sliderWidth = availableWidth / static_cast<int>(sliders.size());
        
        // Ensure minimum slider width for usability
        sliderWidth = juce::jmax(sliderWidth, 60);
        
        for (size_t i = 0; i < sliders.size(); ++i)
        {
            auto sliderBounds = bounds.removeFromLeft(sliderWidth);
            sliders[i]->setBounds(sliderBounds.reduced(2)); // Small padding around each slider
            
            if (i < sliders.size() - 1) // Don't add spacing after last slider
                bounds.removeFromLeft(8);
        }
    }
    
    // Add subtle visual cues - this could be used for parameter grouping in the future
    // (Implementation would go in paint method)
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
    // Apply modern look and feel
    setLookAndFeel(customLookAndFeel.get());
    
    // Add components in proper Z-order
    addAndMakeVisible(analyzer);
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGui);
    
    // Set up interaction listeners
    tabbedComponent.addListener(this);
    
    // Start UI refresh timer (15 FPS for better performance)
    startTimerHz(15);
    
    // Set optimal initial size with 16:10 aspect ratio for modern displays
    setSize(880, 660);
    
    // Enable resizing with constraints
    setResizable(true, true);
    setResizeLimits(650, 450, 1400, 1000);
    
    // Enable keyboard focus for better accessibility
    setWantsKeyboardFocus(true);
    
    // Optional: Add subtle drop shadow for depth (if supported by host)
    // setComponentEffect(&dropShadow);
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
    // Modern multi-layer background
    auto bounds = getLocalBounds().toFloat();
    
    // Base gradient background
    juce::ColourGradient mainGradient(
        PluginColors::getWindowBackground().brighter(0.05f), 
        bounds.getCentreX(), bounds.getY(),
        PluginColors::getWindowBackground().darker(0.1f), 
        bounds.getCentreX(), bounds.getBottom(),
        false
    );
    g.setGradientFill(mainGradient);
    g.fillAll();
    
    // Removed noise texture for better performance
    
    // Title area with brand styling
    auto titleArea = bounds.removeFromTop(25);
    juce::ColourGradient titleGradient(
        PluginColors::getPrimaryBlueDark().withAlpha(0.8f),
        titleArea.getTopLeft(),
        PluginColors::getPrimaryBlue().withAlpha(0.3f),
        titleArea.getBottomLeft(),
        false
    );
    g.setGradientFill(titleGradient);
    g.fillRect(titleArea);
    
    // Plugin title
    g.setColour(PluginColors::getTextPrimary());
    g.setFont(juce::Font(juce::FontOptions(14.0f)).boldened());
    g.drawText("AUDIO PRO", titleArea.reduced(10, 0).toNearestInt(), 
               juce::Justification::left, true);
    
    // Outer border with enhanced styling
    g.setColour(PluginColors::getBorderDark());
    g.drawRect(bounds, 2.0f);
    
    // Inner highlight border
    g.setColour(PluginColors::getBorderLight().withAlpha(0.3f));
    g.drawRect(bounds.reduced(2), 1.0f);
}

void Audio_proAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for title bar
    bounds.removeFromTop(25);
    
    // Add padding around the entire interface
    bounds.reduce(8, 8);
    
    // Enhanced proportions for better visual balance
    // 45% for analyzer (main visual element)
    auto analyzerArea = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.45f));
    
    // Elegant spacing
    bounds.removeFromTop(8);
    
    // 12% for tabbed interface (compact but functional)
    auto tabArea = bounds.removeFromTop(static_cast<int>(bounds.getHeight() * 0.15f));
    tabArea = tabArea.withHeight(juce::jmax(32, tabArea.getHeight())); // Minimum tab height
    tabbedComponent.setBounds(tabArea);
    
    // Another small gap
    bounds.removeFromTop(6);
    
    // Remaining space for DSP controls (approximately 40% of original height)
    dspGui.setBounds(bounds);
    
    // Analyzer with proper padding and rounded corners consideration
    analyzer.setBounds(analyzerArea.reduced(2));
    
    // Ensure minimum size constraints
    auto minSize = juce::Point<int>(650, 450);
    if (getWidth() < minSize.x || getHeight() < minSize.y)
    {
        setSize(juce::jmax(getWidth(), minSize.x), juce::jmax(getHeight(), minSize.y));
    }
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