/*
License : You are free to use this code as you wish but you must  
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "reaper_plugin_functions.h"
#include <map>
#include <set>

#undef min
#undef max

//==============================================================================
Reaper_api_vstAudioProcessorEditor::Reaper_api_vstAudioProcessorEditor (Reaper_api_vstAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), 
	m_track_vol_slider(Slider::LinearHorizontal,Slider::TextBoxRight), m_resizer(this,nullptr)
{
	addAndMakeVisible(&m_resizer);
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
	addAndMakeVisible(&m_fx_par_assign_but);
	m_fx_par_assign_but.setButtonText("Assign...");
	m_fx_par_assign_but.addListener(this);
	addAndMakeVisible(&m_fx_par_slider);
	m_fx_par_slider.setRange(0.0, 1.0, 0.0);
	m_fx_par_slider.addListener(this);
	if (processor.m_last_w<0 && processor.m_last_h<0)
		setSize (400, 300);
	else setSize(processor.m_last_w, processor.m_last_h);
	startTimer(250);
}

Reaper_api_vstAudioProcessorEditor::~Reaper_api_vstAudioProcessorEditor()
{
}

//==============================================================================
void Reaper_api_vstAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::grey);
	g.setColour (Colours::white);
	MediaTrack* track = processor.getReaperTrack();
	if (track == nullptr)
	{
		g.setFont(15.0f);
		g.drawFittedText("Hello, Reaper API!", getLocalBounds(), Justification::centred, 1);
	}
	else
	{
		double maxpos = 0.0;
		for (int i = 0; i < CountTrackMediaItems(track); ++i)
		{
			MediaItem* item = GetTrackMediaItem(track, i);
			double itempos = GetMediaItemInfo_Value(item, "D_POSITION");
			double itemlen = GetMediaItemInfo_Value(item, "D_LENGTH");
			double item_end = itempos + itemlen;
			maxpos = std::max(maxpos, item_end);
		}
		for (int i = 0; i < CountTrackMediaItems(track); ++i)
		{
			MediaItem* item = GetTrackMediaItem(track, i);
			double itempos = GetMediaItemInfo_Value(item, "D_POSITION");
			double itemlen = GetMediaItemInfo_Value(item, "D_LENGTH");
			double item_end = itempos + itemlen;
			double x0 = jmap<double>(itempos, 0.0, maxpos, 0.0, getWidth());
			double xw = jmap<double>(itemlen, 0.0, maxpos, 0.0, getWidth());
			g.drawRect(x0, 120, xw, 50);
		}
	}
}

void Reaper_api_vstAudioProcessorEditor::resized()
{
	m_track_vol_slider.setBounds(1, 1, getWidth() - 2, 25);
	m_fx_par_slider.setBounds(1, m_track_vol_slider.getBottom() + 1, getWidth() - 100, 25);
	m_fx_par_assign_but.setBounds(m_fx_par_slider.getRight()+1 , m_track_vol_slider.getBottom() + 1, 99 , 25);
	m_test_button.setBounds(1, m_fx_par_assign_but.getBottom()+1, 200, 25);
	
	m_text_ed.setBounds(1, m_test_button.getBottom() +1 , getWidth() - 2, 25);
	m_resizer.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);
	processor.m_last_w = getWidth();
	processor.m_last_h = getHeight();
	m_was_resized = true;
}

void Reaper_api_vstAudioProcessorEditor::sliderValueChanged(Slider * slid)
{
	if (slid == &m_track_vol_slider)
		processor.setTrackVolume(slid->getValue());
	if (slid == &m_fx_par_slider)
	{
		if (m_fx_target_track != nullptr)
		{
			TrackFX_SetParam(m_fx_target_track, m_fx_target_fx, m_fx_target_par, slid->getValue());
		}
	}
}

void Reaper_api_vstAudioProcessorEditor::buttonClicked(Button * but)
{
	if (but == &m_test_button)
		processor.setTakeName("Changed from VST plugin");
	if (but == &m_fx_par_assign_but)
	{
		char parnamebuf[2048];
		PopupMenu menu;
		std::map<MediaTrack*, std::shared_ptr<PopupMenu>> trackmenus;
		std::map<int, std::shared_ptr<PopupMenu>> fxmenus;
		int fxcount = 0;
		int id = 1;
		std::map<int,std::tuple<MediaTrack*, int, int>> parmap;
		for (int i = 0; i < CountTracks(nullptr); ++i)
		{
			MediaTrack* track = GetTrack(nullptr, i);
			if (trackmenus.count(track) == 0)
			{
				trackmenus[track] = std::make_shared<PopupMenu>();
			}
			for (int j = 0; j < TrackFX_GetCount(track); ++j)
			{
				if (fxmenus.count(fxcount) == 0)
				{
					fxmenus[fxcount] = std::make_shared<PopupMenu>();
				}
				for (int k = 0; k < TrackFX_GetNumParams(track, j); ++k)
				{
					
					bool r = TrackFX_GetParamName(track, j, k, parnamebuf, 2048);
					if (r)
					{
						String parname = CharPointer_UTF8(parnamebuf);
						fxmenus[fxcount]->addItem(id, parname, true, false);
						parmap[id] = { track,j,k };
						++id;
					}
				}
				TrackFX_GetFXName(track, j, parnamebuf, 2048);
				trackmenus[track]->addSubMenu(CharPointer_UTF8(parnamebuf), *fxmenus[fxcount]);
				++fxcount;
			}
			menu.addSubMenu(String(i), *trackmenus[track], true);
		}
		int r = menu.show();
		if (r > 0)
		{
			m_fx_target_track = std::get<0>(parmap[r]);
			m_fx_target_fx = std::get<1>(parmap[r]);
			m_fx_target_par = std::get<2>(parmap[r]);
			double nval = TrackFX_GetParamNormalized(m_fx_target_track, m_fx_target_fx, m_fx_target_par);
			m_fx_par_slider.setValue(nval, dontSendNotification);
		}
	}
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

void Reaper_api_vstAudioProcessorEditor::timerCallback()
{
	repaint();
	if (m_was_resized == true)
	{
		processor.extendedStateHasChanged();
		m_was_resized = false;
	}
}
