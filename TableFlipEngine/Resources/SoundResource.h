/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef SOUND_RESOURCE_H
#define SOUND_RESOURCE_H

#include <AL/al.h>
#include "Utilities.h"

enum SoundTypes { SNDTYPE_WAV = 0, SNDTYPE_OGG, SND_NUMTYPES };

struct RIFFHeader
{
    char           id[4];        // "RIFF"
    unsigned int   length;       // size of chunk ( may include extra bytes )
    char           fmtId[4];     // "WAVE", "AVI", etc.
};
namespace Engine
{
struct ChunkHeader
{
    char           type[4];      // chunk type
    unsigned int   length;       // size of chunk
};
}
struct WavFormat
{
    short formatTag;
    short numChannels;
    int   sampleRate;
    int   avgBytesPerSecond;
    short blockAlign;
    short bitsPerSample;
};

class SoundResource : public Resource
{
private:
    SoundTypes type;
    ALuint buffer;  //if SNDTYPE_WAV theres only buffer loaded into OpenAL
    ~SoundResource(); 
public:
    void Delete() { delete this; }
    SoundResource() { type = SND_NUMTYPES; }
    
    bool LoadSoundWav(const char *filename);
    ALuint GetBuffer() 
    { 
        if (type == SNDTYPE_WAV)
            return buffer;   
        else
        {
            LogError("GetBuffer() : only WAV sound types contain a buffer.");
            return 0;
        }
    }
    
    
};

#endif
