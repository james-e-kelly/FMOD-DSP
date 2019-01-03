//
//  DelayUnit.cpp
//  DelayPlugin
//
//  Created by James Kelly on 14/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//
//  Delay plugin

#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

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

const float DELAY_PLUGIN_MIN_DELAY_TIME_MS = 1.0f;      // 1ms
const float DELAY_PLUGIN_MAX_DELAY_TIME_MS = 10000.0f;  // 10,000ms / 10 seconds
const float DELAY_PLUGIN_INIT_DELAY_TIME_MS = 5.0f;     // 5ms

// levels for both dry and wet
const float DELAY_PLUGIN_LEVELS_MIN = -80.0f;
const float DELAY_PLUGIN_LEVELS_MAX = 10.0f;
const float DELAY_PLUGIN_LEVELS_INIT = 0.0f;

const float DELAY_PLUGIN_FEEDBACK_MIN = 0.0f;
const float DELAY_PLUGIN_FEEDBACK_MAX = 99.0f;
const float DELAY_PLUGIN_FEEDBACK_INIT = 0.0f;

enum
{
    DELAY_TIME = 0,
    FEEDBACK_PERCENT,
    DRY_LEVEL,
    WET_LEVEL,
    NUM_PARAMS
};

static FMOD_DSP_PARAMETER_DESC p_delayTime;
static FMOD_DSP_PARAMETER_DESC p_feedback;
static FMOD_DSP_PARAMETER_DESC p_dry;
static FMOD_DSP_PARAMETER_DESC p_wet;

FMOD_DSP_PARAMETER_DESC* PluginsParameters[NUM_PARAMS] =
{
    &p_delayTime,
    &p_feedback,
    &p_dry,
    &p_wet
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Kelly Delay",              // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Create_Callback,            // create
    Release_Callback,           // release
    Reset_Callback,             // reset
    0,                          // read
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

#define MS_TO_SAMPLES(__ms__, __rate__) ((__ms__ * __rate__) / 1000.0f)
#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= DELAY_PLUGIN_LEVELS_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? DELAY_PLUGIN_LEVELS_MIN : 20.0f * log10f((float)__linval__))

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription ()
    {
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_delayTime, "Delay Time", "ms", "Time of delay", DELAY_PLUGIN_MIN_DELAY_TIME_MS, DELAY_PLUGIN_MAX_DELAY_TIME_MS, DELAY_PLUGIN_INIT_DELAY_TIME_MS);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_feedback, "Feedback", "%", "Amount of feedback in delay", DELAY_PLUGIN_FEEDBACK_MIN, DELAY_PLUGIN_FEEDBACK_MAX, DELAY_PLUGIN_FEEDBACK_INIT);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_dry, "Dry", "dB", "Dry amount in dB. -80 to 10. Default = 0", DELAY_PLUGIN_LEVELS_MIN, DELAY_PLUGIN_LEVELS_MAX, DELAY_PLUGIN_LEVELS_INIT);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_wet, "Wet", "dB", "Wet amount in dB. -80 to 10. Default = 0", DELAY_PLUGIN_LEVELS_MIN, DELAY_PLUGIN_LEVELS_MAX, DELAY_PLUGIN_LEVELS_INIT);
        
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
    Plugin() { }
    
    /// Start the plugin and load resources
    void Init (FMOD_DSP_STATE*);
    /// Release resources
    void Release ();
    /// Called when the event is restarted
    void Reset(FMOD_DSP_STATE*);
    
    // parameter gets and sets
    float GetDelayTime() const {return m_delayTime; }
    float GetFeedback() const {return ((m_feedbackAmount > DELAY_PLUGIN_FEEDBACK_MAX) ? DELAY_PLUGIN_FEEDBACK_MAX : (m_feedbackAmount < DELAY_PLUGIN_FEEDBACK_MIN) ? DELAY_PLUGIN_FEEDBACK_MIN : m_feedbackAmount) / 100;}
    float GetDry(bool linear = true) const {return linear ? DECIBELS_TO_LINEAR(m_dryAmount) : m_dryAmount; }
    float GetWet(bool linear = true) const {return linear ? DECIBELS_TO_LINEAR(m_wetAmount) : m_wetAmount; }
    void SetDelayTime(float);
    void SetFeedback(float);
    void SetDry(float);
    void SetWet(float);
    
    /// Advance the write position by one sample
    void TickLeft();
    
    void TickRight();
    
    /// Get the sample value at the negative time
    float GetDelayedSampleLeft();
    /// Write the sample into the delay buffer
    void WriteDelayLeft(float sample);
    
    float GetDelayedSampleRight();
    
    void WriteDelayRight(float sample);
    
    /// Main DSP processing
    void Read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    
