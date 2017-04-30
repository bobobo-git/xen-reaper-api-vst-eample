/*
License : You are free to use this code as you wish but you must
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "juce_audio_processors/format_types/juce_VSTInterface.h"


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
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Reaper_api_vstAudioProcessor)
};
