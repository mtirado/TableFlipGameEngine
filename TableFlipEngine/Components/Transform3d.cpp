/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Transform3d.h"
#include "Entity.h"
Transform3d::Transform3d() 
{
    entity = 0;
    hasChildren = false;
    position = Vector3(0.0f, 0.0f, 0.0f);
    rotation = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    radius = 1.0;
    Matrix4Identity(transform);  
    //right       = (Vector3 *) &transform[0];
    //up          = (Vector3 *) &transform[4];
   // back     = (Vector3 *) &transform[8];
    Update();
    motionState = new TheMotionState(this);
    motionState->disabled = false;
}
Transform3d::~Transform3d() 
{ 
    delete motionState; 
}

void Transform3d::LookAt(const Vector3 &target)
{
    //Vector3 fwd = *this->back * -1.0f;
   // float angle = target.DegreesBetween(fwd);
   // if (angle < 2)
   //     return;

 

    Vector3 gUp = Vector3(0,1,0);
    Vector3 _forward = position - target;
    // _forward = _forward;
    _forward.Normalize();

    Vector3 _right = gUp.Cross(_forward);
    _right.Normalize();
    Vector3 _up = _forward.Cross(_right);

    _up.Normalize();

    rotation.SetFromAxes(_right, _up, _forward);
   // rotation.x = -rotation.x;
   // rotation.y = -rotation.y;
    //rotation.z = -rotation.z;
    rotation.w = -rotation.w;
    Update();
}

 //TODO fucking optimize, excessive normalizes?!!!
void Transform3d::RotateRadians(float pitch, float yaw, float roll)
{
    Quaternion qX, qY, qZ, qXY, qXYZ, qR;
    Vector3 gUp;
    gUp.x = 0.0f;
    gUp.y = 1.0f;
    gUp.z = 0.0f;

    
    qX.SetAxisAngle(GetRight(), (pitch));
    qY.SetAxisAngle(gUp, (yaw));
    //qY.SetAxisAngle(*up, yaw);
    qZ.SetAxisAngle(GetBack(), (roll));
    

    qR = rotation * qX;
    //qR.Normalize();
    qXY = qR * qY;
    //qXY.Normalize();
    qXYZ = qXY * qZ;
    qXYZ.Normalize();
    rotation = qXYZ;
//     qR =  qY * rotation;
//     qR.Normalize();
//     //qXY = qX * qY;
//     //qXY.Normalize();
//     qXYZ = (qX * qR);
//     Quaternion newRotation;
//     
//     qXYZ.Normalize();
//     
//     //qXYZ.w = -qXYZ.w;
//    // rotation = qXYZ;
//     rotation.RotateEulerDegrees(pitch, yaw, roll);
    
    Update();

}

void Transform3d::Update()
{
    rotation.GetMat4(transform);
    memcpy(&transform[12], &position, sizeof(Vector3));
    if (hasChildren) entity->UpdateChildTransforms();
}

//parent is updating child transforms
void Transform3d::ParentUpdate(Vector3 pos, Quaternion rot)
{
    //inherit parent rotation, and translation 
    Vector3 rotatedOffset = rot.GetInverse().RotatePoint(position);

    //rotation = parentTransform->rotation * rotation;
    Vector3 absPos = pos + rotatedOffset;
    // position = parentTransform->position + rotatedOffset;

    rotation = rot;
    rotation.GetMat4(transform);
    memcpy(&transform[12], &absPos, sizeof(Vector3));
    if (hasChildren)
        entity->UpdateChildTransforms();

}

void TheMotionState::getWorldTransform(btTransform &t) const
{
    //if (disabled)
    //    return
    t.setFromOpenGLMatrix(transform->transform);
}
void TheMotionState::setWorldTransform(const btTransform &t)
{
    if (disabled)
        return;
    btVector3 pos = t.getOrigin();
    btQuaternion q = t.getRotation();
    // LOGOUTPUT << "quat: <" << q.getX() << ", " << q.getY() << ", " << q.getZ() << ", " << q.getW() << ">";
    // LogInfo();
    transform->position.x = pos.getX();
    transform->position.y = pos.getY();
    transform->position.z = pos.getZ();

    transform->rotation.x = q.getX();
    transform->rotation.y = q.getY();
    transform->rotation.z = q.getZ();

    //not acutally sure whats going on here, but the bullet quaternion seems inverted to mine?
    transform->rotation.w = -q.getW();
    transform->Update();
}
