//
//  SynthEditor.h
//  TestSynthAU
//
//  Used to specify the contents and layout of the TestSynthAU User Interface (UI).
//

#ifndef __SynthEditor_h__
#define __SynthEditor_h__

enum CONTROL_TYPE
{
    ROTARY, // rotary knob (pot)
    BUTTON, // push button (trigger)
    TOGGLE, // on/off switch (toggle)
    SLIDER, // linear slider (fader)
    MENU,   // drop-down list (menu)
};

typedef Rectangle<int> Bounds;

struct Control
{
    String name;            // name for control label / saved parameter
    int parameter;          // parameter index associated with control
    CONTROL_TYPE type;      // control type (see above)
    
    // ROTARY and SLIDER only:
    float min;              // minimum slider value (e.g. 0.0)
    float max;              // maximum slider value (e.g. 1.0)
    
    float initial;          // initial value for slider (e.g. 0.0)
    
    Bounds size;            // position (x,y) and size (height, width) of the control (use AUTO_SIZE for automatic layout)
    
    const char* const options[8]; // text options for menus and group buttons
};

const Bounds AUTO_SIZE = Bounds(-1,-1,-1,-1); // used to trigger automatic layout
enum { kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, kParam8, kParam9, kParam10, kParam11, kParam12, kParam13 };

//=========================================================================
// UI_CONTROLS - Use this array to completely specify your UI
// - tell the system what controls you want
// - add or remove controls by adding or taking away from the list
// - each control is linked to the specified parameter name and identifier
// - controls can be of different types - rotary, button, toggle, slider (see above)
// - for rotary and linear sliders, you can set the range of values
// - by default, the controls are laid out in a grid, but you can also move and size them manually
//   i.e. replace AUTO_SIZE with Bounds(50,50,100,100) to place a 100x100 control at (50,50)

const Control UI_CONTROLS[] = {
//      name,       parameter,  type,   min, max, initial,size          // options (for MENU)
    {   "LFO Rate",  kParam0,    ROTARY, 0.0, 1.0, 0.25,    AUTO_SIZE   },
    {   "Mod Type",  kParam1,    TOGGLE, 0.0, 1.0, 0.0,    AUTO_SIZE   },
    {   "Tuning",  kParam2,    ROTARY, 0.0, 1.0, 0.5,    AUTO_SIZE   },
    {   "LPFilter",  kParam3,    ROTARY, 0.0, 1.0, 0.75,    AUTO_SIZE   },
    {   "Out Gain",  kParam4,    ROTARY, 0.0, 1.0, 0.5,    AUTO_SIZE   },
    

    {   "LFO Depth",    kParam5,    ROTARY, 0.0, 1.0, 0.0,    AUTO_SIZE   },  // ************ ASSIGNMENT RESTRICTED *************
    {   "-----",    kParam6,    ROTARY, 0.0, 1.0, 0.0,    AUTO_SIZE   },  // These 5 parameters are controlled by the music.
    {   "Richness",    kParam7,    ROTARY, 0.0, 1.0, 0.0,    AUTO_SIZE   },  // You are allowed to change the name and size of a
    {   "AM Mod",    kParam8,    ROTARY, 0.0, 1.0, 0.0,    AUTO_SIZE   },  // control, but MUST NOT change any other settings.
    {   "Release",    kParam9,    ROTARY, 0.0, 1.0, 0.0,    AUTO_SIZE   },  // ************************************************
    
    {   "Attack",  kParam10,    SLIDER, 0.0, 1.0, 0.01,    AUTO_SIZE   },
    {   "Decay",  kParam11,    SLIDER, 0.0, 1.0, 0.4,    AUTO_SIZE   },
    {   "Sustain",  kParam12,    SLIDER, 0.0, 1.0, 0.6,    AUTO_SIZE   },
    {   "Dry/Wet",  kParam13,    ROTARY, 0.0, 1.0, 0.5,    AUTO_SIZE   },

};

const int kNumberOfControls = sizeof(UI_CONTROLS) / sizeof(Control);
const int kNumberOfParameters = kNumberOfControls;

#endif
