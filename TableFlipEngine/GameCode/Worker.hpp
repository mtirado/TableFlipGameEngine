/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _WORKER_HPP_
#define _WORKER_HPP_

#include "AIControlled.hpp"

class Worker : public AIControlled
{
private:
public:

    virtual void Init()
    {
        AIControlled::Init();
        
    }
    virtual void Update(float dt)
    {
        //entity->GetCharacterController()->SetForward(90.0f*dt);
        //entity->GetCharacterController()->SetTurn(8.0f*dt);

        t3dOverride.position = transform->position;
        btVector3 v = entity->GetRigidBody()->rigidBody->getLinearVelocity();
        speed = v.length();
        v.normalize();
        //Vector3 vel(v);
        Vector3 vel = transform->position - lastPos;
        speed = vel.MagnitudeSq();
        vel.Normalize();
        vertical = vel.y;
        //vel.y = 0.0f; //no up/down!
        Vector3 fwd = t3dOverride.GetBack() * -1;
        btVector3 right = t3dOverride.GetRight().btVector();
        btVector3 down = t3dOverride.GetUp().btVector() * -1;


//          bool pitchOverride = true;
// 
//         if (fabs(vel.y) > 0.989f)
//             vel.y = 0.0f;
//         else
//             pitchOverride = false;

        vel.Normalize();

       // if (fwd.AngleBetween(vel) > 0.2f)
        t3dOverride.LookAt(transform->position + vel );
//         if (pitchOverride)
//         {
//             if (vertical <= 0.0f)
//                 t3dOverride.RotateRadians(DegToRad(86.0), 0, 0);
//             else
//                 t3dOverride.RotateRadians(DegToRad(-86.0), 0, 0);
//         }

        

        transform->rotation = t3dOverride.rotation;
        transform->Update();
        lastPos = transform->position;
    }

    virtual void Think(float dt)
    {
    }
    
};

#endif
