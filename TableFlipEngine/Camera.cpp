/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#include "Camera.h"
#include "Input.h"

void Camera::Init()
{
   velocity = Vector3(0.0f);
   distance = 320.0f;
   orbitSpeedX = 1.0;
   orbitSpeedY = 0.65;
}
//sorting renderable objects and whatnot, will need seperate render queus?
void Camera::Update(float dt)
{
    Vector3 p = CTransform->position;
    if (Input::GetInstance()->GetKeyDown(KB_MINUS))
        distance += 1.5;
    if (Input::GetInstance()->GetKeyDown(KB_EQUAL))
        distance -= 1.5;

    if (Input::GetInstance()->mouseBtn1Down)
    {
        Vector3 rotAxis;
        rotAxis.x = Input::GetInstance()->GetYAxis();
        rotAxis.y = Input::GetInstance()->GetXAxis();
        rotAxis.z = 0.0f;
        GetTransform()->RotateRadians(rotAxis.x, rotAxis.y, rotAxis.z);
    }
    

    GetTransform()->position += velocity;
    //movement feels
    GetTransform()->Update();
    //update inverse view storage
    MatrixInvert(inverseView, GetTransform()->transform);

    velocity *= linearDrag;


}

//kinda springy, maybe buttery, but definitely not rigid
void Camera::SoftAttach(const Vector3& targetPos, int trailLength)
{
    camTrail.push_back(targetPos);
   
    if (camTrail.size() == trailLength)
    {
        Vector3 newpos = camTrail.front();
        camTrail.pop_front();
        
        GetTransform()->position = (GetTransform()->position * 7 + newpos) / 8;
       
    }
}
void Camera::SoftLookAt(const Vector3& target)
{

}
void Camera::HardAttach(const Vector3& targetPos, const Quaternion &targetRotation)
{
    GetTransform()->position = targetPos;
    GetTransform()->rotation = targetRotation;
    GetTransform()->Update();
}

void Camera::HardLookAt(const Vector3& target)
{
    GetTransform()->LookAt(target);
}

void Camera::SetOrbit ( Entity* orbit )
{
    orbitTarget = orbit;
    dragX = Input::GetInstance()->GetXAxis() * orbitSpeedX;
    dragY = Input::GetInstance()->GetYAxis() * orbitSpeedY;

    camOrbit.RotateRadians(dragY, dragX, 0.0f);
    Vector3 orbitPos = camOrbit.GetBack();

    //TODO raycast from target in camera direction, clamp to geom within range
    Vector3 camPos = (orbitTarget->GetTransform()->position + (orbitPos * distance) ) ;

    GetTransform()->position = camPos;
   // HardLookAt(orbitTarget->GetTransform()->position);
}

void Camera::Orbit()
{
    
    if (orbitTarget && Input::GetInstance()->GetMouseBtn1Down())
    {

        dragX = Input::GetInstance()->GetXAxis() * orbitSpeedX;
        dragY = Input::GetInstance()->GetYAxis() * orbitSpeedY;
    
        camOrbit.RotateRadians(dragY, dragX, 0.0f);
        Vector3 orbitPos = camOrbit.GetBack();

        //TODO raycast from target in camera direction, clamp to geom within range
        Vector3 camPos = (orbitTarget->GetTransform()->position + (orbitPos * distance) ) ;

        GetTransform()->rotation = camOrbit.rotation;
        GetTransform()->position = camPos;
        GetTransform()->Update();
        //HardLookAt(orbitTarget->GetTransform()->position);
        //GetTransform()->position = camPos;
        
       /* x += Input::GetInstance()->GetXAxis() * xSpeed;// * Time.deltaTime;
        y -= Input::GetInstance()->GetYAxis() * ySpeed;// * Time.deltaTime;

        //y = ClampAngle(y, yMinLimit, yMaxLimit);
        if (y < -360)
        {
            y += 360;
        }
        if (y > 360)
        {
            y -= 360;
        }
        y = clampf(y, yMinLimit, yMaxLimit);
        Quaternion q;// = Quaternion(y,x,0);
        q.SetEurlerAngles(y,x,0.0);
        GetTransform()->rotation = q;
        GetTransform()->position = q.RotatePoint(Vector3(0.0f, -distance, 0.0f)) + orbitTarget->GetTransform()->position;
        GetTransform()->Update();
        HardLookAt(orbitTarget->GetTransform()->position);
        //transform.rotation = Quaternion.Euler(y, x, 0);
        //transform.position = (Quaternion.Euler(y, x, 0)) * new Vector3(0.0f, 0.0f, -distance) + target.position;*/
    
    }
   // HardLookAt(orbitTarget->GetTransform()->position);
}

void Camera::FlyLookControl()
{
    //TODO move all controls to the game code...
        float speed = 6.0;
        if (Input::GetInstance()->GetKeyDown(KB_LCTRL))
        {
            speed = 1.20f;
        }
        if (Input::GetInstance()->GetKeyDown(KB_LSHIFT))
        {
            speed = 18.01f;
        }

        bool f = false;
        bool s = false;
        bool u = false;

        if (Input::GetInstance()->GetKeyDown(KB_S))
        {
            moveFwd = moveForce * speed; //reversed because GetBack()...
            f = true;
        }
        if (Input::GetInstance()->GetKeyDown(KB_W))
        {
            moveFwd = -moveForce * speed;
            f = true;
        }
        if (Input::GetInstance()->GetKeyDown(KB_A))
        {
            moveRight = -moveForce * speed;
            s = true;
        }
        if (Input::GetInstance()->GetKeyDown(KB_D))
        {
            moveRight = moveForce * speed;
            s = true;
        }
        if (Input::GetInstance()->GetKeyDown(KB_Q))
        {
            moveUp = moveForce * speed; //reversed because GetBack()...
            u = true;
        }
        if (Input::GetInstance()->GetKeyDown(KB_Z))
        {
            moveUp = -moveForce * speed;
            u = true;
        }
    if (f)
    {
        Vector3 fForce = GetTransform()->GetBack() * moveFwd;
        velocity += fForce;
    }
    if (s)
    {
        Vector3 sForce = GetTransform()->GetRight() * moveRight;
        velocity += sForce;
    }
    if (u)
    {
        Vector3 fForce = GetTransform()->GetUp() * moveUp;
        velocity += fForce;
    }

      

}

