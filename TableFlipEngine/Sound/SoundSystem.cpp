/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include "SoundSystem.h"
#include "Utilities.h"
#include <iostream>
#include <string.h>

using std::cout;

SoundSystem *SoundSystem::instance = 0;


//testt and hacky right now..
ALfloat listenerPos[]={0.0,0.0,0.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0};      // Listener facing into the screen


SoundSystem::SoundSystem()
{
    efxSupported = false;
}
SoundSystem::~SoundSystem()
{}

void SoundSystem::InitInstance()
{
    if (!instance)
        instance = new SoundSystem;
}

void SoundSystem::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}



bool SoundSystem::Initialize()
{
    
      //WTF VALGRIND LEAKAGE
        device = 0;
        context = 0;
        ALint attribs[4] = {0};
        
     //   listenerPos[]={0.0,0.0,0.0};
     //   listenerVel[]={0.0,0.0,0.0};
     //   listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0};      // Listener facing into the screen

        device = alcOpenDevice(NULL); //open 'preferred' device
        if (!device) return false;
        
        if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_FALSE)
        {
            cout << "OpenAL - NO EFX SUPPORT!?\n";
            efxSupported = false;
            //return false;
        }
        else
            efxSupported = true;
        
        
        AuxCount = 0;
        attribs[0] = ALC_MAX_AUXILIARY_SENDS;
        attribs[1] = 4;

        if (efxSupported)
            context = alcCreateContext(device, attribs);
        else
            context = alcCreateContext(device, 0);
        if (!context) 
        {
            LogError("Could not create OpenAL context");
            return false;
        }
        alcMakeContextCurrent(context);
        if ((error = alGetError()) != AL_NO_ERROR) return false; //TODO: error reporting

        if (efxSupported)
            alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &AuxCount);
        else
            AuxCount = 0;
        cout << "AL context created with [" << AuxCount << "] aux sends!\n";
        
        //verify the efx system exists  TODO this! notice that define at the top??  make this safe!
        //page 28 of effects extension guide
        //alGenEffects=(LPALGENEFFECTS)alGetProcAddress("alGenEffects");
     

        alListenerfv(AL_POSITION,listenerPos);  // Position ...
        alListenerfv(AL_VELOCITY,listenerVel);  // Velocity ...
        alListenerfv(AL_ORIENTATION,listenerOri);       // Orientation ...

        //gen a test buffer
        alGenBuffers(1, &testBuffer);

    
    
    
    
        //gen aux sends
       /* if (efxSupported)
        {
            for (int i = 0; i < AuxCount; i++)
            {
                alGenAuxiliaryEffectSlots(1, &aux[i]);
                if (alGetError() != AL_NO_ERROR)
                {
                    cout << "OPENAL ERROR GENERATING AUX SEND!!! whyyyy?\n";
                }
            }
            
            //gen a test effect
            alGenEffects(1, &testEffect);
            
            if (alGetError() != AL_NO_ERROR)
               LogError("OPENAL ERROR GENERATING EFFECT, WHHHHHY!!!???!?\n");
            
            
            rvDensity = 1.0f;
            rvDiffusion = 1.0f;
            rvDecay = 1.49f;
            rvRolloff = 0.0f;
            
            //lets make this a reverb
            alEffecti(testEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            if (alGetError() != AL_NO_ERROR)
                cout << "OPENAL ERROR NO REVERB!!>\n";
            else // set props
            {
                alEffectf(testEffect, AL_REVERB_DECAY_TIME, rvDecay);
                alEffectf(testEffect, AL_REVERB_DIFFUSION, rvDiffusion);
                alEffectf(testEffect, AL_REVERB_GAIN, rvDensity);
                alEffectf(testEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, rvRolloff);
            }
            
            //test filter
            //gen a test filter
            alGenFilters(1, &testFilter);
            
            if (alGetError() != AL_NO_ERROR)
                cout << "OPENAL ERROR GENERATING FILTER, WHHHHHY!!!???!?\n";
            
            //lets make this a lowpass
            alFilteri(testFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
            if (alGetError() != AL_NO_ERROR)
                cout << "OPENAL ERROR NO LOW PASS!!>\n";
            else // set props
            {
                alFilterf(testFilter, AL_LOWPASS_GAIN, 0.90f);
                alFilterf(testFilter, AL_LOWPASS_GAINHF, 0.9900001f);
            }
            
            //attach effect to aux
            alAuxiliaryEffectSloti(aux[0], AL_EFFECTSLOT_EFFECT, testEffect);
            if (alGetError() != AL_NO_ERROR)
                cout << "cannot assign effect to aux 0\n";
        }*/
        
        //load test wav
       // ALsizei size, freq;
        //ALenum format;
        //ALvoid *data;
        //ALboolean loop;

        //TODO write wav loader
        /*LogInfo("Loading wav file");
        
        ALbyte filename[]="Data/Sounds/dstryit.wav";
        alutLoadWAVFile(filename, &format, &data, &size, &freq, &loop);
        if ((error = alGetError()) != AL_NO_ERROR) { alDeleteBuffers(1, &testBuffer); return false; }

         LogInfo("buffering data");
        //put the data into the openal system
        alBufferData(testBuffer, format, data, size, freq);
        alutUnloadWAV(format, data, size, freq);
    
         LogInfo("gen sources");
        //create a source
        alGenSources(1, &testSource);
        alSourcef(testSource, AL_PITCH, 1.0f);
        alSourcef(testSource, AL_GAIN, 1.0f);
        alSourcefv(testSource, AL_POSITION, listenerPos);
        alSourcefv(testSource, AL_VELOCITY, listenerVel);
        alSourcei(testSource, AL_BUFFER, testBuffer);
        alSourcei(testSource, AL_LOOPING, AL_FALSE);
        */
        //EFFECT!
       /* if (efxSupported)
        {
            //aux 0  no filter
            alSource3i(testSource, AL_AUXILIARY_SEND_FILTER, aux[0], 0, testFilter );
            //alSourcei(testSource, AL_DIRECT_FILTER, testFilter);
            
            if (alGetError() != AL_NO_ERROR)
                LogError("source send to aux 0 FAILURE\n");
        }*/
        
      testSynth.Init();
        
       
        
        return true;
}

