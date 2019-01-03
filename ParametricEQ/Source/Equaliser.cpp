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
const float PLUGIN_INIT_FREQ = 500.0f;

const float PLUGIN_Q_MIN = 0.10f;
const float PLUGIN_Q_MAX = 10.0f;
const float PLUGIN_Q_INIT = 1.0f;

const float PLUGIN_GAIN_MIN = -12.0f;
const float PLUGIN_GAIN_MAX = 12.0f;
const float PLUGIN_GAIN_INIT = 0.0f;

enum
{
    PLUGIN_PARAM_FREQ = 0,
    PLUGIN_PARAM_Q,
    PLUGIN_PARAM_GAIN,
    PLUGIN_PARAM_TYPE,
    NUM_PARAMS
};

enum FILTERTYPE
{
    FILTER_TYPE_LOWPASS = 0,
    FILTER_TYPE_HIGHPASS,
    FILTER_TYPE_BANDPASS,
    FILTER_TYPE_NOTCH,
    FILTER_TYPE_ALLPASS,
    FILTER_TYPE_PEAKING,
    FILTER_TYPE_HIGHSHELF,
    FILTER_TYPE_LOWSHELF,
    NUM_TYPES
};

char const* FILTERTYPE_NAMES[NUM_TYPES] = {"Lowpass", "Highpass", "Bandpass", "Notch", "Allpass", "Peaking", "Highshelf", "LowShelf"};

static FMOD_DSP_PARAMETER_DESC p_freq;
static FMOD_DSP_PARAMETER_DESC p_q;
static FMOD_DSP_PARAMETER_DESC p_gain;
static FMOD_DSP_PARAMETER_DESC p_type;

FMOD_DSP_PARAMETER_DESC* PluginsParameters[NUM_PARAMS] =
{
    &p_freq,
    &p_q,
    &p_gain,
    &p_type
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Kelly Biquad EQ",          // name
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
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_freq, "Frequency", "Hz", "Frequency the EQ is affecting", PLUGIN_MIN_CUTOFF, PLUGIN_MAX_CUTOFF, PLUGIN_INIT_FREQ);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_q, "Q", "", "Width of EQ", PLUGIN_Q_MIN, PLUGIN_Q_MAX, PLUGIN_Q_INIT);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_gain, "Gain", "dB", "Gain of peak / notch", PLUGIN_GAIN_MIN, PLUGIN_GAIN_MAX, PLUGIN_GAIN_INIT);
        FMOD_DSP_INIT_PARAMDESC_INT(p_type, "Type", "", "Type of filter", 0, NUM_TYPES - 1, FILTER_TYPE_PEAKING, false, FILTERTYPE_NAMES);  // min is 0 because the first type is 0
        // setting to 1 skips the first filter type, and makes the last type NUM_TYPES
        
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //

const int PLUGIN_SAMPLE_DEPTH = 1;

typedef std::vector<float> DelayBuffer;

class Plugin
{
public:
    Plugin();
    void Init (FMOD_DSP_STATE* state);
    void Release ();
    
    void CreateBuffers (int numChannels);
    
    void CalculateCoefficients();
    
    void Read (float* inbuffer, float* outbuffer, unsigned int length, int channels);
    
    void SetFreq (float value) { m_frequency = value; }
    void SetQ (float value) { m_q = value; }
    void SetGain (float value) { m_gain = value; }
    void SetType (int value) { m_type = (FILTERTYPE)value; }
    
    float GetFreq () const { return m_frequency; }
    float GetQ () const { return m_q; }
    float GetGain () const { return m_gain; }
    int GetType () const { return m_type; }
    
private:
    // The three parameters are stored in their raw form
    // and are not stored as 0 - 1
    float m_frequency;
    float m_q;
    float m_gain;
    
    FILTERTYPE m_type;
    
    int m_sampleRate;
    int m_channels;
    int m_bufferLength;
    DelayBuffer* m_ym1, *m_ym2, *m_xm1, *m_xm2;
    
