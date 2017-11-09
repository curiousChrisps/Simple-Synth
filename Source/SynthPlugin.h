//
//  SynthPlugin.h
//  TestSynthAU
//
//  Software Synthesizer template project for UWE AMT/CMT/BAMT students.
//

#ifndef __SynthPlugin_h__
#define __SynthPlugin_h__

#include "PluginProcessor.h"
#include "SynthExtra.h"

//===================================================================================
/** An example STK-voice, based on a sine wave generator                           */
class MyVoice : public Voice
{
public:
    void onStartNote (const int pitch, const float velocity);
    bool onStopNote ();
    Envelope ampEnv;
    Envelope pan1;
    Envelope pan2;
    
    bool process (float** outputBuffer, int numChannels, int numSamples);
    
private:
    Wavetable carrier1;
    Sine carrier2;
    Sine modulator1;
    sawWave modulator2;
    Sine modulator3;
    Sine LFO;
    float fCarrierFrequency;
    
    LPF filter;

    
    float fLevel;
};

class MySynth : public Synth
{
public:
    MySynth() : Synth() {
        initialise();
    }
    
    const Wavetable* getWavetable() {return &wavetable;}
    
    void initialise();
    void postProcess(float** outputBuffer, int numChannels, int numSamples);

private:
    // Insert synthesizer variables here
    Wavetable wavetable;
    HPF filterGlobal;

};

#endif
