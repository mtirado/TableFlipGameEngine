/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef SOUND_SYSTEM_H_
#define SOUND_SYSTEM_H_


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

//TODO  pretty sure there was something wrong with the way i am getting
//these openal functions, but this works for now(tm).
#define AL_ALEXT_PROTOTYPES
#include <AL/efx.h>
#include "Resources.h"
#include "SynthEffect.h"



//TODO need to make all of these default copy constructors and assignment operators private...
class SoundSample
{
private:
    SoundResource *soundResource;
    ALuint source; //TODO add multiple samples (voices)
    ALenum fmt;
    SoundSample(){}
public:
    SoundSample(SoundResource *snd);
    ~SoundSample();
    
    void Play();
    void Stop();
    void Pause();
};


class SoundSystem
{
private:
    
    //disable functions for singleton use
    SoundSystem();
    ~SoundSystem();
    SoundSystem(const SoundSystem &) {}
    SoundSystem &operator = (const SoundSystem &) { return *this; }


    //the singletons instance.
    static SoundSystem *instance;
    
   bool efxSupported;
    
    ALint error;
    ALCcontext      *context;
    ALCdevice       *device;

    ALuint testBuffer;
    ALuint testSource;
    
    SynthEffect testSynth;

public:
    //inlines
    inline static SoundSystem *GetInstance() { return instance; }

    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();
    
        void PlayTest();
        bool Initialize();
        void Shutdown();
        void UpdateEffectParams();
        
        void Update();
        
   // vector<SynthEffect *> effects;
    
    ALCint AuxCount;
    ALuint aux[ALC_MAX_AUXILIARY_SENDS];
    ALuint testEffect;
    ALuint testFilter;
    
    
    //hacky type test stuff
    float rvDensity;
    float rvDiffusion;
    float rvDecay;
    float rvRolloff;
        
};


#endif
