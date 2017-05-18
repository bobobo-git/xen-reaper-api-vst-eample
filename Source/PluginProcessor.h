/*
License : You are free to use this code as you wish but you must
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "juce_audio_processors/format_types/juce_VSTInterface.h"

#include "breakpoint_envelope.h"

template<typename F>
inline void iterateMidiBuffer(const MidiBuffer& mb, F&& f)
{
	if (mb.isEmpty())
		return;
	MidiBuffer::Iterator msgiter(mb);
	MidiMessage msg;
	int pos = 0;
	for (int i = 0; i < mb.getNumEvents(); ++i)
	{
		msgiter.getNextEvent(msg, pos);
		f(msg, pos);
	}
}


void LogToReaper(String txt);

class MediaTrack;
class MediaItem_Take;

class Reaper_api_vstAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Reaper_api_vstAudioProcessor();
    ~Reaper_api_vstAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	void afterCreate() override;
	void setTrackVolume(double gain);
	String getTakeName();
	void setTakeName(String name);
	String getTrackName();
	void setTrackName(String name);
	MediaTrack* getReaperTrack();
	MediaItem_Take* getReaperTake();
	void extendedStateHasChanged();
	int m_last_w = -1;
	int m_last_h = -1;
private:
	VstHostCallback m_host_cb = nullptr;
	VstEffectInterface* m_ae = nullptr;
	struct env_info
	{
		env_info() {}
		breakpoint_envelope m_env;
		double m_tpos = 0.0;
		int m_note_number = 0;
		bool m_playing = false;
		double m_len = 1.0;
		int m_track_index = 0;
		int m_fx_index = 0;
		int m_param_index = 0;
	};
	std::vector<env_info> m_envs;
	//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reaper_api_vstAudioProcessor)
};
