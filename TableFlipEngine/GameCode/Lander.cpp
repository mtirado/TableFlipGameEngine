/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "ParticleSystem.h"
#include "Lander.h"

unsigned int logDelay = 1200;
unsigned int lastLog = 0;

void Lander::Init()
{
    //engine call update pls!
    activeUpdate = true;
    instrumentation.Init(this);
    
    entity->GetRigidBody()->rigidBody->getCollisionShape()->setMargin(.13);
    //use CCD?
    entity->GetRigidBody()->rigidBody->setCcdMotionThreshold(0.000001);
    entity->GetRigidBody()->rigidBody->setCcdSweptSphereRadius(0.2);

    //entity->GetRigidBody()->rigidBody->setCollisionFlags(entity->GetRigidBody()->rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
   // entity->GetRigidBody()->rigidBody->setFriction(1.0f);
}
void Lander::Update(float dt)
{
    Transform3d *tr = entity->GetTransform();
    btRigidBody *rb = entity->GetRigidBody()->rigidBody;
    rb->activate(); //should probably just tell RB to never sleep...

    float altitude = tr->position.y;
    if (altitude <= 0.0)
        altitude = 0.01;
    float altVal = altitude / 11000.0;

    //calculate drag force
    entity->GetRigidBody()->rigidBody->setDamping(0, entity->GetRigidBody()->angularDrag);
    float A = 1.0;
    float density = Lerp(1.0, 0.2, altVal);//1.2;
    float Cd = entity->GetRigidBody()->linearDrag;
    float v = fabs(entity->GetRigidBody()->rigidBody->getLinearVelocity().y());
    float drag = Cd * density * (v*v);
    //apply earth atmospheric drag
    //rb->applyForce(entity->GetRigidBody()->rigidBody->getLinearVelocity().normalized() * -drag, btVector3(0.0,0.0,0.0));


    //handle thrust
    float fwd=0.0;
    float right=0.0;
    float roll=0.0;
    if (gInput->GetKeyDown(KB_UP))
        fwd = 1.0;
    if (gInput->GetKeyDown(KB_DOWN))
        fwd = -1.0;
    if (gInput->GetKeyDown(KB_RIGHT))
        right = -1.0;
    if (gInput->GetKeyDown(KB_LEFT))
        right = 1.0;
    if (gInput->GetKeyDown(KB_S))
        roll = 1.0;
    if (gInput->GetKeyDown(KB_F))
        roll = -1.0;

    if (gInput->GetKeyDown(KB_Q))
        roll *= -1.0;

    if (roll != 0.0)
    {
        Vector3 rollTorque = tr->GetUp() * (2800.33f * roll);
        rb->applyTorqueImpulse(rollTorque.btVector());
    }
    if (fwd != 0.0)
    {
        Vector3 pitchTorque = tr->GetRight() * (-2800.0 * fwd);
        rb->applyTorqueImpulse(pitchTorque.btVector());
    }
    if (right != 0.0)
    {
        Vector3 yawTorque = tr->GetBack() * (2800.0 * right);
        rb->applyTorqueImpulse(yawTorque.btVector());
    }

    if (gInput->GetKeyDown(KB_SPACE))
    {
        entity->GetParticleSystem()->Emit(42000);

        Vector3 lateral(right, 0.0f, fwd);

        Vector3 pitch = tr->GetRight() * lateral.x;
        Vector3 yaw = tr->GetBack() * lateral.z;
        //neutral lift = mass * gravity
        Vector3 force = (tr->GetUp() + (yaw + pitch)/6.31 ) * 731000.3210;
        rb->applyForce(force.btVector(), tr->GetUp().btVector()*-1.0 );
    }
    else
    {
        entity->GetParticleSystem()->Stop();
    }

    //update hud
    instrumentation.Update();

    //log stuff
//     if (lastLog + logDelay < GetTimecode())
//     {
//         LOGOUTPUT << "pitchRad: " << pitchRad << "  pitch : " << transPitch << "  roll : " << trRoll;
//         LogInfo();
//         LOGOUTPUT << "altitude: " << altitude << "   density: " << density;
//         LogInfo();
//         LOGOUTPUT << "drag : " << drag << "   v: " << v;
//         LogInfo();
// 
//         lastLog = GetTimecode();
//     }
}


void Lander::Destroy()
{
    instrumentation.Destroy();
}
