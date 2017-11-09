//
//  SynthExtra.h
//  TestSynthAU
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#define MAX_HARMONICS 8
#include "PluginWrapper.h"

class sawWave
{
public:
    
    void reset()    ////Used to set the phase position to the start of the wave.
    {
        for (int n=0; n < MAX_HARMONICS; n++)
        {
            harmonic[n].reset();
            envelopes[n].set(Envelope::Points(0,0)(0.05,1)(0,0));
        }
    }
    
    void setFrequency(float frequency)  ////Used to set the frequency of the tone.
    {
        for (int count=0; count < MAX_HARMONICS; count++)
        {
            harmonic[count].setFrequency((count+1) * frequency);
        }
    }
    
    float tick()    ////Generates the audio
    {
        float value = 0.0;
        
        for (int count=0; count < MAX_HARMONICS; count++)
        {
            value += harmonic[count].tick() * (1.0 / (count+1)); // the +1 is because it's the 1st harmonic, but 0th array item
        }
        return value;
    }
    
private:
    Sine harmonic[MAX_HARMONICS];
    Envelope envelopes[MAX_HARMONICS];
    
};
