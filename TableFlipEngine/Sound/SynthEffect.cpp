/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "SynthEffect.h"
#include "EngineMath.h"
#include "Logger.h"
#include "Utilities.h"

//#define BUFFER_LENGTH 44100 * 2
#define BUFFER_LENGTH (44100 / 8)
short pcm1[BUFFER_LENGTH];
short pcm2[BUFFER_LENGTH]; //need 2 so i can interpolate a smooth transition'
bool firstBuffer = true;
float freq = 100;
float step = 0;

ALfloat defaultPos[]={0.0,0.0,0.0};
ALfloat defaultVel[]={0.0,0.0,0.0};

SynthEffect::SynthEffect()
{
    waveType = WAVE_SINE;
    phase=0.0f;
    freqAdjust = 0;
    source = 0;
    buffer[0] = 0; buffer[1] = 0;
    playing = false;
    swapBuff = false;

    waveGen = new WaveGenerator(44100, 16, 2.0);
}

SynthEffect::~SynthEffect()
{
    delete waveGen;
}

void SynthEffect::Init()
{
    alGenBuffers(2, buffer);
    alGenSources(1, &source);

    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 0.29f);
    alSourcefv(source, AL_POSITION, defaultPos);
    alSourcefv(source, AL_VELOCITY, defaultVel);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE); //puts source right on listener

    format = AL_FORMAT_MONO16;
}

void SynthEffect::Play()
{
    if (playing)
        return;
    //load data
    SoundWave wave;
    switch (waveType)
    {
        default:
        case WAVE_TRIANGLE:
            wave = waveGen->TriangleWave(180, 0.8, 0.5);
            alBufferData(buffer[0], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            wave = waveGen->TriangleWave(180, 0.8, 0.5);
            alBufferData(buffer[1], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            break;
        case WAVE_SINE:
            wave = waveGen->SineWave(180, 0.8, 0.5);
            alBufferData(buffer[0], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            wave = waveGen->SineWave(180, 0.8, 0.5);
            alBufferData(buffer[1], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            break;
        case WAVE_SQUARE:
            wave = waveGen->SquareWave(180, 0.8, 0.5);
            alBufferData(buffer[0], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            wave = waveGen->SquareWave(180, 0.8, 0.5);
            alBufferData(buffer[1], format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
            break;
    }
    alSourceQueueBuffers(source, 2, buffer);

    alSourcePlay(source);
    playing = true;

}

void SynthEffect::Stop()
{
    if (!playing)
        return;
    playing = false;
    alSourceStop(source);
    ALuint buff;
    alSourceUnqueueBuffers(source, 2, &buff);
}

void SynthEffect::Update()
{
    if (!playing)
        return;
    int processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);


    //hard limitey TODO BAD?
    SoundWave wave;
    while(processed > 0 && processed < 10)
    {
        processed--;
        ALuint buff;
        alSourceUnqueueBuffers(source, 1, &buff);
        switch (waveType)
        {
            default:
            case WAVE_TRIANGLE:
                wave = waveGen->SineWave(180, 0.8, 0.5);
                alBufferData(buff, format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
                break;
            case WAVE_SINE:
                wave = waveGen->SineWave(180, 0.8, 0.5);
                alBufferData(buff, format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
                break;
            case WAVE_SQUARE:
                wave = waveGen->SquareWave(180, 0.8, 0.5);
                alBufferData(buff, format, wave.data, wave.numSamples * wave.bytesPerSample, 44100);
                break;
        }
        alSourceQueueBuffers(source, 1, &buff);
    }

    alGetSourcei(source, AL_BUFFERS_QUEUED, &processed);
    if (!processed)
        Stop();
}
