/*
License : You are free to use this code as you wish but you must  
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Reaper_api_vstAudioProcessorEditor::Reaper_api_vstAudioProcessorEditor (Reaper_api_vstAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), 
	m_track_vol_slider(Slider::LinearHorizontal,Slider::TextBoxRight)
{
	addAndMakeVisible(&m_track_vol_slider);
	addAndMakeVisible(&m_text_ed);
	m_text_ed.addListener(this);
	if (processor.getReaperTrack() != nullptr)
		m_text_ed.setText(processor.getTrackName(), dontSendNotification);
	if (processor.getReaperTake() != nullptr)
		m_text_ed.setText(processor.getTakeName(), dontSendNotification);
	if (processor.getReaperTrack() == nullptr)
		m_track_vol_slider.setEnabled(false);
	m_track_vol_slider.setRange(0.0, 1.0);
	m_track_vol_slider.addListener(this);
	addAndMakeVisible(&m_test_button);
	m_test_button.addListener(this);
	m_test_button.setButtonText("Take FX test");
	setSize (400, 300);
}

Reaper_api_vstAudioProcessorEditor::~Reaper_api_vstAudioProcessorEditor()
{
}

//==============================================================================
void Reaper_api_vstAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::grey);
	g.setColour (Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Hello, Reaper API!", getLocalBounds(), Justification::centred, 1);
}

void Reaper_api_vstAudioProcessorEditor::resized()
{
	m_track_vol_slider.setBounds(1, 1, getWidth() - 2, 25);
	m_test_button.setBounds(1, 30, 200, 25);
	m_text_ed.setBounds(1, 60, getWidth() - 2, 25);
}

void Reaper_api_vstAudioProcessorEditor::sliderValueChanged(Slider * slid)
{
	if (slid == &m_track_vol_slider)
		processor.setTrackVolume(slid->getValue());
}

void Reaper_api_vstAudioProcessorEditor::buttonClicked(Button * but)
{
	if (but == &m_test_button)
		processor.setTakeName("Changed from VST plugin");
}

void Reaper_api_vstAudioProcessorEditor::textEditorTextChanged(TextEditor & ed)
{
	if (&ed == &m_text_ed)
	{
		if (processor.getReaperTake() != nullptr)
			processor.setTakeName(m_text_ed.getText());
		if (processor.getReaperTrack() != nullptr)
			processor.setTrackName(m_text_ed.getText());
	}
}