    float a0, a1, a2, b0, b1, b2;   // poles
};

Plugin::Plugin() :
a0(0),
a1(0),
a2(0),
b0(0),
b1(0),
b2(0),
m_bufferLength(0),
m_channels(0),
m_sampleRate(44100),
m_frequency(PLUGIN_INIT_FREQ),
m_q(PLUGIN_Q_INIT),
m_gain(PLUGIN_GAIN_INIT),
m_type(FILTER_TYPE_PEAKING)
{
    
}

void Plugin::Init(FMOD_DSP_STATE* state)
{
    FMOD_DSP_GETSAMPLERATE(state, &m_sampleRate);
    CalculateCoefficients();
}

void Plugin::Release()
{
    delete m_ym1;
    delete m_ym2;
    delete m_xm1;
    delete m_xm2;
}

/// Creates the buffers if they are not allocated yet or resizes them when the number of channels changes
void Plugin::CreateBuffers(int numChannels)
{
    if (numChannels != m_channels)
    {
        delete m_ym1;
        delete m_ym2;
        delete m_xm1;
        delete m_xm2;
        
        m_channels = numChannels;
        m_bufferLength = m_channels * PLUGIN_SAMPLE_DEPTH;
        m_ym1 = new DelayBuffer(m_channels);
        m_ym2 = new DelayBuffer(m_channels);
        m_xm1 = new DelayBuffer(m_channels);
        m_xm2 = new DelayBuffer(m_channels);
    }
}

void Plugin::CalculateCoefficients()
{
    float A, omega, cs, sn, alpha;
    
    A = powf(10, GetGain()/40.0f);
    omega = (2 * M_PI * GetFreq()) / m_sampleRate;
    sn = sinf(omega);
    cs = cosf(omega);
    alpha = sn / (2.0f * GetQ());
    
    float sqA = sqrtf(A);
    
    switch (m_type) {
        case FILTER_TYPE_LOWPASS:
            
            b0 = (1 - cs) / 2;
            b1 = 1 - cs;
            b2 = (1 - cs) / 2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            
            break;
            
        case FILTER_TYPE_HIGHPASS:
            
            b0 = (1 + cs) / 2;
            b1 = -(1 + cs);
            b2 = (1 + cs) / 2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            
            break;
            
        case FILTER_TYPE_BANDPASS:
            
            b0 = GetQ() * alpha;
            b1 = 0;
            b2 = -(GetQ() * alpha);
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            
            break;
        case FILTER_TYPE_NOTCH:
            
            b0 = 1;
            b1 = -2 * cs;
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            
            break;
        case FILTER_TYPE_ALLPASS:
            
            b0 = 1 - alpha;
            b1 = -2 * cs;
            b2 = 1 + alpha;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            
            break;
            
        case FILTER_TYPE_PEAKING:
            
            b0 = 1 + (alpha * A);
            b1 = -2 * cs;
            b2 = 1 - (alpha * A);
            a0 = 1 + (alpha / (float)A);
            a1 = -2 * cs;
            a2 = 1 - (alpha / (float)A);
            
            break;
            
        case FILTER_TYPE_LOWSHELF:
            
            b0 = A * ((A + 1) - (A - 1) * cs + 2 * sqA * alpha);
            b1 = 2 * A * ((A - 1) - (A + 1) * cs);
            b2 = A * ((A + 1) - (A - 1) * cs - 2 * sqA * alpha);
            a0 = (A + 1) + (A - 1) * cs + 2 * sqA * alpha;
            a1 = -2 * ((A - 1) + (A + 1) * cs);
            a2 = (A + 1) + (A - 1) * cs - 2 * sqA * alpha;
            
            break;
            
        case FILTER_TYPE_HIGHSHELF:
            
            b0 = A * ((A + 1) + (A - 1) * cs + 2 * sqA * alpha);
            b1 = -2 * A * ((A - 1) + (A + 1) * cs);
            b2 = A * ((A + 1) + (A - 1) * cs - 2 * sqA * alpha);
            a0 = (A + 1) - (A - 1) * cs + 2 * sqA * alpha;
            a1 = 2 * ((A - 1) - (A + 1) * cs);
            a2 = (A + 1) - (A - 1) * cs - 2 * sqA * alpha;
            
            break;

        case NUM_TYPES:
            break;
    }
    
    
}

