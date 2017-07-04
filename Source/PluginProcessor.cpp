/*
License : You are free to use this code as you wish but you must
respect the separate licensing of JUCE and the Reaper SDK files.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"

#include "reapervst3.h"

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
	m_envs.emplace_back(48, 1.0, 1, 0, 0);
	m_envs.back().m_env.AddNode({ 0.0,0.0 });
	m_envs.back().m_env.AddNode({ 0.5,1.0 });
	m_envs.back().m_env.AddNode({ 1.0,0.0 });
	
	m_envs.emplace_back(60, 2.0, 1, 0, 0);
	m_envs.back().m_env.AddNode({ 0.0,1.0 });
	m_envs.back().m_env.AddNode({ 0.25,0.0 });
	m_envs.back().m_env.AddNode({ 1.75,0.0 });
	m_envs.back().m_env.AddNode({ 2.00,1.0 });
	
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
    
}

void Reaper_api_vstAudioProcessor::releaseResources()
{
    
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

void Reaper_api_vstAudioProcessor::handleMIDIMessages(MidiBuffer& messages)
{
	// Slightly inefficient algorithm here, the envelope infos are iterated over again for each note on message...
	iterateMidiBuffer(messages, [this](MidiMessage& msg, int pos)
	{
		if (msg.isNoteOn() == true)
		{
			for (auto& e : m_envs)
			{
				if (e.m_note_number == msg.getNoteNumber())
				{
					e.m_tpos = 0.0;
					e.m_playing = true;
				}
			}
		}
	});
}

void Reaper_api_vstAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	handleMIDIMessages(midiMessages);
	for (auto& e : m_envs)
	{
		if (e.m_playing == true)
		{
			if (e.m_tpos < e.m_len)
			{
				double v = e.m_env.GetInterpolatedNodeValue(e.m_tpos);
				MediaTrack* track = GetTrack(nullptr, e.m_track_index);
				TrackFX_SetParamNormalized(track, e.m_fx_index, e.m_param_index, v);
				e.m_tpos += (double)buffer.getNumSamples() / getSampleRate();
			}
			else
			{
				e.m_playing = false;
			}
		}
	}
	const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

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
    return true; 
}

AudioProcessorEditor* Reaper_api_vstAudioProcessor::createEditor()
{
    return new Reaper_api_vstAudioProcessorEditor (*this);
}

//==============================================================================
void Reaper_api_vstAudioProcessor::getStateInformation (MemoryBlock& destData)
{
	ValueTree vt("reaperexamplestate");
	vt.setProperty("gui_w", m_last_w, nullptr);
	vt.setProperty("gui_h", m_last_h, nullptr);
	MemoryOutputStream ms(destData, false);
	vt.writeToStream(ms);
	//LogToReaper("Created state chunk of size " + String(destData.getSize())+"\n");
	//LogToReaper("Stored GUI size is " + String(m_last_w) + "x" + String(m_last_h)+"\n");
}

void Reaper_api_vstAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	//LogToReaper("Restoring state from chunk of size " + String(sizeInBytes)+"\n");
	ValueTree vt = ValueTree::readFromData(data, sizeInBytes);
	if (vt.isValid())
	{
		m_last_w = vt.getProperty("gui_w");
		m_last_h = vt.getProperty("gui_h");
		//LogToReaper("Restored GUI size is " + String(m_last_w) + "x" + String(m_last_h)+"\n");
		if (getActiveEditor())
			getActiveEditor()->setSize(m_last_w, m_last_h);
	}
	else
	{
		//LogToReaper("Chunk did not contain valid ValueTree\n");
	}
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
		if (errcnt > 0 && ShowConsoleMsg!=nullptr)
			LogToReaper("some errors when loading reaper api functions\n");
		var aevar = getProperties()["aeffect"];
		m_ae = (VstEffectInterface*)(int64)aevar;
		if (m_ae == nullptr)
			LogToReaper("aeffect is null\n");
		return;
	}
	if (getProperties().contains("hostctx") == true)
	{
		// VST3
		auto hostctx = (Steinberg::FUnknown*)(int)getProperties()["hostctx"];
		IReaperHostApplication *reaperptr = nullptr;
		hostctx->queryInterface(IReaperHostApplication::iid, (void**)&reaperptr);
		if (reaperptr != nullptr)
		{
			int errcnt = REAPERAPI_LoadAPI([reaperptr](const char* funcname)
			{
				return reaperptr->getReaperApi(funcname);
			});
			LogToReaper("got reaper vst3 host context\n");
			if (errcnt>0 && ShowConsoleMsg!=nullptr)
				LogToReaper("some errors when loading reaper api functions\n");
		} 
	}
}

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

// Demonstrates how to get the Reaper MediaTrack where the plugin is loaded.

MediaTrack * Reaper_api_vstAudioProcessor::getReaperTrack()
{
	if (m_ae == nullptr)
		return nullptr;
	return (MediaTrack*)m_host_cb(m_ae, 0xDEADBEEF, 0xDEADF00E, 1, 0, 0.0);
}

// Demonstrates how to get the Reaper MediaItem_Take where the plugin is loaded.

MediaItem_Take * Reaper_api_vstAudioProcessor::getReaperTake()
{
	if (m_ae == nullptr)
		return nullptr;
	return (MediaItem_Take*)m_host_cb(m_ae, 0xDEADBEEF, 0xDEADF00E, 2, 0, 0.0);
}

void Reaper_api_vstAudioProcessor::extendedStateHasChanged()
{
	// Use VST2 API directly to notify Reaper of change for parameter -1. 
	// That will be considered a generic state change of the plugin in Reaper so that Reaper will query the current plugin state
	// and add an undo entry etc.
	// Doing it this way seems to be necessary since JUCE does not support notifying a change for parameter -1.
	if (m_host_cb!=nullptr)
		m_host_cb(m_ae, hostOpcodeParameterChanged, -1, 0, nullptr, 0.0f);
	// vst3...uhum...
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Reaper_api_vstAudioProcessor();
}
