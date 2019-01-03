//
//  DelayUnit.cpp
//  DelayPlugin
//
//  Created by James Kelly on 14/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//
//  This file is a 'blank' plugin that does not process audio and only lets it pass through

#include <math.h>
#include <stdio.h>
#include <string>

#include "fmod.hpp"

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

// ==================== //
// CALLBACK DEFINITIONS //
// ==================== //
FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Release_Callback                    (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Reset_Callback                      (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT Read_Callback                       (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
FMOD_RESULT Process_Callback                    (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT SetPosition_Callback                (FMOD_DSP_STATE *dsp_state, unsigned int pos);
FMOD_RESULT ShouldIProcess_Callback             (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);

FMOD_RESULT SetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float value);
FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value);
FMOD_RESULT SetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT SetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length);
FMOD_RESULT GetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr);
FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr);
FMOD_RESULT GetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr);
FMOD_RESULT GetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void **data, unsigned int *length, char *valuestr);

FMOD_RESULT SystemRegister_Callback             (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT SystemDeregister_Callback           (FMOD_DSP_STATE *dsp_state);
FMOD_RESULT SystemMix_Callback                  (FMOD_DSP_STATE *dsp_state, int stage);

// ==================== //
//      PARAMETERS      //
// ==================== //

// these plugins are stand-ins
// if you remove these parameters -and have an empty description- remove it from the DSP_DESCRIPTION struct
// otherwise, fmod will crash
static FMOD_DSP_PARAMETER_DESC parameter;

FMOD_DSP_PARAMETER_DESC* PluginsParameters[1] =
{
    &parameter
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Basic Plugin",             // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Create_Callback,            // create
    Release_Callback,           // release
    Reset_Callback,             // reset
    Read_Callback,              // read
    Process_Callback,           // process
    SetPosition_Callback,       // setposition
    1,                          // no. parameter
    PluginsParameters,          // pointer to parameter descriptions
    SetFloat_Callback,          // Set float
    SetInt_Callback,            // Set int
    SetBool_Callback,           // Set bool
    SetData_Callback,           // Set data
    GetFloat_Callback,          // Get float
    GetInt_Callback,            // Get int
    GetBool_Callback,           // Get bool
    GetData_Callback,           // Get data
    ShouldIProcess_Callback,    // Check states before processing
    0,                          // User data
    SystemRegister_Callback,    // System register
    SystemDeregister_Callback,  // System deregister
    SystemMix_Callback          // Mixer thread exucute / after execute
};

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription ()
    {
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //

class Plugin
{
public:
    Plugin();
    // Add methods for specific plugin use
private:
    // Add member variables as needed for the plugin
};

Plugin::Plugin()
{
    // plugin initialisation code goes here...
}


// ======================= //
// CALLBACK IMPLEMENTATION //
// ======================= //

FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state)
{
    // create our plugin class and attach to fmod
    dsp_state->plugindata = (Plugin* )FMOD_DSP_ALLOC(dsp_state, sizeof(Plugin));
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    return FMOD_OK;
}

FMOD_RESULT Release_Callback                    (FMOD_DSP_STATE *dsp_state)
{
    // release our plugin class
    Plugin* state = (Plugin* )dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);
    
    return FMOD_OK;
}

FMOD_RESULT Reset_Callback                      (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT Read_Callback                       (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
    // This method is not called if ShouldIProcess_Callback returns FMOD_ERR_DSP_DONTPROCESS or FMOD_ERR_DSP_SILENCE
    
    /*
    unsigned int samples = length * inchannels; // get total number of samples as length would only be only channel
    while (samples--) {
        *outbuffer++ = *inbuffer++;             // value at outbuffer is equal to inbuffer
    }  */                                         // then the pointer is moved to the next value
    return FMOD_OK;
}

FMOD_RESULT Process_Callback                    (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    switch (op) {
        case FMOD_DSP_PROCESS_QUERY:
            return FMOD_ERR_DSP_DONTPROCESS;
            break;
            
        case FMOD_DSP_PROCESS_PERFORM:
            // We shouldn't get to this part of the code due to returning FMOD_ERR_DSP_SILENCE earlier
            // however, if we do get here, we need to fill the output buffer
            //*outbufferarray[0].buffers[0]++ = *inbufferarray[0].buffers[0]++;
            return FMOD_OK;
            break;
    }
    
    return FMOD_OK;
}

FMOD_RESULT SetPosition_Callback                (FMOD_DSP_STATE *dsp_state, unsigned int pos)
{
    return FMOD_OK;
}

FMOD_RESULT ShouldIProcess_Callback             (FMOD_DSP_STATE *dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    return FMOD_ERR_DSP_DONTPROCESS;    // return silence so FMOD ignores this dsp until we add functionality
    return FMOD_OK;
}

FMOD_RESULT SetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float value)
{
    return FMOD_OK;
}

FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value)
{
    return FMOD_OK;
}

FMOD_RESULT SetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
    return FMOD_OK;
}

FMOD_RESULT SetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length)
{
    return FMOD_OK;
}

FMOD_RESULT GetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void **data, unsigned int *length, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT SystemRegister_Callback             (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemDeregister_Callback           (FMOD_DSP_STATE *dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemMix_Callback                  (FMOD_DSP_STATE *dsp_state, int stage)
{
    return FMOD_OK;
}
