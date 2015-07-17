/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#ifndef LIGHT_H__
#define LIGHT_H__
#include "EngineMath.h"

struct PointLight
{
    Vector3 position;
    Vector4 diffuseColor;
    Vector4 specularColor;
    float   magnitude;
    float   range;
    
    PointLight() 
    { 
        position = Vector3(0.0f, 0.0f, 0.0f); 
        magnitude = 1.0f; 
        range = 10.0f;
        memset(&diffuseColor, 0, sizeof(Vector4));
        memset(&specularColor, 0, sizeof(Vector4));
    }
};

class SunLight
{
private:
    
    //stores direction to light, from origin (0,0,0)
    Vector3 direction; 
    GLFrustum shadowDepthFrustum;  
    float shadowFrustumDistance; // how far from 0,0,0 to put the frustum
    Matrix4 shadowV; //view matrix for shadow frustum
    
    //updates frustum and view matrix if direction was changed
    void UpdateMatrices()
    {
        float vpSize = shadowFrustumDistance * 1.5f;
        shadowDepthFrustum.SetOrthographic(-vpSize,vpSize, -vpSize, vpSize, -1, 200);
       // Vector3 invDir = direction * -1.0f;
        Vector3 sfPos = direction * shadowFrustumDistance;
        Matrix4LookAt(sfPos, Vector3(0,0,0) , Vector3(0,1,0), shadowV);
    }
    
public :
    
    Vector4 diffuseColor;
    Vector4 specularColor;
    float intensity;
    
    void SetDirection(Vector3 dir)
    {
        direction = dir * -1.0f; //invert direction because thats what our shaders want
        direction.Normalize();
        UpdateMatrices();
    }
    void SetShadowFrustumDistance(float dist)
    {
        shadowFrustumDistance = dist;
        UpdateMatrices();
    }
    void GetViewProjection(Matrix4 out)
    {
        Matrix4Multiply(out, shadowDepthFrustum.projMatrix, shadowV);
    }
    const Vector3 GetDir() { return direction * -1.0f; }//invert back to normal
    const Vector3 *GetInvDir() { return &direction; }
    
    SunLight()
    {
        direction = Vector3(0,1000,0);
        direction.Normalize();
        intensity = 1.0f;
        memset(&diffuseColor, 0, sizeof(Vector4));
        memset(&specularColor, 0, sizeof(Vector4));   
        shadowFrustumDistance = 50.0f;
        UpdateMatrices();
    }
};

#endif
