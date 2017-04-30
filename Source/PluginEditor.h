/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class Reaper_api_vstAudioProcessorEditor  : public AudioProcessorEditor, 
	public Slider::Listener, public Button::Listener, public TextEditor::Listener
{
public:
    Reaper_api_vstAudioProcessorEditor (Reaper_api_vstAudioProcessor&);
    ~Reaper_api_vstAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void sliderValueChanged(Slider* slid) override;
	void buttonClicked(Button* but) override;
	void textEditorTextChanged(TextEditor& ed) override;
private:
    Reaper_api_vstAudioProcessor& processor;
	Slider m_track_vol_slider;
	TextButton m_test_button;
	TextEditor m_text_ed;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reaper_api_vstAudioProcessorEditor)
};
