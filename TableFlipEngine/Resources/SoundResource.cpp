/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Resources.h"
#include <fstream>
using std::ifstream;
using std::ios_base;


ALbyte *LoadWavFile(const char *filename, ALsizei *size, WavFormat *fmt)
{
    std::ifstream file;
    RIFFHeader header;
    Engine::ChunkHeader chunkheader;
    ALbyte *buffer = 0; 
    
    file.open(filename, ios_base::binary);
        
    if (file.is_open())
    {
        // open the file
        // read the main RIFFHeader

        file.read((char *)&header, sizeof(RIFFHeader));

        // read first chunk ? only what we need for wav format structure & skip rest of chunk
        file.read( ( char* )&chunkheader, sizeof( Engine::ChunkHeader ) );
        file.read( ( char* )&fmt->formatTag,      sizeof( short ) );
        file.read( ( char* )&fmt->numChannels,       sizeof( short ) );
        file.read( ( char* )&fmt->sampleRate,  sizeof( int ) );
        file.read( ( char* )&fmt->avgBytesPerSecond, sizeof( int ) );
        file.read( ( char* )&fmt->blockAlign,     sizeof( short ) );
        file.read( ( char* )&fmt->bitsPerSample,  sizeof( short ) );
        file.seekg( ( int )file.tellg() + ( chunkheader.length - 16 ), ios_base::beg );


        // skip all chunks that aren?t the ?data? chunk
        char check[4];
        unsigned int CheckHaxDepth = 10000; //if we didnt hit data in 10kish bytes, somethings f'd up
        unsigned int bytesChecked = 0;
        while (true)
        {
            if (bytesChecked > CheckHaxDepth)
            {
                LOGOUTPUT << "Error loading wav file: " << filename << " -- could not locate data header";
                LogError();
                return 0;
            }
            // read another ChunkHeader
            file.read((char *)&check, 4);
            // if type is "data" break
            if (!memcmp(check, "data", 4))
            {
                int pos = (int)file.tellg();
                file.seekg(0, ios_base::beg);
                file.seekg(pos - 4, ios_base::beg);
                break;
            }
            bytesChecked += 4; // TODO this is HORRIBLY HACKED, could potentially miss the check if its not aligned with 4 byte checker, fix this shit mang
        }

        //read this chunk
        file.read((char *)&chunkheader, sizeof(Engine::ChunkHeader));

        //allocate the buffer
        buffer = new ALbyte[chunkheader.length];          

        //read pcm data in
        file.read((char *)buffer, chunkheader.length);
        //TODO error check incase read fails.
        //set size
        *size = chunkheader.length;

    }
    else
    {
        LOGOUTPUT << "Could not open sound file: " << filename;
        LogError();
    }

    if (file.is_open())
        file.close();

    return buffer;    
}

SoundResource::~SoundResource()
{
    if (type == SNDTYPE_WAV)
        alDeleteBuffers(1, &buffer);
}


bool SoundResource::LoadSoundWav(const char* filename)
{
   /* if (type == SND_NUMTYPES)
    {
        LogError("LoadSoundWav(): trying to load, but sound resource is already loaded....STOP THAT!!");
        return false;
    }*/
    
    string file = filename;
   
    

    //setup variables for loading the wav
    ALsizei         alSize, alFreq;
    ALenum          alFormat;
    ALbyte          *alData;
    ALint           alError;
    //ALboolean     alLoop = bLooping;

    //load the wav file into a buffer
    WavFormat fmt;
    alData = LoadWavFile(filename, &alSize, &fmt);
    if (alData == 0)
        return false;
    
    //create the AL buffer
    ALuint tempBuffer;
    alGenBuffers(1, &tempBuffer);
    
    if (fmt.numChannels == 1)
    {
        if (fmt.bitsPerSample == 8)
        {
            alFormat = AL_FORMAT_MONO8;
        }
        else if (fmt.bitsPerSample == 16)
        {
            alFormat = AL_FORMAT_MONO16;
        }
        else
        {
            alDeleteBuffers(1, &tempBuffer);
            delete[] alData;
            LogError("Unknown wav format");
            return false;
        }
    }
    else if (fmt.numChannels == 2)
    {
        if (fmt.bitsPerSample == 8)
        {
            alFormat = AL_FORMAT_STEREO8;
        }
        else if (fmt.bitsPerSample == 16)
        {
            alFormat = AL_FORMAT_STEREO16;
        }
        else
        {
            alDeleteBuffers(1, &tempBuffer);
            delete[] alData;
            LogError("Unknown wav format");
            return false;
        }
    }
    else
    {
        alDeleteBuffers(1, &tempBuffer);
        delete[] alData;
        LogError("Unknown wav format");
        return false;
    }

    alFreq = fmt.sampleRate;
    
    //load that buffer into an alBuffer
    alBufferData(tempBuffer, alFormat, alData, alSize, alFreq);
    alError = alGetError();
    if (alError != AL_NO_ERROR)
    {
        LOGOUTPUT << "OpenAL Error - alBufferData (" << filename << ", error: " <<  alError << ")";
        LogError();
        alDeleteBuffers(1, &tempBuffer);
        delete[] alData;
        return false;
    }

    // unload the wav buffer
    delete[] alData;
    buffer = tempBuffer;

    type = SNDTYPE_WAV;
    
    return true;
}
