/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _WAVE_GEN_H_
#define _WAVE_GEN_H_

enum WaveType { WAVE_SINE = 0, WAVE_SAW, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_NUMTYPES };

struct SoundWave
{
    WaveType type;
    char *data;
    unsigned int numSamples;
    int bytesPerSample;
};

class WaveGenerator
{
private:
    int sampleRate;
    int bitDepth;   //lowest data type is char, 32bits max (int)!
    char *pcm;      //this buffer is sized for constructors maxSeconds param

    unsigned int maxSamples;
    int maxValue;
    int bytesPerSample;

    //keep track of the wave position
    float phase;
    int index;
    //float delta;
    WaveGenerator(){};
public:
    //can not generate beyond max seconds
    WaveGenerator(int _sampleRate, int _bitDepth, float maxSeconds);
    ~WaveGenerator() { delete[] pcm; }

    SoundWave SquareWave(float frequency, float amplitude, float seconds);
    SoundWave SineWave(float frequency, float amplitude, float seconds);
    SoundWave TriangleWave(float frequency, float amplitude, float seconds);
    
};

#endif
