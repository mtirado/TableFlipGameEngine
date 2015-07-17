/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include "Utilities.h"
#include "Renderer.h"

//timer will theoretically break after about 49 days with u32bits.
//elapsed miliseconds since InitMasterTimeCode() was called.
static unsigned long G_MasterTimeCode = 0;
static unsigned long G_LastTimeCode = 0;
static float G_DTSeconds = 0.0;
static timeval timeStart;
static timeval timeCurrent;
static unsigned int FPS = 0;

unsigned long FPSTimer = 1000;
unsigned int frameCounter = 0;

void InitMasterTimecode()
{
    gettimeofday(&timeStart, 0);
}

void UpdateMasterTimecode()
{

    G_LastTimeCode = G_MasterTimeCode;
    suseconds_t elapsedMicroSeconds;
    time_t      elapsedSeconds;
    
    //TODO: for some stupid f'n reason on android this seems inconsistent,
    //i think this needs to be switched to a monotick monotonick or something like that clock.
    gettimeofday(&timeCurrent, 0);
    elapsedMicroSeconds = timeCurrent.tv_usec - timeStart.tv_usec;
    elapsedSeconds = timeCurrent.tv_sec - timeStart.tv_sec;
       
    
    //set elapsed miliseconds since init.
    G_MasterTimeCode = (elapsedSeconds * 1000) + (elapsedMicroSeconds / 1000);
    
    
    //fps counter
    frameCounter++;
    
    if (G_MasterTimeCode >= FPSTimer)
    {
        FPS = frameCounter;
        FPSTimer = G_MasterTimeCode + 996; //not a fan of seeing 61FPS, throw some expected frame latency in.
        frameCounter = 0;
    }

    G_DTSeconds = 0.001f * (float)(G_MasterTimeCode - G_LastTimeCode);
    
    
}

float GetDTSeconds()
{
    return G_DTSeconds;
}


static timeval clockStart;
static timeval clockEnd;
//NOTE, anything taking longer than a second may not be reliable
void StartClock()
{
    gettimeofday(&clockStart, 0);
}

unsigned int EndClock()
{
   // gettimeofday(&clockEnd, 0);
    
   // long elapsedSeconds = clockEnd.tv_sec - clockStart.tv_sec;
    //long elapsedMicroSeconds = (clockEnd.tv_usec - clockStart.tv_usec) + (elapsedSeconds * 1000000);
    
    //TODO  finish writing this profiler stuff
    return 0;
}

unsigned int GetFPS()
{
    return FPS;
}

unsigned long GetTimecode()
{
    return G_MasterTimeCode;
}




#include <stdio.h>
void CheckGLErrors()
{
	char buff[256];
	 int err;
	    while ((err = glGetError()) != GL_NO_ERROR) {
	       sprintf(buff, "OpenGL Error: %d", err);
	        LogError(buff);
	    }
}

//just going to let the OS do this...
char CapsModifier(char c)
{
    return 0;
}

//ug, probably only works with latin-1 charset
char ShiftModifier(char c)
{
    if ( c > 64 && c < 91)
        return c; //uppercase A-Z
        
    if ( c > 96 && c < 126 )
    {
        return c - 32;
    }
    else
    {
        switch (c)
        {
            case '`':
                return '~';
            case '-':
                return '_';
            case '=':
                return '+';
            case ',':
                return '<';
            case '.':
                return '>';
            case ';':
                return ':';
            case '\'':
                return '\"';
            case '\\':
                return '|';
            case '[':
                return '{';
            case ']':
                return '}';
            case '/':
                return '?';
            case '1':
                return '!';
            case '2':
                return '@';
            case '3':
                return '#';
            case '4':
                return '$';
            case '5':
                return '%';
            case '6':
                return '^';
            case '7':
                return '&';
            case '8':
                return '*';
            case '9':
                return '(';
            case '0':
                return ')';
            
            default: //dont even shift :[
                return c;
        }
    }
    return 65; // should never get here 'A' 
}
