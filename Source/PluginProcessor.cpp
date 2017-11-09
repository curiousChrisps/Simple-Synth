#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter();
Voice* JUCE_CALLTYPE createVoice(); // callback to student's code to create a single voice instance (e.g. new MyVoice())
Synth* JUCE_CALLTYPE createSynth(); // callback to create the synthesiser instance (e.g. new MySynthesiser())

//==============================================================================
PluginAudioProcessor::PluginAudioProcessor()
: pEditor(NULL)
{
    lastUIWidth = 640;
    lastUIHeight = 320;

    lastPosInfo.resetToDefault();

    synth = createSynth();
    synth->addSound (new SimpleSound());
    
    // Initialise the synth...
    for (int i = 32; --i >= 0;){
        Voice* pVoice = createVoice();
        pVoice->setParameters(synth);
        pVoice->setSynthesiser(reinterpret_cast<MySynth*>(synth));
        synth->addVoice (pVoice);   // These voices will play our custom sine-wave sounds..
    }
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    delete synth;
    synth = NULL;
}

//==============================================================================
int PluginAudioProcessor::getNumParameters()
{
    return kNumberOfParameters;
}

float PluginAudioProcessor::getParameter (int index)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    return synth->getParameter(index);
}

void PluginAudioProcessor::setParameter (int index, float newValue)
{
    // This method will be called by the host, probably on the audio thread, so
    // it's absolutely time-critical. Don't use critical sections or anything
    // UI-related, or anything at all that may block in any way!
    synth->setParameter(index, newValue);
}

const String PluginAudioProcessor::getParameterName (int index)
{
    return synth->getParameterName(index);
}

const String PluginAudioProcessor::getParameterText (int index)
{
    return String (getParameter (index), 2);
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth->setCurrentPlaybackSampleRate (sampleRate);
    keyboardState.reset();
    
    stk::Stk::setSampleRate(sampleRate);
}

void PluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    keyboardState.reset();
}

void PluginAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
}

void PluginAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();
    
    // Pass any incoming midi messages to our keyboard state object, and let it
    // add messages to the buffer if the user is clicking on the on-screen keys
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, numSamples);
    
    // and now get the synth to process these midi events and generate its output.
    synth->renderNextBlock (buffer, midiMessages, 0, numSamples);
    synth->postProcess(buffer.getArrayOfChannels(), getNumOutputChannels(), numSamples);
    
    if (getActiveEditor()){
        PluginAudioProcessorEditor* editor = dynamic_cast<PluginAudioProcessorEditor*>(pEditor);

        if(editor && editor->scope_mode & SCOPE_VISIBLE){
            if(editor->oscilloscope && (editor->scope_mode & SCOPE_OSCILLOSCOPE))
                editor->oscilloscope->processBlock(buffer.getSampleData(0), numSamples);
            else if(editor->spectrum && (editor->scope_mode & SCOPE_SPECTRUM))
                editor->spectrum->copySamples(buffer.getSampleData(0), numSamples);
            else if(editor->sonogram)
                editor->sonogram->copySamples(buffer.getSampleData(0), numSamples);
        }
    }
    
    // ask the host for the current time so we can display it...
    AudioPlayHead::CurrentPositionInfo newTime;

    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition (newTime))
    {
        // Successfully got the current time from the host..
        lastPosInfo = newTime;
    }
    else
    {
        // If the host fails to fill-in the current time, we'll just clear it to a default..
        lastPosInfo.resetToDefault();
    }
}

//==============================================================================
AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
    return pEditor = new PluginAudioProcessorEditor (this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:

    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute ("uiWidth", lastUIWidth);
    xml.setAttribute ("uiHeight", lastUIHeight);
    
    for(int p=0; p<getNumParameters(); p++){
        String name;
        for (String::CharPointerType t (UI_CONTROLS[p].name.getCharPointer()); ! t.isEmpty(); ++t){
            if(t.isLetterOrDigit() || *t == '_' || *t == '-' || *t == ':'){
                name += *t;
            }
        }
        xml.setAttribute(name, getParameter(p));
    }

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            lastUIWidth  = xmlState->getIntAttribute ("uiWidth", lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute ("uiHeight", lastUIHeight);

            for(int p=0; p<getNumParameters(); p++){
                String name;
                for (String::CharPointerType t (UI_CONTROLS[p].name.getCharPointer()); ! t.isEmpty(); ++t){
                    if(t.isLetterOrDigit() || *t == '_' || *t == '-' || *t == ':'){
                        name += *t;
                    }
                }
                setParameter(p, (float) xmlState->getDoubleAttribute (name, getParameter(p)));
            }
        }
    }
}

const String PluginAudioProcessor::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginAudioProcessor::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool PluginAudioProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginAudioProcessor::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