private:
    /// Vector of samples
    DelayBuffer* m_bufferLeft;
    DelayBuffer* m_bufferRight;
    /// Which index we are writing the samples into
    int m_writePosLeft;
    int m_writePosRight;
    /// Time in ms of delay
    float m_delayTime;
    /// Amount in % for feedback. Divide by 100 to get linear value
    float m_feedbackAmount;
    /// Amount in dB for dry signal. Use GetDry to get linear value
    float m_dryAmount;
    /// Amount in dB for wet / delayed signal Use GetWet to get linear value
    float m_wetAmount;
    /// Sample rate of application
    int m_sampleRate;
    /// Maximum time of delay in samples
    int m_maxSampleDelay;
    int m_numOfChannels;
};

void Plugin::Init(FMOD_DSP_STATE* dsp_state)
{
    m_writePosLeft = 0;
    m_delayTime = DELAY_PLUGIN_INIT_DELAY_TIME_MS;
    m_feedbackAmount = DELAY_PLUGIN_FEEDBACK_INIT;
    m_dryAmount = DELAY_PLUGIN_LEVELS_INIT;
    m_wetAmount = DELAY_PLUGIN_LEVELS_INIT;
    m_numOfChannels = 2;
    Reset(dsp_state);
}

void Plugin::Release()
{
    delete m_bufferLeft;
    delete m_bufferRight;
}

void Plugin::Reset(FMOD_DSP_STATE* dsp_state)
{
    delete m_bufferLeft;
    delete m_bufferRight;
    
    dsp_state->functions->getsamplerate(dsp_state, &m_sampleRate);
    m_maxSampleDelay = MS_TO_SAMPLES(DELAY_PLUGIN_MAX_DELAY_TIME_MS, m_sampleRate);
    
    m_bufferLeft = new DelayBuffer(m_maxSampleDelay);
    m_bufferRight = new DelayBuffer(m_maxSampleDelay);
}

void Plugin::TickLeft()
{
    ++m_writePosLeft;
    
    if (m_writePosLeft >= m_maxSampleDelay)
    {
        m_writePosLeft = 0;
    }
}

void Plugin::TickRight()
{
    ++m_writePosRight;
    
    if (m_writePosRight >= m_maxSampleDelay || m_writePosRight < 0)
    {
        m_writePosRight = 0;
    }
}

void Plugin::WriteDelayLeft(float sample)
{
    if (m_bufferLeft)
    {
        (*m_bufferLeft)[m_writePosLeft] = sample;
    }
}

void Plugin::WriteDelayRight(float sample)
{
    if (m_bufferRight)
    {
        (*m_bufferRight)[m_writePosLeft] = sample;
    }
}

float Plugin::GetDelayedSampleLeft()
{
    if (m_bufferLeft)
    {
       
        
        float r, previousIndex, nextIndex;
        
        float delayInSamples = MS_TO_SAMPLES(m_delayTime, m_sampleRate);
        
        r = modf(m_writePosLeft - delayInSamples, &previousIndex);
        
        nextIndex = previousIndex + 1;
        
        while (previousIndex >= m_maxSampleDelay) previousIndex -= m_maxSampleDelay;
        while (previousIndex < 0) previousIndex += m_maxSampleDelay;
        
        while (nextIndex >= m_maxSampleDelay) nextIndex -= m_maxSampleDelay;
        while (nextIndex < 0) nextIndex += m_maxSampleDelay;
        
        float previousValue, nextValue;
        
        previousValue = (*m_bufferLeft)[(int)previousIndex];
        nextValue = (*m_bufferLeft)[(int)nextIndex];
        
        return (previousValue*(1-r)+nextValue*r);
        
    }
    
    return 0.0f;
    
}

