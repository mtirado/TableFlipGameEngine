/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef CHARACTER_CONTROLLER_H__
#define CHARACTER_CONTROLLER_H__

#include "btBulletDynamicsCommon.h"
//#include <BulletDynamics/Character/btCharacterControllerInterface.h>
//#include "btCharacterControllerInterface.h"
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
class btCollisionShape;
class btRigidBody;
class btCollisionWorld;
#include "Components.h"

///DynamicCharacterController is obsolete/unsupported at the moment
class CharacterController : public Engine::Component, public btKinematicCharacterController
{
private:
    CharacterController();
    
    float forwardSpeed; //current speed
    float strafeSpeed;
    float direction; 
    
    float dampener;
    
    btVector3 velocity;
    float forward, turn, strafe;
    
    //correction angles for legs on slopes
    float rLegAngle, lLegAngle;
   
public:
    //do not fux with these, unless you must.
    btPairCachingGhostObject *ghostObject;
    btConvexShape *capsule;

    //stuff you can fux with
    float walkSpeed;
    float runSpeed;
    float turnWalkSpeed; // turning speeds
    float turnRunSpeed;  
    float s_yOffset; //static offset from file
    float s_zOffset;
    float yOffset; //offset of collision object y axis.
    float zOffset; //program controlled offset
    
    //negative for backwards
    //void Move(float forward, float turn, float strafe);
    
    //NOTE removed inline for scripting purposes
    void SetForward(float _forward) { forward = _forward; }
    void SetTurn(float _turn) { turn = _turn; }
    void SetStrafe(float _strafe) { strafe = _strafe; }
    bool isRunning() { return (forwardSpeed > walkSpeed); }
    void Warp(Vector3 pos) { warp(btVector3(pos.x, pos.y, pos.z)); }

    btCollisionObject *GetCollisionObject() { return ghostObject; }

    
    //override
    void playerStep(btCollisionWorld* collisionWorld, btScalar dt);
    void preStep (btCollisionWorld* collisionWorld);
    void stepDown ( btCollisionWorld* collisionWorld, btScalar dt);
    void stepUp (btCollisionWorld* world);
    void stepForwardAndStrafe(btCollisionWorld* collisionWorld, const btVector3& walkMove);
    void debugDraw(btIDebugDraw* debugDrawer);
    bool recoverFromPenetration ( btCollisionWorld* collisionWorld);
    ///btActionInterface interface
    virtual void updateAction( btCollisionWorld* collisionWorld,btScalar deltaTime)
    {
            preStep ( collisionWorld);
            playerStep (collisionWorld, deltaTime);
    }
    
    CharacterController(btPairCachingGhostObject* _ghostObject, btConvexShape* convexShape, btScalar stepHeight, int upAxis = 1);
    ~CharacterController();
};
#endif
