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
#include <vector>

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

const float PLUGIN_MIN_CUTOFF = 20.0f;
const float PLUGIN_MAX_CUTOFF = 20000.0f;

enum
{
    PLUGIN_PARAM_CUTOFF = 0,
    PLUGIN_PARAM_ISHIGHPASS,
    NUM_PARAMS
};

static FMOD_DSP_PARAMETER_DESC p_cutoff;
static FMOD_DSP_PARAMETER_DESC p_isHighpass;

FMOD_DSP_PARAMETER_DESC* PluginsParameters[NUM_PARAMS] =
{
    &p_cutoff,
    &p_isHighpass
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Kelly High-Low Filter",    // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Create_Callback,            // create
    Release_Callback,           // release
    Reset_Callback,             // reset
    Read_Callback,              // read
    Process_Callback,           // process
    SetPosition_Callback,       // setposition
    NUM_PARAMS,                 // no. parameter
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
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_cutoff, "Cutoff", "Hz", "Cutoff Frequency Of Filter", PLUGIN_MIN_CUTOFF, PLUGIN_MAX_CUTOFF, PLUGIN_MAX_CUTOFF);
        FMOD_DSP_INIT_PARAMDESC_BOOL(p_isHighpass, "Highpass", "On/Off", "Wheter this is a highpass or lowpass filter", false, 0);
        
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //

typedef std::vector<float> DelayBuffer;

class Plugin
{
public:
    Plugin() :
    m_isHighpass(0) { }
    void Init (int sampleRate);
    void Release () { delete m_yBuffer; }
    void SetCutoff (float value) { m_inputFilter = value; }
    float GetCutoff () const { return m_inputFilter; }
    
    void SetHighPass (bool value) { m_isHighpass = value; }
    bool GetHighPass () const { return m_isHighpass; }
    
    void Read (float* inbuffer, float* outbuffer, unsigned int length, int channels);
    
private:
    FMOD_BOOL m_isHighpass;
    float m_inputFilter;
    int m_sampleRate;
    int m_channels;
    DelayBuffer* m_yBuffer;
    DelayBuffer* m_xBuffer;
};

void Plugin::Init(int sampleRate)
{
    m_sampleRate = sampleRate;
}

void Plugin::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    if (m_channels != channels)
    {
        delete m_yBuffer;
        delete m_xBuffer;
        
        m_channels = channels;
        m_yBuffer = new DelayBuffer(channels);
        m_xBuffer = new DelayBuffer(channels);
    }
    
    // value between 0 - 1 that describes the 'strength' of the filter
    // we take away the min so we are working from 0 upwards
    // then, when we divide, the range is properly between 0 and 1
    static float beta = (m_inputFilter - PLUGIN_MIN_CUTOFF) / (PLUGIN_MAX_CUTOFF - PLUGIN_MIN_CUTOFF);
    static float hBeta = 1 - beta;
    
    // Loop through all samples
    for (unsigned int i = 0; i < length; i++)
    {
        // recalculate beta for each sample
        beta = m_inputFilter / PLUGIN_MAX_CUTOFF;
        hBeta = 1 - beta;
        
        // Loop through all channels within the sample (audio is interleaved)
        for (unsigned int n = 0; n < channels; n++)
        {

            /*
            float previousOutput = *(outbuffer - 1);
            *outbuffer++ = previousOutput + alpha * (*inbuffer++ - previousOutput);
             */
            
            float currentInput = *inbuffer;
            float previousOutput = (*m_yBuffer)[n];
            float previousInput = (*m_xBuffer)[n];
            if (!m_isHighpass)
            {
                // y[i] := y[i-1] + α * (x[i] - y[i-1])
                *outbuffer = previousOutput + beta * (currentInput - previousOutput);
            }
            else
            {
                // y[i] := α * (y[i-1] + x[i] - x[i-1])
                *outbuffer = hBeta * (previousOutput + currentInput - previousInput);
            }
            
            // Store previous values
            (*m_xBuffer)[n] = *inbuffer++;
            (*m_yBuffer)[n] = *outbuffer++;
            
        }
    }
}


// ======================= //
// CALLBACK IMPLEMENTATION //
// ======================= //

FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state)
{
    // create our plugin class and attach to fmod
    Plugin* state = (Plugin* )FMOD_DSP_ALLOC(dsp_state, sizeof(Plugin));
    dsp_state->plugindata = state;
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    int rate(0);
    state->Init(FMOD_DSP_GETSAMPLERATE(dsp_state, &rate));
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

    return FMOD_OK;
}

FMOD_RESULT Process_Callback                    (FMOD_DSP_STATE *dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (op) {
        case FMOD_DSP_PROCESS_QUERY:
            if (outbufferarray && inbufferarray)
            {
                outbufferarray[0].bufferchannelmask[0] = inbufferarray[0].bufferchannelmask[0];
                outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
                outbufferarray[0].speakermode       = inbufferarray[0].speakermode;
                
                
                
            }
            
            if (inputsidle)
            {
                return FMOD_ERR_DSP_DONTPROCESS;
            }
            break;
            
        case FMOD_DSP_PROCESS_PERFORM:
            
            if (inputsidle)
            {
                return FMOD_ERR_DSP_DONTPROCESS;
            }
            
            state->Read(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, outbufferarray[0].buffernumchannels[0]);
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
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    
    return FMOD_OK;
}

FMOD_RESULT SetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float value)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (index) {
        case PLUGIN_PARAM_CUTOFF:
            state->SetCutoff(value);
            break;
    }
    return FMOD_OK;
}

FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value)
{
    return FMOD_OK;
}

FMOD_RESULT SetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL value)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (index) {
        case PLUGIN_PARAM_ISHIGHPASS:
            state->SetHighPass(value);
            break;
    }
    
    return FMOD_OK;
}

FMOD_RESULT SetData_Callback                    (FMOD_DSP_STATE *dsp_state, int index, void *data, unsigned int length)
{
    return FMOD_OK;
}

FMOD_RESULT GetFloat_Callback                   (FMOD_DSP_STATE *dsp_state, int index, float *value, char *valuestr)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    switch (index) {
        case PLUGIN_PARAM_CUTOFF:
            *value = state->GetCutoff();
            break;
    }
    
    return FMOD_OK;
}

FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetBool_Callback                    (FMOD_DSP_STATE *dsp_state, int index, FMOD_BOOL *value, char *valuestr)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (index) {
        case PLUGIN_PARAM_ISHIGHPASS:
            *value = state->GetHighPass();
            break;
    }
    
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
