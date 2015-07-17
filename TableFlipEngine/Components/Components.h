/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef COMPONENTS_H__
#define COMPONENTS_H__

#include "Utilities.h"

#include "EngineMath.h"
#include "Materials.h"
#include "btBulletDynamicsCommon.h"
#include "Resources.h"
#include "Physics.h"
#include "Scripting.h"
#include "Transform3d.h"
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "AnimationController.h"
#include "Logic.h"
#include "Sprite.h"

//TODO think these should be more OOP,  a base mesh that static, dynamic, skinned, skeletal all derive from.
//but for now lets see how this works out as to not break the rendering code  TODO: remove CMP_MATERIAL its not a damn component
enum ComponentTypes { CMP_TRANSFORM_3D = 0, CMP_MESH, CMP_SKELETAL_MESH, CMP_ANIMATION_CONTROLLER, 
CMP_RIGIDBODY, CMP_COLLISION_SHAPE, CMP_LOGIC, CMP_CHARACTER_CONTROLLER, CMP_PARTICLE_SYSTEM, CMP_MATERIAL, CMP_SPRITE, NUM_COMPONENTS };


//for physics simulation
class RigidBody  : public Engine::Component
{ 
    //pointer to transform so we can update position / orientation
private:
    RigidBody() { is3d = true;}
    
public:
    float linearDrag;
    float angularDrag;
    bool is3d; // this is a HACK?, because of the way the builder constructs rigid bodies that depend on collision shapes
    
    RigidBody(Entity *ent, Transform3d *trans){ entity = ent; transform = trans; rigidBody = 0;}
    ~RigidBody() 
    {
        delete rigidBody;
    }
    // bullet physics rigidbody
    btRigidBody *rigidBody;
    //there is a tick callback in bullet for applying forces more accurately
    //void myTickCallback(btDynamicsWorld *world, btScalar timeStep)
};

enum CollisionShapeType {SHAPE_BOX = 0, SHAPE_CAPSULE, SHAPE_SPHERE, SHAPE_MESH, SHAPE_BOX2D, NUM_COLLISION_SHAPES }; 

class CollisionShape : public Engine::Component
{
private:
    
    bool ownsCO; //is this the collision object owner?
    
protected:
    CollisionShape(){ _is3d = true; shape2dProxy = 0;}
    //Null collision object if this is not a rigidbody.
    //NOTE: rigidbodies are NOT meant to be triggers, not sure how they will react.
    
    bool _is3d;   //z - yay or nay?   
    bool trigger; //a trigger has no response other than a callback function 
    CollisionShapeType type; //not too sure if i will need this.
    
public:
    
    void Init(bool _isTrigger, bool hasRigidBody) 
    { 
        ownsCO = hasRigidBody;
        trigger = _isTrigger; 
        collisionObject = 0;
        if (ownsCO)
        {
            collisionObject = new btCollisionObject;
            collisionObject->setCollisionShape(shape);
        }
        
        if (trigger && ownsCO) //so it doesnt move from collisions
            collisionObject->setCollisionFlags(collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
  
    }
    
    virtual void Make2d() {}; //override makes this a 2d shape
    
    //delete our pointers, make sure we own the collision object...
    ~CollisionShape() {  if (shape2dProxy) delete shape2dProxy;
                        delete shape; if (ownsCO) delete collisionObject;  }
    
    btCollisionShape  *shape;
    btConvexShape  *shape2dProxy; //2d shapes need a 3d proxy, this is the 3d version
    btCollisionObject *collisionObject;
    inline bool isTrigger() { return trigger; }
    inline bool is3d() {return _is3d; }
    inline CollisionShapeType getShapeType() { return type; }
    
};


//TODO array of components !
template <typename T>
class ComponentArray
{
};


#endif
