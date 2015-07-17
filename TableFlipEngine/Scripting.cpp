/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include "Scripting.h"
#if 0 && defined(PC_PLATFORM) && !defined(__APPLE__)

#include "stdio.h"
#include <iostream>
#include "EngineMath.h"
#include "Utilities.h"
#include "Entity.h"
#include <cstdlib>

Scripting *Scripting::instance = 0;

Scripting::Scripting()
{
   engine = 0;
}
Scripting::~Scripting()
{
   
    
}

void Scripting::InitInstance()
{
    if (!instance)
        instance = new Scripting;
}

void Scripting::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void Scripting::Shutdown()
{
    for (int i = 0; i < scriptControllers.size(); i++)
    {
        delete scriptControllers[i];
    }
    
    for (int i = 0; i < contextPool.size(); i++)
        contextPool[i]->Release();
    
    if (engine)
        engine->Release();
}


void MessageCallback (const asSMessageInfo* msg, void* param)
{
    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";
    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void PrintError(int err, const char *errFunc)
{
    LOGOUTPUT << "Error: " << errFunc << "  - ";
    
        if (err == asINVALID_ARG)
            LOGOUTPUT << "asINVALID_ARG\n";
        else if (err == asNOT_SUPPORTED)
            LOGOUTPUT << "asNOT_SUPPORTED\n";
        else if (err == asWRONG_CALLING_CONV)
            LOGOUTPUT << "asWRONG_CALLING_CONV\n";
        else if (err == asINVALID_DECLARATION)
            LOGOUTPUT << "asINVALID_DECLARATION\n";
        else if (err == asNAME_TAKEN)
            LOGOUTPUT << "asNAME_TAKEN\n";
        else if (err == asALREADY_REGISTERED)
            LOGOUTPUT << "asALREADY_REGISTERED\n";
        else
            LOGOUTPUT << "   error code - " << err << "\n";        
        LogError();
}

// Print the script string to the standard output stream
void print(std::string &msg)
{
    printf("%s", msg.c_str());
}


bool Scripting::Init()
{
    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    int r;
    r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
    if (r < 0){ PrintError(r, "SetMessageCallback"); return false; }
    
    //engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
    
    //HACK  not really a hack, but this could be dangerous, if you're
    //searching for hacks, this might be of interest...
    engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
    
    //register string class (roll my own eventually?)
    RegisterStdString(engine);
    //generic handle  type called 'ref'
    RegisterScriptHandle(engine);
    
    if (!RegisterMathRoutines())
    {
        LogError("Error registering scripting Math Functions");
        return false;
    }
    
    
    //register basic script class Entity
    
        //register type, and reference management functions
        r = engine->RegisterObjectType("Entity", 0, asOBJ_REF); 
        if (r < 0) { PrintError(r, "RegisterObjectType()"); return false; }
        
        r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_ADDREF, "void f()", asMETHOD(Entity, AddRef), asCALL_THISCALL); 
        if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
       
        r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_RELEASE, "void f()", asMETHOD(Entity, Release), asCALL_THISCALL); 
        if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
    
        //register all functions this script will be able to see  start with xyz, change to vector type, or direct variable access
        r = engine->RegisterObjectMethod("Entity", "void Kill()", asMETHOD(Entity, Kill), asCALL_THISCALL); 
        if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
    
    if (!RegisterComponents())
    {
        LogError("Error registering scripting Component Functions");
        return false;
    }
        
        
      /*  r = engine->RegisterObjectMethod("Entity", "void SetVelocity(float z)", asMETHOD(Entity, SetVelocity), asCALL_THISCALL); 
        if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
        
            
            
            r = engine->RegisterObjectMethod("Entity", "void RotateDegrees(float pitch, float yaw, float roll)", 
                                             asMETHOD(Entity, RotateDegrees), asCALL_THISCALL); 
            if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
            
            r = engine->RegisterObjectMethod("Entity", "void RotateRadians(float pitch, float yaw, float roll)", 
                                             asMETHOD(Entity, RotateRadians), asCALL_THISCALL); 
            if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
            
            r = engine->RegisterObjectProperty("Entity", "Vector3 Position", asOFFSET(Entity, Position)); 
            if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
            r = engine->RegisterObjectProperty("Entity", "Vector3 velocity", asOFFSET(Entity, velocity)); 
            if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
            r = engine->RegisterObjectProperty("Entity", "Vector3 acceleration", asOFFSET(Entity, acceleration)); 
            if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
            r = engine->RegisterObjectProperty("Entity", "Quaternion _rotation", asOFFSET(Entity, Position)); 
            if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
            */
              
          /*  r = engine->RegisterObjectMethod("Entity", "Vector3 GetForward()", 
                                    asMETHODPR(Entity, GetForward, (void), Vector3), asCALL_THISCALL);
            r = engine->RegisterObjectMethod("Entity", "Vector3 GetUp()", 
                                    asMETHODPR(Entity, GetUp, (void), Vector3), asCALL_THISCALL);
            r = engine->RegisterObjectMethod("Entity", "Vector3 GetRight()", 
                                    asMETHODPR(Entity, GetRight, (void), Vector3), asCALL_THISCALL);
            r = engine->RegisterObjectMethod("Entity", "void AddForwardForce(float f)", 
                                    asMETHODPR(Entity, AddForwardForce, (float), void), asCALL_THISCALL);
            r = engine->RegisterObjectMethod("Entity", "void BrakeForce(float f)", 
                                    asMETHODPR(Entity, BrakeForce, (float), void), asCALL_THISCALL);
    
            
           */
    
    
    
    
    
        //register messaging function, 2 soup cans and a string.
        r = engine->RegisterObjectMethod("Entity", "void SendMessage(ref msg, const Entity @+ to)", asMETHOD(Entity, SendMessage), asCALL_THISCALL); 
        if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
        
        // The game engine will determine the class that represents the controller
        // by checking if the class implements the IController interface. No methods
        // are registered for this interface, as the script shouldn't be required to
        // implement the methods. This will allow the game engine to avoid calling
        // methods that doesn't do anything, thus improving performance.
        r = engine->RegisterInterface("IController"); 
        if (r < 0) { PrintError(r, "RegisterInterface()"); return false; }
        
    
    //register function for script to call
    r = engine->RegisterGlobalFunction("uint GetTimecode()", asFUNCTIONPR(GetTimecode, (void), unsigned long), asCALL_CDECL);
    if (r < 0){ PrintError(r, "RegisterGlobalFunction"); return false; }
    
    
   
   

    return true;
    
}


