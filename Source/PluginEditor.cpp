/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Reaper_api_vstAudioProcessorEditor::Reaper_api_vstAudioProcessorEditor (Reaper_api_vstAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), 
	m_track_vol_slider(Slider::LinearHorizontal,Slider::TextBoxRight)
{
	printf("xenakios editor ctor %p", processor.getReaperTrack());	
	addAndMakeVisible(&m_track_vol_slider);
	//if (processor.getReaperTrack() == nullptr)
	//	m_track_vol_slider.setEnabled(false);
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
    g.fillAll (Colours::white);

    g.setColour (Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Hello, Reaper API!", getLocalBounds(), Justification::centred, 1);
}

void Reaper_api_vstAudioProcessorEditor::resized()
{
	m_track_vol_slider.setBounds(1, 1, getWidth() - 2, 25);
	m_test_button.setBounds(1, 30, 200, 25);
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