void SoundSystem::Shutdown()
{
      /*  alDeleteSources(1, &testSource);
        alDeleteBuffers(1, &testBuffer);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);*/
}

void SoundSystem::PlayTest()
{
    LogInfo("Playtest()");
    if (testSynth.isPlaying())
        testSynth.Stop();
    else
        testSynth.Play();
        alSourcePlay(testSource);
}

void SoundSystem::Update()
{
    testSynth.Update();
}


void SoundSystem::UpdateEffectParams()
{
    if (!efxSupported)
        return;
    
   /* alEffectf(testEffect, AL_REVERB_DECAY_TIME, rvDecay);
    alEffectf(testEffect, AL_REVERB_DIFFUSION, rvDiffusion);
    alEffectf(testEffect, AL_REVERB_GAIN, rvDensity);
    alEffectf(testEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, rvRolloff);
    
    alAuxiliaryEffectSloti(aux[0], AL_EFFECTSLOT_EFFECT, testEffect);*/
}



SoundSample::SoundSample ( SoundResource* snd )
{
    if (!snd)
    {
        LogError("SoundSample() no sound resource to reference!");
        return;
    }
    soundResource = snd;
    snd->AddRef();
    
    alGenSources(1, &source);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 0.570f);
    alSourcefv(source, AL_POSITION, listenerPos);
    alSourcefv(source, AL_VELOCITY, listenerVel);
    alSourcei(source, AL_BUFFER, snd->GetBuffer());
    alSourcei(source, AL_LOOPING, AL_FALSE);
    
    
}
SoundSample::~SoundSample()
{
    soundResource->RemoveRef();
}
void SoundSample::Play()
{
    ALint alState;
    alGetSourcei(source, AL_SOURCE_STATE, &alState);
    
    if (alState == AL_PLAYING)
        return;
    
    alSourcePlay(source);
}
void SoundSample::Stop()
{
    alSourceStop(source);
}
void SoundSample::Pause()
{
    alSourcePause(source);
}

