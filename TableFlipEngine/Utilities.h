/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */

#ifndef UTILITIES_H__
#define UTILITIES_H__


//i guess we should always include this first so we get the platform defines!
#if defined(__ANDROID__) || defined(__WEB_GL__)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #define MOBILE_PLATFORM 
#elif defined(__APPLE__)
#include "GLEW/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#define PC_PLATFORM
#else
    #include <GLEW/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #define PC_PLATFORM
#endif

#if defined(__ANDROID__)
#include "androidEX.h"
#endif


#include <string>
#include "Logger.h"
#include "EngineMath.h"

char ShiftModifier(char c);

//static stringstream ss(std::stringstream::out);
/********************
//       Timer 
*********************/
#include <sys/time.h>
void InitMasterTimecode();
void UpdateMasterTimecode();

//returns milliseconds since init
unsigned long GetTimecode(void);
unsigned int GetFPS();
float GetDTSeconds();

/*******************
*    Profiling
*******************/
void StartClock(); //start profiler clock
unsigned int EndClock(); //return microseconds since clock started

void CheckGLErrors();

/**********************
  Resource Base Class
**********************/
//bass resource behavior
//why is this in here again? i think some kind of circular include problem
class Resource
{
protected:
    int references;
    int priority; //0 will never be removed by garbage collector (default)
    bool loaded;
    std::string FileName;
    

public:
    virtual void Delete() = 0;
    Resource() { loaded = false; references = 0; priority = 0; }
    Resource(int _priority) { priority = _priority; }
    ~Resource() { }
    inline int getReferences() { return references; }
    inline void AddRef()            { references++; }
    inline void RemoveRef()         { references--; } 
    inline int  GetPriority()       { return priority; }
    inline void SetPriority(int p)  { priority = p; }
    inline const char *GetFileName() { return FileName.c_str(); }
    
};
//instead of writing a bunch of files, load from memory
struct ResourceLoadMemInfo
{
    void *mem;
    size_t size;
};


//every entity will have a static sized array of *'s  and the above will be indexes.
//null would mean that component does not exist for a given entity
//class Transform3d;
class Transform3d;
class Entity;

//i guess apple has something typedef'd as Component in Carbon somewhere, included by CoreServices :[
namespace Engine 
{
class Component
{
public:
    Entity *entity;
    Transform3d *transform;
};
}


//singleton macros, (so tired of all the xxxx::GetInstance()'s
#define gInput      Input::GetInstance()
#define gBuilder    Builder::GetInstance()
#define gRenderer   Renderer::GetInstance()
#define gPhysics    Physics::GetInstance()
#define gUIManager  UIManager::GetInstance()


#endif
