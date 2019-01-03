//
//  Main.cpp
//  FMOD-Plugin
//
//  Created by James Kelly on 06/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//
//  This plugin is built for 64-bit architectures
//  This file can be placed directly in a game's code and be loaded, or compiled as a DLL or DYLIB file to be loaded as a library

#include <math.h>
#include <stdio.h>
#include <string>

#include "fmod.hpp"


extern "C"
{
    // Define Main FMOD Call To Access Our Plugin
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

// All Callback Function Definitions
FMOD_RESULT Plugin_Create                    (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Plugin_Release                   (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Plugin_Reset                     (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Plugin_Read                      (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
FMOD_RESULT Plugin_Process                   (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT Plugin_ShouldIProcess            (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);
FMOD_RESULT Plugin_SetBool             (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT Plugin_GetBool             (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr);

// Our parameters in the plugin
static FMOD_DSP_PARAMETER_DESC mute;    // When true, mutes all audio input

FMOD_DSP_PARAMETER_DESC* Silence_DSP_Param[1] =
{
    &mute
};

// Our main definition
// Sets callbacks and info like name
FMOD_DSP_DESCRIPTION Silence_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Kelly Silence",            // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Plugin_Create,              // create
    Plugin_Release,             // release
    Plugin_Reset,               // reset
    0,                          // read
    Plugin_Process,             // process
    0,                          // setposition
    1,                          // no. parameter
    Silence_DSP_Param,          // pointer to parameter descriptions
    0,                          // Set float
    0,                          // Set int
    Plugin_SetBool,             // Set bool
    0,                          // Set data
    0,                          // Get float
    0,                          // Get int
    Plugin_GetBool,             // Get bool
    0,                          // Get data
    Plugin_ShouldIProcess,      // Check states before processing
    0,                          // User data
    0,                          // System register
    0,                          // System deregister
    0                           // Mixer thread exucute / after execute
};

extern "C"
{
    
// Implementation of GetDescription
// Hands our struct from above
// and sets our parameters up
F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription ()
{
    
    FMOD_DSP_INIT_PARAMDESC_BOOL(mute, "Mute", "", "Whether this plugin lets through audio or not", false, 0);
    return &Silence_Desc;
}
    
}

class TOmSSilenceState
{
public:
    void SetMute(bool);
    bool GetMute() const { return m_mute; }
    
private:
    bool m_mute;
};

void TOmSSilenceState::SetMute(bool value)
{
    m_mute = value;
};


FMOD_RESULT Plugin_Create                    (FMOD_DSP_STATE *dsp_state)
{
    dsp_state->plugindata = (TOmSSilenceState* )FMOD_DSP_ALLOC(dsp_state, sizeof(TOmSSilenceState));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    
    return FMOD_OK;
}

FMOD_RESULT Plugin_Release                   (FMOD_DSP_STATE *dsp_state)
{
    TOmSSilenceState* state = (TOmSSilenceState* )dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;
}

FMOD_RESULT Plugin_Reset                     (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK Plugin_Read(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
    //*outbuffer = *inbuffer;
    return FMOD_OK;
}

FMOD_RESULT Plugin_Process                   (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    switch (op) {
        case FMOD_DSP_PROCESS_QUERY:
            
            if (outbufferarray && inbufferarray)
            {
                outbufferarray[0].bufferchannelmask[0] = inbufferarray[0].bufferchannelmask[0];
                outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
                outbufferarray[0].speakermode          = inbufferarray[0].speakermode;
            }
            
            if (inputsidle)
            {
                return FMOD_ERR_DSP_DONTPROCESS;
            }
            break;
            
        case FMOD_DSP_PROCESS_PERFORM:
            
            TOmSSilenceState* state = (TOmSSilenceState* )dsp_state->plugindata;
            unsigned int samples = length * inbufferarray[0].buffernumchannels[0];
            
            while (samples--)
            {
                if (state->GetMute())
                {
                    *outbufferarray[0].buffers[0]++ = 0;
                }
                else
                {
                    *outbufferarray[0].buffers[0]++ = *inbufferarray[0].buffers[0]++;
                }
                
            }
            
            break;
    }
    
    return FMOD_OK;
}

FMOD_RESULT Plugin_ShouldIProcess            (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    if (inputsidle)
        return FMOD_ERR_DSP_DONTPROCESS;
    return FMOD_OK;
}

FMOD_RESULT Plugin_SetBool             (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
    TOmSSilenceState* state = (TOmSSilenceState* )dsp_state->plugindata;
    state->SetMute(value);
    return FMOD_OK;
}

FMOD_RESULT Plugin_GetBool             (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
    TOmSSilenceState* state = (TOmSSilenceState* )dsp_state->plugindata;
    *value = state->GetMute();
    return FMOD_OK;
}






