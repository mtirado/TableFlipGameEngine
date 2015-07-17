/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "EngineMath.h"
#include "WaveGen.h"

WaveGenerator::WaveGenerator(int _sampleRate, int _bitDepth, float maxSeconds)
{
    sampleRate = _sampleRate;
    bitDepth = _bitDepth;
    phase = 0.0f;
    index = 0;
    //delta = 0.0f;

    bytesPerSample = 1; //<=8bit
    if (bitDepth > 8 && bitDepth <= 16)
        bytesPerSample = 2;
    else
        bytesPerSample = 4;

    if (bitDepth < 0)
        bitDepth = 1;
    else if (bitDepth > 32)
        bitDepth = 32;
    
    maxValue = (1 << (bitDepth-1)) - 1;    
    maxSamples = maxSeconds * (double)sampleRate;
    
    //hopefully floating point error doesnt screw us?
    pcm = new char[maxSamples * bytesPerSample];
}

SoundWave WaveGenerator::SquareWave(float frequency, float amplitude, float seconds)
{
    if (seconds < 0)
        seconds = 0;
        
    if (amplitude > 1.0f)
        amplitude = 1.0f;
    else if (amplitude < 0.0f)
        amplitude = 0.01f;

    unsigned int numSamples = seconds * sampleRate;
    if (numSamples > maxSamples)
        numSamples = maxSamples; //dont go outta bounds!

    float amp = amplitude;
    unsigned int i;
    for (i = 0; i < numSamples; i++)
    {
        if (phase < M_PI)
            amp = amplitude;
        else
            amp = -amplitude;
        
        phase = phase + ((2 * M_PI * frequency) / sampleRate);
        switch (bytesPerSample)
        {
            case 2:
                ((short *)pcm)[i] = (short)(maxValue * amp );
            break;
            case 4:
                ((int *)pcm)[i] = (int)(maxValue * amp );
            break;
            default:
                ((char *)pcm)[i] = (char)(maxValue * amp );
            break;
        }
        

        if (phase > (2*M_PI))
            phase = phase - (2 * M_PI);
    }

    SoundWave wave;
    wave.data = pcm;
    wave.numSamples = numSamples;
    wave.type = WAVE_SQUARE;
    wave.bytesPerSample = bytesPerSample;
    return wave;
}

SoundWave WaveGenerator::SineWave(float frequency, float amplitude, float seconds)
{
    if (seconds < 0)
        seconds = 0;

    if (amplitude > 1.0f)
        amplitude = 1.0f;
    else if (amplitude < 0.0f)
        amplitude = 0.01f;

    unsigned int numSamples = seconds * sampleRate;
    if (numSamples > maxSamples)
        numSamples = maxSamples; //dont go outta bounds!

    unsigned int i;
    for (i = 0; i < numSamples; i++)
    {
        float delta = 2*M_PI*(frequency / sampleRate);

        switch (bytesPerSample)
        {
            case 2:
                ((short *)pcm)[i] = (short)(amplitude * maxValue * sin(phase));
            break;
            case 4:
                ((int *)pcm)[i] = (int)(amplitude * maxValue * sin(phase));
            break;
            default:
                ((char *)pcm)[i] = (char)(amplitude * maxValue * sin(phase));
            break;
        }
        
        //pcm[i] = (amplitude * maxValue * sin(phase));
        
        phase += delta;
        if (phase > 2 * M_PI)
            phase = phase - 2 * M_PI;
    }

    SoundWave wave;
    wave.data = pcm;
    wave.numSamples = numSamples;
    wave.type = WAVE_SINE;
    wave.bytesPerSample = bytesPerSample;
    return wave;
}

SoundWave WaveGenerator::TriangleWave(float frequency, float amplitude, float seconds)
{
    //int triIndex = 0;void generateTriangle(SInt16 *sampleBuffer, int numFrames, float sampleRate, float frequency, float amp) {
    if(amplitude > 1.0)
        amplitude = 1.0;
    if( amplitude < 0)
        amplitude = 0.01;
    
    //amp = amp * SHRT_MAX;
    amplitude = amplitude * maxValue; //max bits
    float samplesPerCycle = sampleRate/frequency;

     unsigned int numSamples = seconds * sampleRate;
    if (numSamples > maxSamples)
        numSamples = maxSamples; //dont go outta bounds!

    
    for(int i = 0; i < numSamples; i++)
    {
        if(fmodf(index, samplesPerCycle)/samplesPerCycle>0.5)
        {
            //pcm[i] = (short)amplitude * ((2-2*((fmodf(index, samplesPerCycle) / samplesPerCycle-0.5)/0.5))-1);
            switch (bytesPerSample)
            {
                case 2:
                    ((short *)pcm)[i] = (short)amplitude * ((2-2*((fmodf(index, samplesPerCycle) / samplesPerCycle-0.5)/0.5))-1);
                    //((short *)pcm)[i] = (short)(amplitude * maxValue * sin(phase));
                break;
                case 4:
                    ((int *)pcm)[i] = (int)amplitude * ((2-2*((fmodf(index, samplesPerCycle) / samplesPerCycle-0.5)/0.5))-1);
                break;
                default:
                    ((char *)pcm)[i] = (char)amplitude * ((2-2*((fmodf(index, samplesPerCycle) / samplesPerCycle-0.5)/0.5))-1);
                break;
            }
        }
        else
        {
            //pcm[i] = (short)amplitude * ((2*((fmodf(index, samplesPerCycle) / samplesPerCycle)/0.5))-1);
            switch (bytesPerSample)
            {
                case 2:
                    ((short *)pcm)[i] = (short)amplitude * ((2*((fmodf(index, samplesPerCycle) / samplesPerCycle)/0.5))-1);
                    //((short *)pcm)[i] = (short)(amplitude * maxValue * sin(phase));
                break;
                case 4:
                    ((int *)pcm)[i] = (int)amplitude * ((2*((fmodf(index, samplesPerCycle) / samplesPerCycle)/0.5))-1);
                break;
                default:
                    ((char *)pcm)[i] = (char)amplitude * ((2*((fmodf(index, samplesPerCycle) / samplesPerCycle)/0.5))-1);
                break;
            }
        }
        index++;
    }
    SoundWave wave;
    wave.data = pcm;
    wave.numSamples = numSamples;
    wave.type = WAVE_TRIANGLE;
    wave.bytesPerSample = bytesPerSample;
    return wave;
}
