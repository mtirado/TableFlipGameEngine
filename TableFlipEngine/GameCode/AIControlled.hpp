#ifndef AIControlled_HPP_
#define AIControlled_HPP_
#include <Components.h>
#include <Entity.h>

class AIControlled : public Logic
{
protected:
    Transform3d t3dOverride;
    btVector3 right;
    btVector3 down;

    Vector3 lastPos;
    Vector3 lastVel;

    float speed;
    float vertical;
    float speedmod;
    float force;
public:
    AIControlled() { }
    
    virtual void Init() 
    {
        activeUpdate = true;
        force = 10.0f;
        t3dOverride.position = transform->position;
    }
    virtual void Destroy() {  }
    virtual void OnCollisionEnter ( Entity* other )
    {
       
    }
    virtual void Update(float dt)
    {
        
    }
    
    virtual void Think(float dt)
    {
    }
    
    
};

#endif