void Plugin::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    /*
    float A, omega, cs, sn, alpha;
    
    A = powf(10, GetGain()/40.0f);
    omega = (2 * M_PI * GetFreq()) / m_sampleRate;
    sn = sinf(omega);
    cs = cosf(omega);
    alpha = sn / (2.0f * GetQ());
    
    b0 = 1 + (alpha * A);
    b1 = -2 * cs;
    b2 = 1 - (alpha * A);
    a0 = 1 + (alpha / (float)A);
    a1 = -2 * cs;
    a2 = 1 - (alpha / (float)A);
     */
    
    
    
    // x(n), y(n), x(n-1), x(n-2), y(n-1), y(n-2)
    // Current input, current output, input one sample back, input two samples back, output one sample back, output two samples back
    float xn, yn, xm1, xm2, ym1, ym2;
    
    // Loop through all samples
    for (unsigned int i = 0; i < length; i++)
    {
        // Loop through all channels within the sample (audio is interleaved)
        for (unsigned int n = 0; n < channels; n++)
        {
            xm1 = (*m_xm1)[n];
            xm2 = (*m_xm2)[n];
            
            ym1 = (*m_ym1)[n];
            ym2 = (*m_ym2)[n];
            
            
            xn = *inbuffer++;
            
            //yn = (b0*xn + b1*xm1 + b2*xm2 - a1*ym1 - a1*ym2) * (1 / a0);
            yn = (b0 / a0) * xn + (b1 / a0) * xm1 + (b2 / a0) * xm2 - (a1 / a0) * ym1 - (a2 / a0) * ym2;
            
            (*m_xm2)[n] = xm1;
            (*m_xm1)[n] = xn;
            
            (*m_ym2)[n] = ym1;
            (*m_ym1)[n] = yn;
            
            *outbuffer++ = yn;
        }
    }
}


// ======================= //
// CALLBACK IMPLEMENTATION //
// ======================= //

FMOD_RESULT Create_Callback                     (FMOD_DSP_STATE *dsp_state)
{
    Plugin* state = (Plugin* )FMOD_DSP_ALLOC(dsp_state, sizeof(Plugin));
    dsp_state->plugindata = state;
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }
    state->Init(dsp_state);
    return FMOD_OK;
}

FMOD_RESULT Release_Callback                    (FMOD_DSP_STATE *dsp_state)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->Release();
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
            
            state->CreateBuffers(outbufferarray[0].buffernumchannels[0]);
            
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
        case PLUGIN_PARAM_FREQ:
            state->SetFreq(value);
            break;
            
        case PLUGIN_PARAM_Q:
            state->SetQ(value);
            break;
            
        case PLUGIN_PARAM_GAIN:
            state->SetGain(value);
            break;
    }
    state->CalculateCoefficients();
    return FMOD_OK;
}

FMOD_RESULT SetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int value)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->SetType(value);
    state->CalculateCoefficients();
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
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (index) {
        case PLUGIN_PARAM_FREQ:
            *value = state->GetFreq();
            break;
            
        case PLUGIN_PARAM_Q:
            *value = state->GetQ();
            break;
            
        case PLUGIN_PARAM_GAIN:
            *value = state->GetGain();
            break;
    }
    state->CalculateCoefficients();
    return FMOD_OK;
}

FMOD_RESULT GetInt_Callback                     (FMOD_DSP_STATE *dsp_state, int index, int *value, char *valuestr)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    *value = state->GetType();
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
