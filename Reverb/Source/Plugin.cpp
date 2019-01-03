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

#include "fmod.hpp"

#include "DelayUnit.hpp"
#include "CutoffFilter.hpp"

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

enum
{
    PARAM_INPUT_DIFFUSE_1 = 0,
    PARAM_INPUT_DIFFUSE_2,
    PARAM_DECAY_DIFFUSE_1,
    PARAM_DECAY_DIFFUSE_2,
    PARAM_BANDWIDTH,
    PARAM_DECAY,
    PARAM_DRY,
    PARAM_WET,
    NUM_PARAMS
};

static FMOD_DSP_PARAMETER_DESC p_inputDiffuse1, p_inputDiffuse2, p_decayDiffuse1, p_decayDiffuse2, p_bandwidth, p_decay, p_dry, p_wet;


FMOD_DSP_PARAMETER_DESC* PluginsParameters[NUM_PARAMS] =
{
    &p_inputDiffuse1,
    &p_inputDiffuse2,
    &p_decayDiffuse1,
    &p_decayDiffuse2,
    &p_bandwidth,
    &p_decay,
    &p_dry,
    &p_wet
};


// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Kelly Reverb",             // name
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

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription ()
    {
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_inputDiffuse1, "Input Diffuse 1", "", "Amount of filterting of input to reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_inputDiffuse2, "Input Diffuse 2", "", "Amount of  of input to reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_decayDiffuse1, "Decay Diffuse 1", "", "Amount of filterting  input to reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_decayDiffuse2, "Decay Diffuse 2", "", "Amount of filterting of input reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_bandwidth, "Bandwidth", "", "Amount of filterting of input to reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_decay, "Decay", "", "Amount of filterting of input to reverb", 0.0f, 0.999f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_dry, "Dry", "dB", "Dry volume", -80.0f, 10.0f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_wet, "Wet", "dB", "Wet volume", -80.0f, 10.0f, 0.0f);
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //


class Plugin
{
public:
    Plugin() :
    m_predelay(nullptr),
    m_inputZ(nullptr),
    m_diffuseDelay11(nullptr),
    m_diffuseDelay12(nullptr),
    m_diffuseDelay21(nullptr),
    m_diffuseDelay22(nullptr),
    m_reverbDiffuse1(nullptr),
    m_reverbDiffuse2(nullptr),
    m_reverbDelay1(nullptr),
    m_reverbDelay2(nullptr),
    m_reverbFilter1(nullptr),
    m_reverbFilter2(nullptr),
    m_reverbDiffuse3(nullptr),
    m_reverbDiffuse4(nullptr),
    m_reverbDelay3(nullptr),
    m_reverbDelay4(nullptr),
    m_bandwidth(0)
    { }
    
    ~Plugin()
    {
        delete m_predelay;
        delete m_diffuseDelay11;
        delete m_inputZ;
        delete m_diffuseDelay11;
        delete m_diffuseDelay12;
        delete m_diffuseDelay21;
        delete m_diffuseDelay22;
        
        delete m_reverbDiffuse1;
        delete m_reverbDiffuse2;
        delete m_reverbDelay1;
        delete m_reverbDelay2;
        delete m_reverbFilter1;
        delete m_reverbFilter2;
        delete m_reverbDiffuse3;
        delete m_reverbDiffuse4;
        delete m_reverbDelay3;
        delete m_reverbDelay4;
    }
    
    /// Start the plugin and load resources
    void Init (FMOD_DSP_STATE*);
    /// Release resources
    void Release ();
    /// Called when the event is restarted
    void Reset(FMOD_DSP_STATE*);
    /// Called before read to set up memory that needs to know the number of channels
    void Query(FMOD_DSP_STATE*, int);
    /// Main DSP processing
    void Read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    /// Set parameter floats
    void SetParameterFloat(int index, float value);
    /// Get paramter floats
    void GetParameterFloat(int index, float* value);
    
private:
    // Input
    DelayUnit* m_predelay;
    DelayUnit* m_inputZ;
    // Diffuse
    DelayUnit* m_diffuseDelay11;
    DelayUnit* m_diffuseDelay12;
    DelayUnit* m_diffuseDelay21;
    DelayUnit* m_diffuseDelay22;
    // Reverb
    DelayUnit* m_reverbDiffuse1;
    DelayUnit* m_reverbDiffuse2;
    DelayUnit* m_reverbDelay1;
    DelayUnit* m_reverbDelay2;
    DelayUnit* m_reverbFilter1;
    DelayUnit* m_reverbFilter2;
    DelayUnit* m_reverbDiffuse3;
    DelayUnit* m_reverbDiffuse4;
    DelayUnit* m_reverbDelay3;
    DelayUnit* m_reverbDelay4;
    
    // Parameters
    float m_bandwidth;
    float m_decay, m_damping;
    float m_inputDiffuse1, m_inputDiffuse2;
    float m_decayDiffuse1, m_decayDiffuse2;
    float m_dry, m_wet;
    
};

void Plugin::Init(FMOD_DSP_STATE* dsp_state)
{
    m_bandwidth = 0.5;
    
    delete m_predelay;
    delete m_inputZ;
    delete m_diffuseDelay11;
    delete m_diffuseDelay12;
    delete m_diffuseDelay21;
    delete m_diffuseDelay22;
    
    delete m_reverbDiffuse1;
    delete m_reverbDiffuse2;
    delete m_reverbDelay1;
    delete m_reverbDelay2;
    delete m_reverbFilter1;
    delete m_reverbFilter2;
    delete m_reverbDiffuse3;
    delete m_reverbDiffuse4;
    delete m_reverbDelay3;
    delete m_reverbDelay4;
    
    m_predelay = new DelayUnit();
    m_inputZ = new DelayUnit();
    m_diffuseDelay11 = new DelayUnit();
    m_diffuseDelay12 = new DelayUnit();
    m_diffuseDelay21 = new DelayUnit();
    m_diffuseDelay22 = new DelayUnit();
    
    m_reverbDiffuse1 = new DelayUnit();
    m_reverbDiffuse2 = new DelayUnit();
    m_reverbDelay1 = new DelayUnit();
    m_reverbDelay2 = new DelayUnit();
    m_reverbFilter1 = new DelayUnit();
    m_reverbFilter2 = new DelayUnit();
    m_reverbDiffuse3 = new DelayUnit();
    m_reverbDiffuse4 = new DelayUnit();
    m_reverbDelay3 = new DelayUnit();
    m_reverbDelay4 = new DelayUnit();
    
    // Predelay
    m_predelay->Init(dsp_state, 20.0f);
    m_predelay->SetDelayTime(10.0f);
    m_predelay->SetFeedback(0.0f);
    
    // Input filter
    m_inputZ->Init(dsp_state, 1);
    m_inputZ->SetFeedback(0.0f);
    
    // 4 diffuse delays
    m_diffuseDelay11->Init(dsp_state, 143);
    m_diffuseDelay11->SetFeedback(0);
    
    m_diffuseDelay12->Init(dsp_state, 108);
    m_diffuseDelay12->SetFeedback(0);
    
    m_diffuseDelay21->Init(dsp_state, 380);
    m_diffuseDelay21->SetFeedback(0);
    
    m_diffuseDelay22->Init(dsp_state, 278);
    m_diffuseDelay22->SetFeedback(0);
    
    // Reverb diffuse
    
    m_reverbDiffuse1->Init(dsp_state, 673);
    m_reverbDiffuse1->SetFeedback(0.0f);
    
    m_reverbDiffuse2->Init(dsp_state, 909);
    m_reverbDiffuse2->SetFeedback(0.0f);
    
    // Reverb delays
    
    m_reverbDelay1->Init(dsp_state, 909);
    m_reverbDelay1->SetFeedback(0.0f);
    
    m_reverbDelay2->Init(dsp_state, 4454);
    m_reverbDelay2->SetFeedback(0.0f);
    
    // Reverb filters
    
    m_reverbFilter1->Init(dsp_state, 1);
    m_reverbFilter1->SetFeedback(0.0f);
    
    m_reverbFilter2->Init(dsp_state, 1);
    m_reverbFilter2->SetFeedback(0.0f);
    
    // Reverb diffuse 2
    
    m_reverbDiffuse3->Init(dsp_state, 1801);
    m_reverbDiffuse3->SetFeedback(0.0f);
    
    m_reverbDiffuse4->Init(dsp_state, 2657);
    m_reverbDiffuse4->SetFeedback(0.0f);
    
    // Reverb delays 2
    
    m_reverbDelay3->Init(dsp_state, 3721);
    m_reverbDelay3->SetFeedback(0.0f);
    
    m_reverbDelay4->Init(dsp_state, 3164);
    m_reverbDelay4->SetFeedback(0.0f);
}

void Plugin::Release()
{
    delete m_predelay;
    delete m_inputZ;
    delete m_diffuseDelay11;
    delete m_diffuseDelay12;
    delete m_diffuseDelay21;
    delete m_diffuseDelay22;
    
    delete m_reverbDiffuse1;
    delete m_reverbDiffuse2;
    delete m_reverbDelay1;
    delete m_reverbDelay2;
    delete m_reverbFilter1;
    delete m_reverbFilter2;
    delete m_reverbDiffuse3;
    delete m_reverbDiffuse4;
    delete m_reverbDelay3;
    delete m_reverbDelay4;
}

void Plugin::Reset(FMOD_DSP_STATE* dsp_state)
{

}

void Plugin::Query(FMOD_DSP_STATE* dsp_state, int channels)
{

    m_predelay->CreateBuffers(channels);
    m_inputZ->CreateBuffers(channels);
    m_diffuseDelay11->CreateBuffers(channels);
    m_diffuseDelay12->CreateBuffers(channels);
    m_diffuseDelay21->CreateBuffers(channels);
    m_diffuseDelay22->CreateBuffers(channels);
    
    m_reverbDiffuse1->CreateBuffers(channels);
    m_reverbDiffuse2->CreateBuffers(channels);
    m_reverbDiffuse3->CreateBuffers(channels);
    m_reverbDiffuse4->CreateBuffers(channels);
    
    m_reverbDelay1->CreateBuffers(channels);
    m_reverbDelay2->CreateBuffers(channels);
    m_reverbDelay3->CreateBuffers(channels);
    m_reverbDelay4->CreateBuffers(channels);
    
    m_reverbFilter1->CreateBuffers(channels);
    m_reverbFilter2->CreateBuffers(channels);
        
    
}

void Plugin::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{

    float in(0), out(0), delayedIn(0), reverbSample(0);
    
    
    for (int i = 0; i < length; i++)
    {
        for (int n = 0; n < channels; n++)
        {
            in = *inbuffer;
            out = *outbuffer;
            
            // Predelay
            delayedIn = m_predelay->GetDelayedSample() * m_bandwidth;   // Multiply bandwidth before filter
            m_predelay->WriteDelay(in * 0.5f);  // Half whatever goes into the predelay
            m_predelay->TickChannel();
            
            // Filter predelay before diffusion
            float outZ = (m_inputZ->GetDelayedSampleAt(1) * (1 - m_bandwidth)) + delayedIn;
            m_inputZ->WriteDelay(outZ);
            m_inputZ->TickChannel();
            
            // DIFFUSION
            
            // 1
            float outDiffuse1 = m_diffuseDelay11->GetDelayedSampleAt(142);
            float diffuse1Top = (-outDiffuse1 * m_inputDiffuse1) + outZ;
            float diffuse1Bottom = outDiffuse1 + (diffuse1Top * m_inputDiffuse1);
            m_diffuseDelay11->WriteDelay(diffuse1Top);
            m_diffuseDelay11->TickChannel();
            
            // 2
            float outDiffuse2 = m_diffuseDelay12->GetDelayedSampleAt(107);
            float diffuse2Top = (-outDiffuse2 * m_inputDiffuse1) + diffuse1Bottom;
            float diffuse2Bottom = outDiffuse2 + (diffuse2Top * m_inputDiffuse1);
            m_diffuseDelay12->WriteDelay(diffuse2Top);
            m_diffuseDelay12->TickChannel();
            
            // 3
            float outDiffuse3 = m_diffuseDelay21->GetDelayedSampleAt(379);
            float diffuse3Top = (-outDiffuse3 * m_inputDiffuse2) + diffuse2Bottom;
            float diffuse3Bottom = outDiffuse3 + (diffuse3Top * m_inputDiffuse2);
            m_diffuseDelay21->WriteDelay(diffuse3Top);
            m_diffuseDelay21->TickChannel();
            
            // 4
            float outDiffuse4 = m_diffuseDelay22->GetDelayedSampleAt(277);
            float diffuse4Top = (-outDiffuse4 * m_inputDiffuse2) + diffuse3Bottom;
            float diffuse4Bottom = outDiffuse4 + (diffuse4Top * m_inputDiffuse2);
            m_diffuseDelay22->WriteDelay(diffuse4Top);
            m_diffuseDelay22->TickChannel();
            
            // REVERB
            
            reverbSample = diffuse4Bottom + (m_reverbDelay4->GetDelayedSampleAt(3163) * m_decay);
            
            // diffuse 1
            float reverbDiffuse1 = m_reverbDiffuse1->GetDelayedSampleAt(672);
            float reverb1Top = (reverbDiffuse1 * m_decayDiffuse1) + reverbSample;
            float reverb1Bottom = (-reverb1Top * m_decayDiffuse1) + reverbDiffuse1;
            m_reverbDiffuse1->WriteDelay(reverb1Top);
            m_reverbDiffuse1->TickChannel();
            
            // reverb delay 1
            float reverbDelay1 = m_reverbDelay1->GetDelayedSampleAt(4453) * (1 - m_damping);
            m_reverbDelay1->WriteDelay(reverb1Bottom);
            m_reverbDelay1->TickChannel();
            
            // reverb filter 1
            float outZ1 = ((m_reverbFilter1->GetDelayedSampleAt(1) * m_damping) + reverbDelay1) * m_decay;
            
            // diffuse 3 (second diffuse on left side)
            float reverbDiffuse3 = m_reverbDiffuse3->GetDelayedSampleAt(1800);
            float reverb3Top = (-reverbDiffuse3 * m_decayDiffuse2) + outZ1;
            float reverb3Bottom = reverbDiffuse3 + (reverb3Top * m_decayDiffuse2);
            m_reverbDiffuse3->WriteDelay(reverb3Top);
            m_reverbDiffuse3->TickChannel();
            
            // reverb delay 3
            float reverbDelay3 = m_reverbDelay3->GetDelayedSampleAt(3720);
            m_reverbDelay3->WriteDelay(reverb3Bottom);
            m_reverbDelay3->TickChannel();
            
            // OTHER SIDE
            
            // diffuse 2
            reverbSample = (reverbDelay3 * m_decay) + diffuse4Bottom;
            
            float reverbDiffuse2 = m_reverbDiffuse1->GetDelayedSampleAt(908);
            float reverb2Top = (reverbDiffuse2 * m_decayDiffuse1) + reverbSample;
            float reverb2Bottom = (-reverb2Top * m_decayDiffuse1) + reverbDiffuse2;
            m_reverbDiffuse2->WriteDelay(reverb2Top);
            m_reverbDiffuse2->TickChannel();
            
            // reverb delay 2
            float reverbDelay2 = m_reverbDelay2->GetDelayedSampleAt(4217) * (1 - m_damping);
            m_reverbDelay2->WriteDelay(reverb2Bottom);
            m_reverbDelay2->TickChannel();
            
            // filter 2
            float outZ2 = ((m_reverbFilter2->GetDelayedSampleAt(1) * m_damping) + reverbDelay2) * m_decay;
            
            // diffuse 4
            float reverbDiffuse4 = m_reverbDiffuse4->GetDelayedSampleAt(2656);
            float reverb4Top = (-reverbDiffuse4 * m_decayDiffuse2) + outZ2;
            float reverb4Bottom = reverbDiffuse4 + (reverb4Top * m_decayDiffuse2);
            m_reverbDiffuse4->WriteDelay(reverb4Top);
            m_reverbDiffuse4->TickChannel();
            
            // reverb delay 4
            m_reverbDelay4->WriteDelay(reverb4Bottom);
            m_reverbDelay4->TickChannel();
            
            *outbuffer = (in * m_dry) + (reverb4Bottom * m_wet);
            
            inbuffer++;
            outbuffer++;
        }
    }
        
        //m_delay->Read(inbuffer, outbuffer, length, channels);
        //m_cutoff->Read(outbuffer, outbuffer, length, channels);

}

void Plugin::SetParameterFloat(int index, float value)
{
    switch (index) {
        case PARAM_INPUT_DIFFUSE_1:
            m_inputDiffuse1 = value;
            break;
            
        case PARAM_INPUT_DIFFUSE_2:
            m_inputDiffuse2 = value;
            break;
            
        case PARAM_DECAY_DIFFUSE_1:
            m_decayDiffuse1 = value;
            break;
            
        case PARAM_DECAY_DIFFUSE_2:
            m_decayDiffuse2 = value;
            break;
            
        case PARAM_BANDWIDTH:
            m_bandwidth = value;
            break;
            
        case PARAM_DECAY:
            m_decay = value;
            break;
            
        case PARAM_DRY:
            m_dry = DECIBELS_TO_LINEAR(value);
            break;
            
        case PARAM_WET:
            m_wet = DECIBELS_TO_LINEAR(value);
            break;
            
        default:
            break;
    }
}

void Plugin::GetParameterFloat(int index, float *value)
{
    switch (index) {
        case PARAM_INPUT_DIFFUSE_1:
            *value = m_inputDiffuse1;
            break;
            
        case PARAM_INPUT_DIFFUSE_2:
            *value = m_inputDiffuse2;
            break;
            
        case PARAM_DECAY_DIFFUSE_1:
            *value = m_decayDiffuse1;
            break;
            
        case PARAM_DECAY_DIFFUSE_2:
            *value = m_decayDiffuse2;
            break;
            
        case PARAM_BANDWIDTH:
            *value = m_bandwidth;
            break;
            
        case PARAM_DECAY:
            *value = m_decay;
            break;
            
        case PARAM_DRY:
            *value = LINEAR_TO_DECIBELS(m_dry);
            break;
            
        case PARAM_WET:
            *value = LINEAR_TO_DECIBELS(m_wet);
            break;
            
        default:
            break;
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
            state->Query(dsp_state, outbufferarray[0].buffernumchannels[0]);
            
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
    state->SetParameterFloat(index, value);
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
    Plugin* state = (Plugin* )dsp_state->plugindata;
    state->GetParameterFloat(index, value);
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
