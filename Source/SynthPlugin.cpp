//
//  SynthPlugin.cpp
//  TestSynthAU
//
//  Software Synthesizer template project for UWE AMT/CMT/BAMT students.
//

#include "SynthPlugin.h"

////////////////////////////////////////////////////////////////////////////
// SYNTH - represents the whole synthesiser
////////////////////////////////////////////////////////////////////////////

// Called to create the synthesizer (use to point JUCE to your synthesizer)
Synth* JUCE_CALLTYPE createSynth() {
    return new MySynth();
}

// Called when the synthesiser is first created
void MySynth::initialise()
{
    // Initialise synthesiser variables here
    wavetable.openResource("Sine.wav");
    wavetable.setBaseFrequency(1);           // Sine.wav contains a 1Hz sine wave
    
}

// Used to apply any additional audio processing to the synthesisers' combined output
// (when called, outputBuffer contains all the voices' audio)
void MySynth::postProcess(float** outputBuffer, int numChannels, int numSamples)
{
    // Use to add global effects, etc.
    float fDry0, fDry1;
    float* pfOutBuffer0 = outputBuffer[0];
    float* pfOutBuffer1 = outputBuffer[1];
    
    filterGlobal.setCutoff(50);
    
    while(numSamples--)
    {
        fDry0 = *pfOutBuffer0;
        fDry1 = *pfOutBuffer1;
        
        // Add your global effect processing here
        fDry0 = filterGlobal.tick(fDry0);
        fDry1 = filterGlobal.tick(fDry1);

        *pfOutBuffer0++ = fDry0;
        *pfOutBuffer1++ = fDry1;
    }
}

////////////////////////////////////////////////////////////////////////////
// VOICE - represents a single note in the synthesiser
////////////////////////////////////////////////////////////////////////////

// Called to create the voice (use to point JUCE to your voice)
Voice* JUCE_CALLTYPE createVoice() {
    return new MyVoice();
}

// Triggered when a note is started (use to initialise / prepare note)
void MyVoice::onStartNote (const int pitch, const float velocity)
{
    fCarrierFrequency = MidiMessage::getMidiNoteInHertz (pitch);
    
    // retreving envelope parameters
    float fAttack = getParameter(kParam10);
    float fDecay = getParameter(kParam11);
    float fSustain = getParameter(kParam12);
    float fPan = getParameter(kParam13);
    
    // resetter
    carrier1.reset();
    carrier2.reset();
    modulator1.reset();
    modulator2.reset();
    modulator3.reset();
    LFO.reset();
    
    
    // Envelpoe Setups
    ampEnv.set(Envelope::Points(0.0,0.0)(fAttack,1.0)(fAttack + fDecay,fSustain));
    ampEnv.setLoop(2,2);
    pan1.set(Envelope::Points(0.0,1.0)(1.0,fPan));
    pan2.set(Envelope::Points(0.0,0.0)(1.0,1-fPan));
    
    // "initialiser"
    carrier1 = *getSynthesiser()->getWavetable();
    carrier2.setFrequency(fCarrierFrequency * 0.5);
    fLevel = velocity;
}

// Triggered when a note is stopped (return false to keep the note alive)
bool MyVoice::onStopNote ()
{
    float fRelease = getParameter(kParam9) + 0.01;
    ampEnv.release(fRelease);
    return false;
}

// Called to render the note's next buffer of audio (generates the sound)
// (return false to terminate the note)
bool MyVoice::process (float** outputBuffer, int numChannels, int numSamples)
{
    float fMix;
    float* pfOutBuffer0 = outputBuffer[0];
    float* pfOutBuffer1 = outputBuffer[1];
    
    // declaration of local variables and their assignments + scaling
    float fModFrequency = fCarrierFrequency * (getParameter(kParam2));
    float fModIndex = (getParameter(kParam7) * 0.5) + 0.5;
    float fOutGain = getParameter(kParam4);
    bool modType = getParameter(kParam1);
    float LFOrate = (getParameter(kParam0) * 19.9 + 0.1);
    float fLFOdepth = getParameter(kParam5) * 0.2;
    float fAMmodFrequency = (fCarrierFrequency * getParameter(kParam8))+20;
    const float fIfd = fModFrequency * fModIndex;
    float fMod, fLFO;
    
    // cubing + scaling of "richness" control
    fModIndex = (fModIndex * fModIndex * fModIndex) * 16;

    // setting frequencies for the while loop
    LFO.setFrequency(LFOrate);
    filter.setCutoff(getParameter(kParam3)*19000+20);
    modulator1.setFrequency(fModFrequency);
    modulator2.setFrequency(fModFrequency);
    modulator3.setFrequency(fAMmodFrequency);

    while(numSamples--)
    {
        if (modType == 1)                   // check for modulator type (saw or sine)
        {
            fMod = modulator1.tick() * fIfd;
        }else if (modType == 0)
        {
            fMod = modulator2.tick() * fIfd;
        }
        
        // calculation of LFO + its depth
        fLFO = LFO.tick() * fLFOdepth + 0.5;
        
        // setting of (fundamental) carrier frequency
        carrier1.setFrequency(fCarrierFrequency + fMod);
        
        // choosing for the AM lead to be active or not
        if (fLevel == 1.0) {
            fMix = filter.tick((((carrier1.tick()*pan1.tick())+(carrier2.tick() * pan2.tick())) * modulator3.tick() * fLevel * ampEnv.tick() * fLFO) * fOutGain);
        }else{
        fMix = filter.tick((((carrier1.tick()*pan1.tick())+(carrier2.tick() * pan2.tick())) * fLevel * ampEnv.tick() * fLFO) * fOutGain);
        }
        
        *pfOutBuffer0++ = fMix;
        *pfOutBuffer1++ = fMix;
    }
    
    return ampEnv.getStage() != Envelope::STAGE::ENV_OFF;
}