Scripting::ScriptController* Scripting::GetScriptController (const std::string& script)
{
    int r;

    // Find the cached controller
    for( unsigned int n = 0; n < scriptControllers.size(); n++ )
    {
        //might need to enforce this, somehow. if its not already
        if( scriptControllers[n]->className == script )
            return scriptControllers[n];
    }

    // No controller, check if the script has already been loaded
    asIScriptModule *mod = engine->GetModule(script.c_str(), asGM_ONLY_IF_EXISTS);
    if( mod )
    {
        // We've already attempted loading the script before, but there is no controller
        PrintError(0, "Loading has failed, No Controller Exists");
        return 0;
    }

    // Compile the script into the module
    CScriptBuilder builder;
    //build script
    r = builder.StartNewModule(engine, script.c_str());
    if (r < 0) { PrintError(r, "cannot build new module, does the file exist?\n"); return 0; }
    

    // If the script file doesn't exist, then there is no script controller for this type
   /* FILE *f;
    if( (f = fopen((script + ".as").c_str(), "r")) == 0 )
        return 0;
    fclose(f);*/

    // Let the builder load the script, and do the necessary pre-processing (include files, etc)
    r = builder.AddSectionFromFile(("Data/Scripts/" + script + ".as").c_str());
    if (r < 0) { PrintError(r, "cannot build file\n"); return 0; }
    
    r = builder.BuildModule();
    if (r < 0) { PrintError(r, "Error compiling script\n"); return 0;}
    
    // Cache the functions and methods that will be used
    ScriptController *ctrl = new ScriptController;
    scriptControllers.push_back(ctrl);
    ctrl->className = script;

    // Find the class that implements the IController interface
    mod = engine->GetModule(script.c_str(), asGM_ONLY_IF_EXISTS);
    asIObjectType *type = 0;
    int tc = mod->GetObjectTypeCount();
    for( int n = 0; n < tc; n++ )
    {
        bool found = false;
        type = mod->GetObjectTypeByIndex(n);
        int ic = type->GetInterfaceCount();
        for( int i = 0; i < ic; i++ )
        {
            if( strcmp(type->GetInterface(i)->GetName(), "IController") == 0 )
            {
                    found = true;
                    break;
            }
        }

        if( found == true )
        {
            ctrl->type = type;
            break;
        }
    }

    if( ctrl->type == 0 )
    {
        std::cout << "Couldn't find the controller class for the type '" << script << "'" << std::endl;
        scriptControllers.pop_back();
        delete ctrl;
        return 0;
    }

    // Find the factory function
    // The game engine will pass in the owning CGameObj to the controller for storage
    std::string s = std::string(type->GetName()) + "@ " + std::string(type->GetName()) + "(Entity @)";
    ctrl->factoryFunc = type->GetFactoryByDecl(s.c_str());
    if( ctrl->factoryFunc == 0 )
    {
        std::cout << "Couldn't find the appropriate factory for the type '" << script << "'" << std::endl;
        scriptControllers.pop_back();
        delete ctrl;
        return 0;
    }

    // Find the optional event handlers
    ctrl->updateFunc         = type->GetMethodByDecl("void Update()");
    ctrl->messageFunc        = type->GetMethodByDecl("void ReceiveMessage(ref @msg, const Entity @sender)");
    ctrl->onCollisionEnter   = type->GetMethodByDecl("void OnCollisionEnter(Entity @other)");
    ctrl->onCollisionStay    = type->GetMethodByDecl("void OnCollisionStay(Entity @other)");
    ctrl->onCollisionLeave   = type->GetMethodByDecl("void OnCollisionLeave(Entity @other)");

    //TODO: more robust notification of script errors pleeeze!
    if (!ctrl->updateFunc || !ctrl->messageFunc)
    {
        PrintError(0, "Script does not implement required methods!");
        scriptControllers.pop_back();
        delete ctrl;
        return 0;
    }
    
    
    // Add the cache as user data to the type for quick access
    type->SetUserData(ctrl);

    return ctrl;
}

