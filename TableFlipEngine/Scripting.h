/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef SCRIPTING_H__
#define SCRIPTING_H__
class Scripting{};
#include "Utilities.h"
#if 0 && defined(PC_PLATFORM) && !defined(__APPLE__)
#include <string>
#include <vector>

#include <AngelScript/angelscript.h>
#include <AngelScript/scriptbuilder/scriptbuilder.h>
#include <AngelScript/scriptstdstring/scriptstdstring.h>
#include <AngelScript/scripthandle/scripthandle.h>

class Entity;


class Scripting
{
private:
    
    //disable functions for singleton use
    Scripting();
    ~Scripting();
    Scripting(const Scripting &) {}
    Scripting &operator = (const Scripting &) { return *this; }
    
    //the singletons instance.
    static Scripting *instance;
    
    asIScriptEngine *engine;
    //compiles the scripts
    CScriptBuilder builder;

public:

    
    //hacky type first implementation, iterate, iterate iterate!
    asIScriptObject *CreateScriptObject(const std::string &type, Entity *worldObj);
    asIScriptContext *PrepareContextFromPool(asIScriptFunction *func);
    void ReturnContextToPool(asIScriptContext *ctx);
    
    //information about our script object   
    struct ScriptController
    {
        ScriptController() : type(0), factoryFunc(0), updateFunc(0), messageFunc(0),
                            onCollisionEnter(0), onCollisionStay(0), onCollisionLeave(0) {}
        std::string className;
        asIObjectType *type;
        asIScriptFunction *factoryFunc;
        asIScriptFunction *updateFunc;
        asIScriptFunction *messageFunc;
        
        //to avoid crazy branching in messageFunc, add any frequent function calls here (dont forge to initialize pointers)
        asIScriptFunction *onCollisionEnter;
        asIScriptFunction *onCollisionStay;
        asIScriptFunction *onCollisionLeave;
        
    };
    
    ScriptController *GetScriptController(const std::string &script);
    int ExecuteCall(asIScriptContext *ctx);
    
    //contexts are apparantly painful to allocate, so store a pool of them
    std::vector<asIScriptContext *> contextPool;
    //script function pointers, so we dont need to look them up all the time.
    std::vector<ScriptController *> scriptControllers;
    
    //helper functions
    void CallReceiveMessage(asIScriptObject *receiver, CScriptHandle &msg, Entity *caller);
    void CallUpdate(asIScriptObject *receiver);
    
    //other is the other entity involved in collision
    void CallOnCollisionEnter(asIScriptObject* receiver, Entity *other);
    void CallOnCollisionStay(asIScriptObject* receiver, Entity *other);
    void CallOnCollisionLeave(asIScriptObject* receiver, Entity *other);
    
    
    bool RegisterMathRoutines();
    bool RegisterComponents();

    
    //inlines
    inline static Scripting *GetInstance() { return instance; }
    
    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();
    
    bool Init();
    void Shutdown();
};
#endif


#endif
