//
//  DelayUnit.hpp
//  Reverb
//
//  Created by James Kelly on 31/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//

#ifndef DelayUnit_hpp
#define DelayUnit_hpp

#include <math.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "fmod.hpp"

#define MS_TO_SAMPLES(__ms__, __rate__) ((__ms__ * __rate__) / 1000.0f)
#define SAMPLES_TO_MS(__samples__, __rate__) ((__samples__ * 1000.0f) / __rate__)
#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= DELAY_PLUGIN_LEVELS_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? DELAY_PLUGIN_LEVELS_MIN : 20.0f * log10f((float)__linval__))

const float DELAY_PLUGIN_MIN_DELAY_TIME_MS = 1.0f;      // 1ms
const float DELAY_PLUGIN_MAX_DELAY_TIME_MS = 1000.0f;   // 1,000ms / 1 second
const float DELAY_PLUGIN_INIT_DELAY_TIME_MS = 5.0f;     // 5ms

// levels for both dry and wet
const float DELAY_PLUGIN_LEVELS_MIN = -80.0f;
const float DELAY_PLUGIN_LEVELS_MAX = 10.0f;
const float DELAY_PLUGIN_LEVELS_INIT = 0.0f;

const float DELAY_PLUGIN_FEEDBACK_MIN = 0.0f;
const float DELAY_PLUGIN_FEEDBACK_MAX = 100.0f;
const float DELAY_PLUGIN_FEEDBACK_INIT = 0.0f;

typedef std::vector<float> DelayBuffer;

/// Basic delay line that can change its delay length at initialisation
class DelayUnit
{
public:
    DelayUnit() :
    m_delayBuffer(nullptr),
    m_writePos(0),
    m_delayTime(DELAY_PLUGIN_INIT_DELAY_TIME_MS),
    m_feedbackAmount(DELAY_PLUGIN_FEEDBACK_INIT),
    m_dryAmount(DELAY_PLUGIN_LEVELS_INIT),
    m_wetAmount(DELAY_PLUGIN_LEVELS_INIT),
    m_sampleRate(44100),
    m_numOfChannels(-1),
    m_maxSampleDelayTime(2)
    { }
    
    ~DelayUnit()
    {
        delete m_delayBuffer;
    }
    
    /// Initialise the plugin with default delay times
    void Init (FMOD_DSP_STATE*);
    
    /// Initialise the plugin and set max delay time
    void Init (FMOD_DSP_STATE*, float);
    
    /// Initialise the plugin and set maximum number of samples
    void Init (FMOD_DSP_STATE*, int);
    
    /// Release resources
    void Release ();
    
    /// Called before read. Creates buffers
    void CreateBuffers (int);
    
    /// Get delay time in ms
    float GetDelayTime() const {return m_delayTime; }
    
    /// Get linear feedback (0 - 1)
    float GetFeedback() const {return ((m_feedbackAmount > DELAY_PLUGIN_FEEDBACK_MAX) ? DELAY_PLUGIN_FEEDBACK_MAX : (m_feedbackAmount < DELAY_PLUGIN_FEEDBACK_MIN) ? DELAY_PLUGIN_FEEDBACK_MIN : m_feedbackAmount) / 100;}
    
    /// Get feedback percentage
    float GetFeedbackPercent () const { return m_feedbackAmount; }
    
    /// Return dry amount in linear or dB format
    float GetDry(bool linear = true) const {return linear ? DECIBELS_TO_LINEAR(m_dryAmount) : m_dryAmount; }
    
    /// Return wet amount in linear or dB format
    float GetWet(bool linear = true) const {return linear ? DECIBELS_TO_LINEAR(m_wetAmount) : m_wetAmount; }
    
    /// Set delay time in ms
    void SetDelayTime(float value) { m_delayTime = value; }
    
    /// Set feedback amount in percentage %
    void SetFeedback(float value) { m_feedbackAmount = value; }
    
    /// Set dry amount in dB
    void SetDry(float value) { m_dryAmount = value; }
    
    /// Set wet amount in dB
    void SetWet(float value) { m_wetAmount = value; }
    
    /// Get maximum number of samples in buffer
    int GetMaxDelayTimeInSamples () const { return m_maxSampleDelayTime; }
    
    /// Get total size of buffer
    int GetMaxBufferSize () const { return GetMaxDelayTimeInSamples() * m_numOfChannels; }
    
    /// Advance the write position by one sample
    void TickSample();
    
    /// Advance the write position by one channel
    void TickChannel();
    
    /// Get the audio value at the set delay time
    float GetDelayedSample();
    
    /// Get the value at the negative time set by the parameter
    float GetDelayedSampleAt(float);
    
    /// Get the value at number of samples back
    float GetDelayedSampleAt(int);
    
    /// Write the sample into the delay buffer
    void WriteDelay(float);
    
    /// Main DSP processing
    void Read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    
    void ReadSingle(float* inSample, float* outSample);
    
private:
    /// Buffer of samples
    DelayBuffer* m_delayBuffer;
    
    /// Which index we are writing the audio into.
    /// This is not the sample index as the buffer holds multiple channels
    int m_writePos;

    /// Delay time in ms
    float m_delayTime;
    
    /// Amount in % for feedback
    float m_feedbackAmount;
    
    /// Amount in dB for dry signal
    float m_dryAmount;
    
    /// Amount in dB for wet / delayed signal
    float m_wetAmount;
    
    /// Sample rate of host application
    int m_sampleRate;
    
    /// Number of channels in one sample
    int m_numOfChannels;
    
    /// Maximum delay time in ms
    float m_maxDelayTime;
    
    /// Maximum time of delay in samples. More accurately, the number of samples in the buffer regardless of channels
    int m_maxSampleDelayTime;
};

#endif /* DelayUnit_hpp */
