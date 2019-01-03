//
//  CutoffFilter.cpp
//  Reverb
//
//  Created by James Kelly on 31/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//

#include "CutoffFilter.hpp"

void CutoffFilter::Init(FMOD_DSP_STATE* dsp_state)
{
    delete m_xBuffer;
    delete m_yBuffer;
    
    FMOD_DSP_GETSAMPLERATE(dsp_state, &m_sampleRate);
    
    m_xBuffer = new DelayUnit();
    m_yBuffer = new DelayUnit();
    
    m_xBuffer->Init(dsp_state, 1);
    m_yBuffer->Init(dsp_state, 1);
    
    m_isHighpass = false;
}

void CutoffFilter::Release()
{
    if (m_xBuffer)
    {
        m_xBuffer->Release();
    }
    
    if (m_yBuffer)
    {
        m_yBuffer->Release();
    }
    
    delete m_xBuffer;
    delete m_yBuffer;
}

void CutoffFilter::Read(float *inbuffer, float *outbuffer, unsigned int length, int channels)
{
    if (m_xBuffer && m_yBuffer)
    {
        m_xBuffer->CreateBuffers(channels);
        m_yBuffer->CreateBuffers(channels);
        
        static float beta = 0;
        static float hBeta = 0;  // highpass beta
        
        // Loop through all samples
        for (unsigned int i = 0; i < length; i++)
        {
            // recalculate beta for each sample
            beta = (m_cutoff - MIN_CUTOFF) / (MAX_CUTOFF - MIN_CUTOFF);
            hBeta = 1 - beta;
            
            // Loop through all channels within the sample (audio is interleaved)
            for (unsigned int n = 0; n < channels; n++)
            {
                float currentInput = *inbuffer;
                float previousOutput = m_yBuffer->GetDelayedSampleAt(1);
                float previousInput = m_xBuffer->GetDelayedSampleAt(1);
                
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
                m_xBuffer->WriteDelay(*inbuffer++);
                m_yBuffer->WriteDelay(*outbuffer++);
                
                m_xBuffer->TickChannel();
                m_yBuffer->TickChannel();
                
            }
        }
    }
}

void CutoffFilter::ReadSingle(float *inSample, float *outSample, int channels)
{
    m_xBuffer->CreateBuffers(channels);
    m_yBuffer->CreateBuffers(channels);
    
    static float beta = 0;
    static float hBeta = 0;  // highpass beta
    
    // recalculate beta for each sample
    beta = (m_cutoff - MIN_CUTOFF) / (MAX_CUTOFF - MIN_CUTOFF);
    hBeta = 1 - beta;
    
    float currentInput = *inSample;
    float previousOutput = m_yBuffer->GetDelayedSampleAt(1);
    float previousInput = m_xBuffer->GetDelayedSampleAt(1);
    
    if (!m_isHighpass)
    {
        // y[i] := y[i-1] + α * (x[i] - y[i-1])
        *outSample = previousOutput + beta * (currentInput - previousOutput);
    }
    else
    {
        // y[i] := α * (y[i-1] + x[i] - x[i-1])
        *outSample = hBeta * (previousOutput + currentInput - previousInput);
    }
    
    // Store previous values
    m_xBuffer->WriteDelay(*inSample);
    m_yBuffer->WriteDelay(*outSample);
    
    m_xBuffer->TickChannel();
    m_yBuffer->TickChannel();
}

void CutoffFilter::ReadSingle(float *inSample, float *outSample, int channels, float feedback)
{
    m_xBuffer->CreateBuffers(channels);
    m_yBuffer->CreateBuffers(channels);
    
    static float beta = 0;
    static float hBeta = 0;  // highpass beta
    
    // recalculate beta for each sample
    beta = (m_cutoff - MIN_CUTOFF) / (MAX_CUTOFF - MIN_CUTOFF);
    hBeta = 1 - beta;
    
    float currentInput = *inSample;
    float previousOutput = m_yBuffer->GetDelayedSampleAt(1);
    float previousInput = m_xBuffer->GetDelayedSampleAt(1);
    
    if (!m_isHighpass)
    {
        // y[i] := y[i-1] + α * (x[i] - y[i-1])
        *outSample = previousOutput + beta * (currentInput - previousOutput);
    }
    else
    {
        // y[i] := α * (y[i-1] + x[i] - x[i-1])
        *outSample = hBeta * (previousOutput + currentInput - previousInput);
    }
    
    // Store previous values
    m_xBuffer->WriteDelay(*inSample + (*outSample * feedback));
    m_yBuffer->WriteDelay(*outSample);
    
    m_xBuffer->TickChannel();
    m_yBuffer->TickChannel();
}
