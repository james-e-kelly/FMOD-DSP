//
//  CutoffFilter.hpp
//  Reverb
//
//  Created by James Kelly on 31/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//

#ifndef CutoffFilter_hpp
#define CutoffFilter_hpp

#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "fmod.hpp"
#include "DelayUnit.hpp"

const float MIN_CUTOFF = 20.0f;
const float MAX_CUTOFF = 20000.0f;

/// Basic cutoff filter that can be used as a lowpass or highpass filter
class CutoffFilter
{
public:
    CutoffFilter() :
    m_xBuffer(nullptr),
    m_yBuffer(nullptr),
    m_cutoff(MAX_CUTOFF),
    m_isHighpass(false),
    m_sampleRate(44100),
    m_channels(-1)
    { }
    
    ~CutoffFilter()
    {
        delete m_xBuffer;
        delete m_yBuffer;
    }
    
    /// Initialise the plugin
    void Init (FMOD_DSP_STATE*);
    
    /// Release resources
    void Release ();
    
    /// Get cutoff frequency of filter (20Hz - 20,000Hz)
    float GetCutoff () const { return m_cutoff; }
    
    /// Returns the state of the filter. True = highpass, False = lowpass
    bool GetHighpass () const  { return m_isHighpass; }
    
    /// Set cutoff frequency of filter (20Hz - 20,000Hz)
    void SetCutoff (float value) { m_cutoff = value; }
    
    /// Set the state of the filter. True = highpass, False = lowpass
    void SetHighpass (bool value) { m_isHighpass = value; }

    /// Main DSP processing
    void Read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    
    /// Read one channel / sample at a time
    void ReadSingle(float* inSample, float* outSample, int channels);
    
    /// Read one channel / sample at a time
    void ReadSingle(float* inSample, float* outSample, int channels, float feedback);
    
private:
    DelayUnit* m_xBuffer;
    
    DelayUnit* m_yBuffer;
    
    float m_cutoff;
    
    FMOD_BOOL m_isHighpass;
    
    int m_sampleRate;
    
    int m_channels;
};

#endif /* CutoffFilter_hpp */
