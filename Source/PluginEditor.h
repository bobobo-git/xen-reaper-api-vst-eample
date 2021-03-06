/*
License : You are free to use this code as you wish but you must
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class Reaper_api_vstAudioProcessorEditor  : public AudioProcessorEditor, 
	public Slider::Listener, public Button::Listener, public TextEditor::Listener,
	public Timer
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
	void timerCallback() override;
private:
    Reaper_api_vstAudioProcessor& processor;
	Slider m_track_vol_slider;
	Slider m_fx_par_slider;
	TextButton m_fx_par_assign_but;
	TextButton m_test_button;
	TextEditor m_text_ed;
	ResizableCornerComponent m_resizer;
	MediaTrack* m_fx_target_track = nullptr;
	int m_fx_target_fx = -1;
	int m_fx_target_par = -1;
	bool m_was_resized = false;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reaper_api_vstAudioProcessorEditor)
};
