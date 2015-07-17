/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#ifndef _ENTITY_H__
#define _ENTITY_H__

#include "EngineMath.h"
#include "BoundingVolumes.h"

//#include <angelscript.h>
//#include <scripthandle/scripthandle.h>
class ParticleSystem;
class BlueprintResource;
#include "CharacterController.h"
#include "Components.h"
#include "CollisionShapes.h"
#if defined(PC_PLATFORM)
//#include <angelscript.h>
#include "Scripting.h"
#include "Builder.h"
#include <SLList.h>
#endif

//component get is ugly, hopefully this is nicer?
#define CTransform           GetTransform()
#define CMesh                GetMesh()
#define CRigidBody           GetRigidBody(
#define CSkeletalMesh        GetSkeletalMesh()
#define CAnimationController GetAnimationController()
#define CCharacterController GetCharacterController()
#define CCollisionShape      GetCollisionShape()
#define CLogic               GetLogic()
#define CSprite              GetSprite()
#define CParticleSystem      GetParticleSystem()
#define CBlueprintResource   GetBlueprint()

enum EntityType { ETYPE_DEFAULT = 0, ETYPE_GENERIC, ETYPE_TRIGGER,
                //user types go here
                  ETYPE_HOMEBASE, ETYPE_TOWER, ETYPE_AGENCY,
NUM_ETYPES };


class Entity
{
    friend class Transform3d; // might as well just put transform in entity at this point?
private:
    
    //0 is not allowed. TODO implement unique ID's
    unsigned int id;
    EntityType type;
    bool _is3d;
    
    //so when we try to access component data we can just jump right to it, instead of iterating through
    //a list of instantiated components, null would mean that component is not attached.
    void *components[NUM_COMPONENTS];
    
    Entity *parent;
    //children share transforms+offset, rigid bodies all share mass, and average center of gravity
    SLList<Entity *> children;

    //returns the absolute offset of all parent transforms
   void UpdateChildTransforms();
    
    
protected:    
   // call destroy and release no manual deletion! (angelscript holds references to entities)
     ~Entity();
     
    //for scripting reference counting.
    //SCRIPTING HAS BEEN REMOVED!!!  just leaving it for now incase i go back to angel script
    int refCount;


    //physics callbacks
    friend class Physics;
    //NOTE: if inherited classes do not call this, scripting collision functions will not be called       //TODO reinstate scripting calls - should be called from logic class, not entity.
    inline void OnCollisionEnter(Entity *other) { if (GetLogic()) GetLogic()->OnCollisionEnter(other); /*Scripting::GetInstance()->CallOnCollisionEnter(scriptObject, other);*/ }
    inline void OnCollisionStay(Entity *other)  { if (GetLogic()) GetLogic()->OnCollisionStay(other);  /*Scripting::GetInstance()->CallOnCollisionStay(scriptObject, other);*/ }
    inline void OnCollisionLeave(Entity *other) { if (GetLogic()) GetLogic()->OnCollisionLeave(other); /*Scripting::GetInstance()->CallOnCollisionLeave(scriptObject, other); */}
    
    //angelscript functions
#if defined(PC_PLATFORM) && !defined(__APPLE__)
    //asIScriptObject *scriptObject;
#endif
    BlueprintResource *blueprint; //of craeted from a blueprint, this entity technically is referencing it, removeref on destroy
    
public:
         
    std::string name;
    //associated with a world file?
    std::string worldTag; //this should probably be an ID
    Entity();

   // BVSphere sphereCollider;//temporary...  for octree
   BVHOctreeNode *octreeNode; //the node we are inside of 
    
    virtual void Update() {}
        
    //attach a NEW component, with any data we may need associated
    //return false if component cannot be attached
    bool AttachComponent(ComponentTypes type, void *data = 0);
    void AttachEntity(Entity *child, Vector3 offset);
    
    //TODO optimize?  inline at least (if not using scripts)
    Transform3d *GetTransform() { return (Transform3d *)components[CMP_TRANSFORM_3D]; }
    Mesh *GetMesh() { return (Mesh *)components[CMP_MESH]; }
    RigidBody *GetRigidBody() { return (RigidBody *)components[CMP_RIGIDBODY]; }
    SkeletalMesh *GetSkeletalMesh() { return (SkeletalMesh *)components[CMP_SKELETAL_MESH]; }
    AnimationController *GetAnimationController() { return (AnimationController *)components[CMP_ANIMATION_CONTROLLER]; }
    CharacterController *GetCharacterController() { return (CharacterController *)components[CMP_CHARACTER_CONTROLLER]; }
    CollisionShape *GetCollisionShape() { return (CollisionShape *)components[CMP_COLLISION_SHAPE]; }
    Logic *GetLogic() { return (Logic *)components[CMP_LOGIC]; }
    Sprite *GetSprite() { return (Sprite *)components[CMP_SPRITE]; }   
    ParticleSystem *GetParticleSystem() { return (ParticleSystem *)components[CMP_PARTICLE_SYSTEM]; }
    const BlueprintResource *GetBlueprint() { return blueprint; }

    //DONT CALL THIS UNLESS YOU KNOW WTF YOU'RE DOING!
    void SetBlueprint(BlueprintResource *bp) { blueprint = bp; }
    
    const EntityType &getType() const { return type; }
    void setType(EntityType t)
    {
        if (t < 0 || t >= NUM_ETYPES)
            type = ETYPE_DEFAULT;
        else
            type = t;
    }
       
    inline bool is3d() { return _is3d; }
       
    //there is only one intended use for this function, if you dont know what it is dont mess with it.
#if defined(PC_PLATFORM) && !defined(__APPLE__)
    //inline asIScriptObject *GetScriptCtrlAddress() { return scriptObject; }
#endif
    //these are needed so the script engine can hold on to references, 
    //even if the object is "deleted" in our engine
    int AddRef() { return ++refCount; }
    
    //call this instead of deleting entities manually
    int Release() 
    { 
        #if defined(PC_PLATFORM) && !defined(__APPLE__)
        //if (scriptObject)       scriptObject->Release();
        #endif
        blueprint->RemoveRef();
        if (--refCount <= 0)    { delete this; return 0; } 
        return refCount;       
    }
    
    
    //used by application when we want to 'delete' this object
    //releasing scriptobject SHOULD hopefully drop refcount to 0, otherwise another script may be referencing this entity
    void DestroyAndRelease() { Release(); }
    
    //loads the script object
    bool LoadScript(const char *filename) 
    { 
#if defined(PC_PLATFORM) && !defined(__APPLE__)
        //scriptObject = Scripting::GetInstance()->CreateScriptObject(filename, this); 
       // return scriptObject;
#endif
        return false;
    }
    
    //functions we will expose to the script
    //TODO for final build all of this scripting stuff may be taken out, and scripts compiled
    //directly to c++ code.
#if defined(PC_PLATFORM) && !defined(__APPLE__)
    /*void SendMessage(CScriptHandle msg, Entity *sendTo)
    {
        
        if (sendTo && sendTo->scriptObject)
            Scripting::GetInstance()->CallReceiveMessage(sendTo->scriptObject, msg, this);
       
    }*/
#endif
    void Kill() //destroys this object (from script)
    {
        LogInfo("Entity Kill() recieved");
    }
};

#endif