float Plugin::GetDelayedSampleRight()
{
    if (m_bufferRight)
    {
       
        
        
      
        
            float r, previousIndex, nextIndex;
            
            float delayInSamples = MS_TO_SAMPLES(m_delayTime, m_sampleRate);
            
            r = modf((float)m_writePosLeft - delayInSamples, &previousIndex);
            
            nextIndex = previousIndex + 1;
            
            while (previousIndex >= m_maxSampleDelay) previousIndex -= m_maxSampleDelay;
            while (previousIndex < 0) previousIndex += m_maxSampleDelay;
            
            while (nextIndex >= m_maxSampleDelay) nextIndex -= m_maxSampleDelay;
            while (nextIndex < 0) nextIndex += m_maxSampleDelay;
            
            float previousValue, nextValue;
            
            previousValue = (*m_bufferRight)[(int)previousIndex];
            nextValue = (*m_bufferRight)[(int)nextIndex];
            
            return (previousValue*(1-r)+nextValue*r);
        
        /*
        int readIndex = m_writePos - samples;
        
        while (readIndex >= m_maxDelay) readIndex -= m_maxDelay;
        while (readIndex < 0) readIndex += m_maxDelay;
        
        return (*m_bufferRight)[readIndex];
        */
    }
    
    return 0.0f;
}

// for all parameter sets, we don't need to check the range as we told fmod the ranges when creating the parameters
// therefor, it is checked for us
void Plugin::SetDelayTime(float delayTime)
{
    m_delayTime = delayTime;
}

void Plugin::SetFeedback(float feedbackAmount)
{
    m_feedbackAmount = feedbackAmount;
}

void Plugin::SetDry(float dry)
{
    m_dryAmount = dry;
}

void Plugin::SetWet(float wet)
{
    m_wetAmount = wet;
}

void Plugin::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    m_numOfChannels = channels;
    
    float* inSample(inbuffer);
    float* outSample(outbuffer);
    
    for (unsigned int i = 0; i < length; i++)
    {
        for (unsigned int n = 0; n < channels; n++, inSample++, outSample++)
        {
            switch (n)
            {
                case 0:
                {
                    float drySample(*inSample), wetSample(GetDelayedSampleLeft());
                    
                    WriteDelayLeft(drySample + (wetSample * GetFeedback()));
                    
                    *outSample = (drySample * GetDry()) + ((wetSample) * GetWet());
                    
                    TickLeft();

                    break;
                }
                case 1:
                {
                    float drySample(*inSample), wetSample(GetDelayedSampleRight());
                    
                    WriteDelayRight(drySample + (wetSample * GetFeedback()));
                    
                    *outSample = (drySample * GetDry()) + ((wetSample) * GetWet());
                    
                    TickRight();

                    break;
                }
                    
                default:
                    *outSample = *inSample * GetDry();
                    break;
            }
            
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
    state->Init(dsp_state);
    dsp_state->plugindata = state;
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
    state->Release();
    FMOD_DSP_FREE(dsp_state, state);
    
    return FMOD_OK;
}

FMOD_RESULT Reset_Callback                      (FMOD_DSP_STATE *dsp_state)
{
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->Reset(dsp_state);
    return FMOD_OK;
}

FMOD_RESULT Read_Callback                       (FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
    //Plugin* state = (Plugin* )dsp_state->plugindata;
    //state->Read(inbuffer, outbuffer, length, *outchannels);
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
        case DELAY_TIME:
            state->SetDelayTime(value);
            return FMOD_OK;
            break;
            
        case FEEDBACK_PERCENT:
            state->SetFeedback(value);
            return FMOD_OK;
            break;
            
        case  DRY_LEVEL:
            state->SetDry(value);
            return FMOD_OK;
            break;
            
        case WET_LEVEL:
            state->SetWet(value);
            return FMOD_OK;
            break;

    }
    return FMOD_ERR_INVALID_PARAM;
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
    Plugin* state = (Plugin* )dsp_state->plugindata;
    
    switch (index) {
        case DELAY_TIME:
            *value = state->GetDelayTime();
            return FMOD_OK;
            break;
            
        case FEEDBACK_PERCENT:
            *value = state->GetFeedback();
            return FMOD_OK;
            break;
            
        case  DRY_LEVEL:
            *value = state->GetDry(false);
            return FMOD_OK;
            break;
            
        case WET_LEVEL:
            *value = state->GetWet(false);
            return FMOD_OK;
            break;
            
    }
    return FMOD_ERR_INVALID_PARAM;
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