//a script instance of an entity
asIScriptObject *Scripting::CreateScriptObject(const std::string &script, Entity *worldObj)
{
        int r;
        asIScriptObject *obj = 0;

        ScriptController *ctrl = GetScriptController(script);
        if( ctrl == 0 ) {PrintError(0, "Could not Load Script Controller Check Errors!\n"); return 0;}
                
        // Create the object using the factory function
        asIScriptContext *ctx = PrepareContextFromPool(ctrl->factoryFunc);

        // Pass the object pointer to the script function. With this call the 
        // context will automatically increase the reference count for the object.
        ctx->SetArgObject(0, worldObj);

        // Make the call and take care of any errors that may happen
        r = ExecuteCall(ctx);
        if( r == asEXECUTION_FINISHED )
        {
                // Get the newly created object
                obj = *((asIScriptObject**)ctx->GetAddressOfReturnValue());

                // Since a reference will be kept to this object 
                // it is necessary to increase the ref count
                obj->AddRef();
        }
        else
        {
            ReturnContextToPool(ctx);
            PrintError(r, "Execute Call Failed in CreateScriptObject\n");
            return 0;
        }
        // Return the context to the pool so it can be reused
        ReturnContextToPool(ctx);
        
        return obj;
}

int Scripting::ExecuteCall(asIScriptContext *ctx)
{
        int r = ctx->Execute();
        if( r != asEXECUTION_FINISHED )
        {
                if( r == asEXECUTION_EXCEPTION )
                {
                        std::cout << "Exception: " << ctx->GetExceptionString() << std::endl;
                        std::cout << "Function: " << ctx->GetExceptionFunction()->GetDeclaration() << std::endl;
                        std::cout << "Line: " << ctx->GetExceptionLineNumber() << std::endl;

                        // It is possible to print more information about the location of the 
                        // exception, for example the call stack, values of variables, etc if 
                        // that is of interest.
                }
        }

        return r;
}

//TODO: thismight not be good to be creating contexts at runtime, prefill this on Init()
asIScriptContext *Scripting::PrepareContextFromPool(asIScriptFunction *func)
{
        asIScriptContext *ctx = 0;
        if( contextPool.size() )
        {
                ctx = *contextPool.rbegin();
                contextPool.pop_back();
        }
        else
                ctx = engine->CreateContext();

        int r = ctx->Prepare(func); 
        if (r < 0) PrintError(r, "ctx->Prepare\n"); //this would be a major failure

        return ctx;
}

void Scripting::ReturnContextToPool(asIScriptContext *ctx)
{
        contextPool.push_back(ctx);

        // Unprepare the context to free any objects that might be held
        // as we don't know when the context will be used again.
        ctx->Unprepare();
}


//helper functions
void Scripting::CallReceiveMessage (asIScriptObject* receiver, CScriptHandle& msg, Entity* caller)
{
    //script controller was passed as user data to make this process fast and painless
    ScriptController *ctrl = reinterpret_cast<ScriptController *>(receiver->GetObjectType()->GetUserData());
    std::cout << "Sending Message To Script\n";
    if (ctrl->messageFunc == 0)
        return;
     
    asIScriptContext *ctx = PrepareContextFromPool(ctrl->messageFunc);
    ctx->SetObject(receiver);
    ctx->SetArgObject(0, &msg);
    ctx->SetArgObject(1, caller);
    ExecuteCall(ctx);
    ReturnContextToPool(ctx);
             
}


