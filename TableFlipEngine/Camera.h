/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _CAMERA_H__
#define _CAMERA_H__

#include "EngineMath.h"
#include "Entity.h"
#include <list>

class Camera : public Entity
{
private:
    Matrix4 tmp;
    Matrix4 invTransform;
    Entity *orbitTarget;
    Transform3d camOrbit; //transform representing the orbit around target
    
    float moveFwd;
    float moveRight;
    float moveUp;
    Vector3 velocity;
    float dragX;
    float dragY;
    
    
    ~Camera();

protected:
    virtual void Orbit();
public:
    Camera(){ orbitTarget = 0; linearDrag = 0.5f; moveForce = 0.10f; moveFwd = 0.0, moveRight = 0.0;}
    
    
    void Init();
    void Update(float dt);
    void Render() {}
    GLFrustum *frustum;

    //fly look 
    float linearDrag;
    float moveForce;
    
    float distance;
    float orbitSpeedX;
    float orbitSpeedY;
    //Matrix4 projection;
    Matrix4 inverseView;
   
    inline void GetProjection(Matrix4 out)
    {
        Matrix4Copy(out, frustum->projMatrix);
    }
    
    inline Matrix4 &GetProjection() { return frustum->projMatrix; }
    inline Matrix4 &GetInverseView() { return inverseView; }
    
    std::list<Vector3> camTrail;
    void SetOrbit(Entity *orbit);
    //yes stuff you may want to override 
    virtual void SoftAttach(const Vector3 &targetPos, int trailLength);
    virtual void SoftLookAt(const Vector3 &target);
    virtual void HardAttach(const Vector3 &targetPos, const Quaternion &targetRotation);
    virtual void HardLookAt(const Vector3 &target);
    virtual void FlyLookControl();
    

};

#endif
