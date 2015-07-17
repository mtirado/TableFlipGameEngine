/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef TRANSFORM_3D_H
#define TRANSFORM_3D_H
#include "Utilities.h"

class TheMotionState;

class Transform3d : public Engine::Component
{
private:

    
public:
   
    Matrix4     transform;       //64
    //if a child trnsform, position is relative to parent.
    Vector3     position;        //12
    Quaternion  rotation;        //16
    float       radius;          //4
    TheMotionState *motionState; //4
    //NOTE if true, position and location are in local space
    bool hasChildren;              //1
                                    //101 + 27 padding
    char padding[27];               //128

    Transform3d();
    ~Transform3d();
    void Update();

    //NOTE should probably remove this and exclusively use get/setters
    inline void UpdatePosition()
    {
        memcpy(&transform[12], &position, sizeof(Vector3));
    }

    inline void SetPosition(Vector3 &pos)
    {
        position = pos;
        memcpy(&transform[12], &position, sizeof(Vector3));
    }

    inline void ExtractPosition(Vector3 *out) { memcpy(out, &transform[12], sizeof(Vector3)); }
    inline Vector3 ExtractPosition() { return *(Vector3 *)&transform[12]; }
    //parents transform has updated, recalculate!
    void ParentUpdate(Vector3 pos, Quaternion rot);
    
    
    //TODO this doesnt really make any sense...  dont use it...
    void GetTransform(Matrix4 t) { memcpy(t, transform, sizeof(Matrix4)); }
    
    //TODO figure out the optimal way to pass these to angelscript?  
    const Vector3 &GetUp() const { return (*(Vector3 *)&transform[4]); }
    const Vector3 &GetRight() const { return (*(Vector3 *)&transform[0]); }
    const Vector3 &GetBack() const { return (*(Vector3 *)&transform[8]); }
    
    // right       = (Vector3 *) &transform[0];
    //up          = (Vector3 *) &transform[4];
    //back     = (Vector3 *) &transform[8];
    
    void RotateRadians(float pitch, float yaw, float roll);
    
    
    void LookAt(const Vector3 &target);
   
    
     
   
};
class TheMotionState : public btMotionState
{
private:
   // TheMotionState();
    
    Transform3d *transform;
public:
    bool disabled;
    TheMotionState(Transform3d *trans) { transform = trans; }
    //bullet motion state methods
    virtual void getWorldTransform(btTransform &t) const;
    virtual void setWorldTransform(const btTransform &t);
    
};


#endif