void Scripting::CallUpdate (asIScriptObject* receiver)
{
   
     //script controller was passed as user data to make this process fast and painless
    ScriptController *ctrl = reinterpret_cast<ScriptController *>(receiver->GetObjectType()->GetUserData());
    
    if (ctrl->updateFunc == 0)
        return;
    
    asIScriptContext *ctx = PrepareContextFromPool(ctrl->updateFunc);
    ctx->SetObject(receiver);
    ExecuteCall(ctx);
    ReturnContextToPool(ctx);
   
}
//TODO theres got to be a way to automate creating these interfaces so we can add any function
//from a game class, instead of hardcoding them as we go  in the engine. figure it out later!?
//just going to consolidate lines to save space at this point in time...
void Scripting::CallOnCollisionStay(asIScriptObject* receiver, Entity* other)
{
    if (receiver == 0) return; 
    ScriptController *ctrl = reinterpret_cast<ScriptController *>(receiver->GetObjectType()->GetUserData()); 
    if (ctrl->onCollisionStay == 0) return;
    asIScriptContext *ctx = PrepareContextFromPool(ctrl->onCollisionStay);
    ctx->SetObject(receiver); ctx->SetArgObject(0, other); ExecuteCall(ctx);  ReturnContextToPool(ctx);
}

void Scripting::CallOnCollisionEnter(asIScriptObject* receiver, Entity* other)
{
    if (receiver == 0) return;
    ScriptController *ctrl = reinterpret_cast<ScriptController *>(receiver->GetObjectType()->GetUserData()); 
    if (ctrl->onCollisionEnter == 0) return;
    asIScriptContext *ctx = PrepareContextFromPool(ctrl->onCollisionEnter);
    ctx->SetObject(receiver); ctx->SetArgObject(0, other); ExecuteCall(ctx);  ReturnContextToPool(ctx);
}

void Scripting::CallOnCollisionLeave(asIScriptObject* receiver, Entity* other)
{
    if (receiver == 0) return;
    ScriptController *ctrl = reinterpret_cast<ScriptController *>(receiver->GetObjectType()->GetUserData()); 
    if (ctrl->onCollisionLeave == 0) return;
    asIScriptContext *ctx = PrepareContextFromPool(ctrl->onCollisionLeave);
    ctx->SetObject(receiver); ctx->SetArgObject(0, other); ExecuteCall(ctx);  ReturnContextToPool(ctx);
}

//Vector Helpers
static void Vector3DefaultConstructor(Vector3 *self)
{
    new(self) Vector3;
}
static void Vector3CopyConstructor(const Vector3 &other, Vector3 *self)
{
    new(self) Vector3(other);
}
static void Vector3InitConstructor(float x, float y, float z, Vector3 *self)
{
    new(self) Vector3(x,y,z);
}

