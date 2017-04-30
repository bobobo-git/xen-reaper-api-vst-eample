/*
License : You are free to use this code as you wish but you must
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"

// At least the last time tested, Reaper on Linux doesn't support ShowConsoleMsg
#ifndef JUCE_LINUX
void LogToReaper(String txt)
{
	ShowConsoleMsg(txt.toRawUTF8());
}
#else
void LogToReaper(String txt)
{
	printf(txt.toRawUTF8());
}
#endif

//==============================================================================
Reaper_api_vstAudioProcessor::Reaper_api_vstAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Reaper_api_vstAudioProcessor::~Reaper_api_vstAudioProcessor()
{
}

//==============================================================================
const String Reaper_api_vstAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Reaper_api_vstAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Reaper_api_vstAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double Reaper_api_vstAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Reaper_api_vstAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Reaper_api_vstAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Reaper_api_vstAudioProcessor::setCurrentProgram (int index)
{
}

const String Reaper_api_vstAudioProcessor::getProgramName (int index)
{
    return String();
}

void Reaper_api_vstAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Reaper_api_vstAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Reaper_api_vstAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Reaper_api_vstAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Reaper_api_vstAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool Reaper_api_vstAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Reaper_api_vstAudioProcessor::createEditor()
{
    return new Reaper_api_vstAudioProcessorEditor (*this);
}

//==============================================================================
void Reaper_api_vstAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Reaper_api_vstAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


void Reaper_api_vstAudioProcessor::afterCreate()
{
	if (getProperties().contains("audioMasterCallback") == true)
	{
		// VST2
		var cbvar = getProperties()["audioMasterCallback"];
		m_host_cb = (VstHostCallback)(int64)cbvar;
		if (m_host_cb == nullptr)
		{
			LogToReaper("audiomastercallback null\n");
			return;
		}
		int errcnt = REAPERAPI_LoadAPI([this](const char* funcname)
		{
			return (void*)m_host_cb(NULL, 0xdeadbeef, 0xdeadf00d, 0, (void*)funcname, 0.0);
		});
		if (errcnt > 0)
			LogToReaper("errors when loading reaper api funcs\n");
		var aevar = getProperties()["aeffect"];
		m_ae = (VstEffectInterface*)(int64)aevar;
		if (m_ae == nullptr)
			LogToReaper("aeffect is null\n");
		return;
	}

}

// Demonstrates how to get the Reaper MediaTrack where the plugin is loaded.

void Reaper_api_vstAudioProcessor::setTrackVolume(double gain)
{
	MediaTrack* tr = getReaperTrack();
	if (tr != nullptr)
	{
		SetMediaTrackInfo_Value(tr, "D_VOL", gain);
	}
}

String Reaper_api_vstAudioProcessor::getTakeName()
{
	const char* name = GetTakeName(getReaperTake());
	if (name != nullptr)
		return String(CharPointer_UTF8(name));
	return String();
}

void Reaper_api_vstAudioProcessor::setTakeName(String name)
{
	MediaItem_Take* tk = getReaperTake();
	if (tk != nullptr)
	{
		GetSetMediaItemTakeInfo_String(tk, "P_NAME", (char*)name.toRawUTF8(), true);
		UpdateArrange();
	}
	else
	{
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Error", "Plugin is not loaded into a Reaper take", "OK");
	}
}

String Reaper_api_vstAudioProcessor::getTrackName()
{
	char buf[2048];
	if (GetSetMediaTrackInfo_String(getReaperTrack(), "P_NAME", buf, false) == true)
		return String(CharPointer_UTF8(buf));
	return String();
}

void Reaper_api_vstAudioProcessor::setTrackName(String name)
{
	GetSetMediaTrackInfo_String(getReaperTrack(), "P_NAME", (char*)name.toRawUTF8(), true);
	UpdateArrange();
}

MediaTrack * Reaper_api_vstAudioProcessor::getReaperTrack()
{
	if (m_ae == nullptr)
		return nullptr;
	return (MediaTrack*)m_host_cb(m_ae, 0xDEADBEEF, 0xDEADF00E, 1, 0, 0.0);
}

MediaItem_Take * Reaper_api_vstAudioProcessor::getReaperTake()
{
	if (m_ae == nullptr)
		return nullptr;
	return (MediaItem_Take*)m_host_cb(m_ae, 0xDEADBEEF, 0xDEADF00E, 2, 0, 0.0);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reaper_api_vstAudioProcessor();
}
