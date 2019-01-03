//
//  DelayUnit.cpp
//  Reverb
//
//  Created by James Kelly on 31/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//

#include "DelayUnit.hpp"

void DelayUnit::Init(FMOD_DSP_STATE* dsp_state)
{
    m_writePos = 0;
    m_delayTime = DELAY_PLUGIN_INIT_DELAY_TIME_MS;
    m_feedbackAmount = DELAY_PLUGIN_FEEDBACK_INIT;
    m_dryAmount = DELAY_PLUGIN_LEVELS_INIT;
    m_wetAmount = DELAY_PLUGIN_LEVELS_INIT;
    FMOD_DSP_GETSAMPLERATE(dsp_state, &m_sampleRate);
    m_maxDelayTime = DELAY_PLUGIN_MAX_DELAY_TIME_MS;
    m_maxSampleDelayTime = MS_TO_SAMPLES(m_maxDelayTime, m_sampleRate);
    m_numOfChannels = -1;
    
    //m_maxDelay = MS_TO_SAMPLES(m_maxDelayTime, m_sampleRate) * m_numOfChannels;
    
}

void DelayUnit::Init(FMOD_DSP_STATE* dsp_state, float max_delay_ms)
{
    m_writePos = 0;
    m_delayTime = DELAY_PLUGIN_INIT_DELAY_TIME_MS;
    m_feedbackAmount = DELAY_PLUGIN_FEEDBACK_INIT;
    m_dryAmount = DELAY_PLUGIN_LEVELS_INIT;
    m_wetAmount = DELAY_PLUGIN_LEVELS_INIT;
    FMOD_DSP_GETSAMPLERATE(dsp_state, &m_sampleRate);
    m_maxDelayTime = max_delay_ms;
    m_maxSampleDelayTime = MS_TO_SAMPLES(m_maxDelayTime, m_sampleRate);
    m_numOfChannels = -1;
}

void DelayUnit::Init(FMOD_DSP_STATE* dsp_state, int max_samples)
{
    m_writePos = 0;
    m_delayTime = 1;
    m_feedbackAmount = DELAY_PLUGIN_FEEDBACK_INIT;
    m_dryAmount = DELAY_PLUGIN_LEVELS_INIT;
    m_wetAmount = DELAY_PLUGIN_LEVELS_INIT;
    FMOD_DSP_GETSAMPLERATE(dsp_state, &m_sampleRate);
    m_maxSampleDelayTime = max_samples;
    m_maxDelayTime = SAMPLES_TO_MS(m_maxSampleDelayTime, m_sampleRate);
    m_numOfChannels = -1;
}

void DelayUnit::CreateBuffers(int channels)
{
    if (m_numOfChannels != channels)
    {
        m_numOfChannels = channels;
        
        m_delayBuffer = new DelayBuffer(GetMaxBufferSize());
    }
}

void DelayUnit::Release()
{
    delete m_delayBuffer;
}

void DelayUnit::TickSample()
{
    int bufferLength = GetMaxBufferSize();
    
    m_writePos += m_numOfChannels;  // move forward one samples (or move forward all channels until we are at the same channel but at sample n+1
    while (m_writePos >= bufferLength) m_writePos -= bufferLength;
}

void DelayUnit::TickChannel()
{
    int bufferLength = GetMaxBufferSize();
    m_writePos++;   // Move foward 1, or one channel
    if (m_writePos >= bufferLength) m_writePos = 0;   // If we are at the end of the buffer, go to the start
}

float DelayUnit::GetDelayedSample()
{
    if (m_delayBuffer)
    {
        float r, previousIndex, nextIndex;
        int bufferLength = GetMaxBufferSize();
        
        float delayInSamples = MS_TO_SAMPLES(m_delayTime, m_sampleRate) * m_numOfChannels;
        
        r = modff(m_writePos - delayInSamples, &previousIndex);
        
        nextIndex = previousIndex + 1;
        
        while (previousIndex >= bufferLength) previousIndex -= bufferLength;
        while (previousIndex < 0) previousIndex += bufferLength;
        
        while (nextIndex >= bufferLength) nextIndex -= bufferLength;
        while (nextIndex < 0) nextIndex += bufferLength;
        
        float previousValue, nextValue;
        
        previousValue = (*m_delayBuffer)[(int)previousIndex];
        nextValue = (*m_delayBuffer)[(int)nextIndex];
        
        return (previousValue*(1-r)+nextValue*r);
    }
    return 0.0f;
}

float DelayUnit::GetDelayedSampleAt(float ms)
{
    if (m_delayBuffer)
    {
        float r, previousIndex, nextIndex;
        int bufferLength = GetMaxBufferSize();
        
        float delayInSamples = MS_TO_SAMPLES(ms, m_sampleRate) * m_numOfChannels;
        
        r = modff(m_writePos - delayInSamples, &previousIndex);
        
        nextIndex = previousIndex + 1;
        
        while (previousIndex >= bufferLength) previousIndex -= bufferLength;
        while (previousIndex < 0) previousIndex += bufferLength;
        
        while (nextIndex >= bufferLength) nextIndex -= bufferLength;
        while (nextIndex < 0) nextIndex += bufferLength;
        
        float previousValue, nextValue;
        
        previousValue = (*m_delayBuffer)[(int)previousIndex];
        nextValue = (*m_delayBuffer)[(int)nextIndex];
        
        return (previousValue*(1-r)+nextValue*r);
    }
    
    return 0.0f;
}

float DelayUnit::GetDelayedSampleAt(int sample)
{
    if (m_delayBuffer)
    {
        float r, previousIndex, nextIndex;
        int bufferLength = GetMaxBufferSize();
        
        float delayInSamples = sample * m_numOfChannels;
        
        r = modff(m_writePos - delayInSamples, &previousIndex);
        
        nextIndex = previousIndex + 1;
        
        while (previousIndex >= bufferLength) previousIndex -= bufferLength;
        while (previousIndex < 0) previousIndex += bufferLength;
        
        while (nextIndex >= bufferLength) nextIndex -= bufferLength;
        while (nextIndex < 0) nextIndex += bufferLength;
        
        float previousValue, nextValue;
        
        previousValue = (*m_delayBuffer)[(int)previousIndex];
        nextValue = (*m_delayBuffer)[(int)nextIndex];
        
        return (previousValue*(1-r)+nextValue*r);
    }
    
    return 0.0f;
}

void DelayUnit::WriteDelay(float value)
{
    if (m_delayBuffer)
    {
        (*m_delayBuffer)[m_writePos] = value;
    }
}

void DelayUnit::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    for (unsigned int i = 0; i < length; i++)
    {
        for (unsigned int n = 0; n < channels; n++)
        {
            float drySample(0), wetSample(GetDelayedSample());
            
            drySample = *inbuffer++;
        
            WriteDelay(drySample + (wetSample * GetFeedback()));
        
            *outbuffer++ = (drySample * GetDry()) + ((wetSample) * GetWet());

            TickChannel();
        }
    }
}

void DelayUnit::ReadSingle(float *inSample, float *outSample)
{
    float drySample(0), wetSample(GetDelayedSample());
    
    drySample = *inSample;
    
    WriteDelay(drySample + (wetSample * GetFeedback()));
    
    *outSample = (drySample * GetDry()) + ((wetSample) * GetWet());
    
    TickChannel();
}