bool Scripting::RegisterMathRoutines()
{
//register the vector class    asOBJ_NOCOUNT  = references only! removes memory management overhead
int r;
    r = engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); 
    if (r < 0) { PrintError(r, "RegisterObjectType()"); return false; }

    //register properties
    r = engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(Vector3, x)); 
    if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
    r = engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(Vector3, y)); 
    if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
    r = engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(Vector3, z)); 
    if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
    
    
    r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,  "void f()", asFUNCTION(Vector3DefaultConstructor), asCALL_CDECL_OBJLAST);
    if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
    r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,  "void f(const Vector3 &in)",asFUNCTION(Vector3CopyConstructor), asCALL_CDECL_OBJLAST);
    if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
    r = engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,  "void f(float, float, float)", asFUNCTION(Vector3InitConstructor), asCALL_CDECL_OBJLAST);
    if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }

    //register operators.
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, operator+, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, operator-, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, operator*, (const float &) const, Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "Vector3 &opAssign(const Vector3 &in)", 
                                    asMETHODPR(Vector3, operator=, (const Vector3 &), Vector3 &), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "void Normalize()", 
                                    asMETHODPR(Vector3, Normalize, (void), void), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "Vector3 Cross(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, Cross, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "float Dot(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, Dot, (const Vector3 &) const, float), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "float AngleBetween(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, AngleBetween, (const Vector3 &) const, float), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "float DegreesBetween(const Vector3 &in) const", 
                                    asMETHODPR(Vector3, DegreesBetween, (const Vector3 &) const, float), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    
    r = engine->RegisterObjectMethod("Vector3", "float MagnitudeSq() const", 
                                    asMETHODPR(Vector3, MagnitudeSq, (void) const, float), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    r = engine->RegisterObjectMethod("Vector3", "float Magnitude() const", 
                                    asMETHODPR(Vector3, Magnitude, (void) const, float), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod"); return false; }
    
    
    
        
        
        
        //register the Quaternion class
    r = engine->RegisterObjectType("Quaternion", 0, asOBJ_REF | asOBJ_NOCOUNT ); 
    if (r < 0) { PrintError(r, "RegisterObjectType()"); return false; }
    
    
    //register floating point math
    r = engine->RegisterGlobalFunction("float abs(float)", asFUNCTIONPR(fabsf, (float), float), asCALL_CDECL);
    if (r < 0) { PrintError(r, "RegisterGlobalFunction()"); return false; }
    r = engine->RegisterGlobalFunction("int rand()", asFUNCTIONPR(rand, (void), int), asCALL_CDECL);
    if (r < 0) { PrintError(r, "RegisterGlobalFunction()"); return false; }
    
        
    
    return true;
}


bool Scripting::RegisterComponents()
{
    int r;
    //register these as single-reference type  eg: asOBJ_REF | asOBJ_NOHANDLE
    //they cannot be decalred in script, but have no overhead of reference counting
    //------------------------------------------------------------------
    //          TRANSFORM3D 
    //------------------------------------------------------------------
    
    r = engine->RegisterObjectType("Transform3d", 0, asOBJ_REF | asOBJ_NOHANDLE); 
    if (r < 0) { PrintError(r, "RegisterObjectType()"); return false; }
    
    r = engine->RegisterObjectProperty("Transform3d", "Vector3 position", asOFFSET(Transform3d, position)); 
    if (r < 0) { PrintError(r, "RegisterObjectProperty()"); return false; }
    
    
    //NOTE angelscript may need its own accessors that pass by value and not reference??
   /* r = engine->RegisterObjectMethod("Transform3d", "Vector3 GetForward()", 
                                    asMETHODPR(Transform3d, GetBack, (void), Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    r = engine->RegisterObjectMethod("Transform3d", "Vector3 GetUp()", 
                                    asMETHODPR(Transform3d, GetUp, (void), Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    r = engine->RegisterObjectMethod("Transform3d", "Vector3 GetRight()", 
                                    asMETHODPR(Transform3d, GetRight, (void), Vector3), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
    */
    //------------------------------------------------------------------
    //          CHARACTER CONTROLLER
    //------------------------------------------------------------------
    
       
    r = engine->RegisterObjectType("CharacterController", 0, asOBJ_REF | asOBJ_NOHANDLE); 
    if (r < 0) { PrintError(r, "RegisterObjectType()"); return false; }
    
     r = engine->RegisterObjectMethod("Entity", "CharacterController &GetCharacterController()",
                                    asMETHOD(Entity, GetCharacterController), asCALL_THISCALL); 
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
    r = engine->RegisterObjectMethod("CharacterController", "void SetForward(float forward)", 
                                    asMETHODPR(CharacterController, SetForward, (float), void), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
    r = engine->RegisterObjectMethod("CharacterController", "void SetTurn(float turn)", 
                                    asMETHODPR(CharacterController, SetTurn, (float), void), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
   
    r = engine->RegisterObjectMethod("CharacterController", "void SetStrafe(float strafe)", 
                                    asMETHODPR(CharacterController, SetStrafe, (float), void), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
     r = engine->RegisterObjectMethod("CharacterController", "void Warp(Vector3 pos)", 
                                    asMETHODPR(CharacterController, Warp, (Vector3), void), asCALL_THISCALL);
    if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }
    
    
    //void Warp(Vector3 pos)
    
    
    
     // void SetForward(float _forward) { forward = _forward; }
   // void SetTurn(float _turn) { turn = _turn; }
   // void SetStrafe(float _strafe) { strafe = _strafe; }
     
   /* r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_ADDREF, "void f()", asMETHOD(Entity, AddRef), asCALL_THISCALL); 
    if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
    
    r = engine->RegisterObjectBehaviour("Entity", asBEHAVE_RELEASE, "void f()", asMETHOD(Entity, Release), asCALL_THISCALL); 
    if (r < 0) { PrintError(r, "RegisterObjectBehaviour()"); return false; }
*/
   
    //register all functions this script will be able to see  start with xyz, change to vector type, or direct variable access
  //  r = engine->RegisterObjectMethod("Entity", "void Kill()", asMETHOD(Entity, Kill), asCALL_THISCALL); 
  //  if (r < 0) { PrintError(r, "RegisterObjectMethod()"); return false; }

    return true;
}
#endif
