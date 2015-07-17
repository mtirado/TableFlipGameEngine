/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _SYNTH_EFFECT_H_
#define _SYNTH_EFFECT_H_

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alut.h"

#include "WaveGen.h"

class SynthEffect
{
private:
    bool playing;
    bool swapBuff; //false = buffer[0] - true = buffer[1]
    ALuint buffer[2];
    ALuint source;
    ALenum format;
    float phase;
    float dfreq;
    short freqAdjust; //so smooth out frequency changes
    WaveType waveType;

    WaveGenerator *waveGen;
    WaveGenerator *sineGen;
    WaveGenerator *squareGen;
    
public:
    SynthEffect();
    ~SynthEffect();
    void Init();
    void Update();
    void Play();
    void Stop();

    inline bool isPlaying() { return playing; }

};

#endif
