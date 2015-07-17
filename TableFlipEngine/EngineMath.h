/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#ifndef _ENGINEMATH_H__
#define _ENGINEMATH_H__

#include <math.h>
#include <string.h>

#define PI (3.14159265358979323846)
#define M2PI (2.0 * PI)
#define PI_DIV_180 (0.017453292519943296)
#define INV_PI_DIV_180 (57.2957795130823229)
//gravitational constant
#define G .0000000000667

#define DegToRad(a) ((a) * PI_DIV_180)
#define RadToDeg(a) ((a) * INV_PI_DIV_180)

//converts meters to kilometers :[
#define MtoKM(a) ((a) * 0.001f)

#include "btBulletDynamicsCommon.h"

//column major!
//0  3  6
//1  4  7
//2  5  8
typedef float Matrix3[9];

//column major!
//0 4 8  12  
//1 5 9  13
//2 6 10 14 
//3 7 11 15
typedef float Matrix4[16];

class Vector2
{
public:
    float x;
    float y;
    
    Vector2(){}
    Vector2(float xy) { x = y = xy; }
    Vector2(float _x, float _y) { x = _x; y = _y; }
    Vector2(const Vector2 &v) { x = v.x; y = v.y; }
    
    Vector2  operator  +(const Vector2 &b)      const { return Vector2(x + b.x, y + b.y);       }
    Vector2  operator  -(const Vector2 &b)      const { return Vector2(x - b.x, y - b.y);       }
    Vector2  operator  *(const float &scalar)   const { return Vector2(x * scalar, y * scalar); }
    Vector2  operator  *(const Vector2 &b)      const { return Vector2(x * b.x, y * b.y);       }
    Vector2  operator  /(const float &scalar)   const { return Vector2(x / scalar, y / scalar); }
    Vector2  operator  /(const Vector2 &b)      const { return Vector2(x / b.x, y / b.y);       }
    
    Vector2 &operator  =(const Vector2 &other) { x = other.x;     y = other.y;     return *this;}
    Vector2 &operator +=(const Vector2 &other) { x = x + other.x; y = y + other.y; return *this;}
    Vector2 &operator -=(const Vector2 &other) { x = x - other.x; y = y - other.y; return *this;}
    Vector2 &operator *=(const Vector2 &other) { x = x * other.x; y = y * other.y; return *this;}
    Vector2 &operator *=(const float &scale)   { x = x * scale;   y = y * scale;   return *this;}
    Vector2 &operator /=(const Vector2 &other) { x = x / other.x; y = y / other.y; return *this;}
    Vector2 &operator /=(const float &scale)   { x = x / scale;   y = y / scale;   return *this;}
    
    bool operator ==(const Vector2 &other) { return ( x == other.x && y == other.y ); }

    inline float MagnitudeSq() const { return (x * x) + (y * y); }
    inline float Magnitude()   const { return sqrtf( (x * x) + (y * y) ); }
};

std::ostream & operator <<(std::ostream &out, const Vector2 &v);

class Vector3 
{
public:
    float x;
    float y;
    float z;
    
    Vector3(){}
    Vector3(float xyz) { x = y = z = xyz; }
    Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
    Vector3(const Vector3 &v) { x = v.x; y = v.y; z = v.z; }
    Vector3(const btVector3 &v) { x = v.getX(); y = v.getY(); z = v.getZ(); }
    
    
    Vector3  operator  +(const Vector3 &b)    const { return Vector3(x + b.x, y + b.y, z + b.z); }
    Vector3  operator  -(const Vector3 &b)    const { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3  operator  *(const float &scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3  operator  /(const float &scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
    Vector3 &operator  =(const Vector3 &other) { x = other.x; y = other.y; z = other.z; return *this; }
    Vector3 &operator +=(const Vector3 &other) { x = x + other.x; y = y + other.y; z = z + other.z; return *this; }
    Vector3 &operator -=(const Vector3 &other) { x = x - other.x; y = y - other.y; z = z - other.z; return *this; }
    Vector3 &operator *=(const float &scale) { x = x * scale; y = y * scale; z = z * scale; return *this; }
    Vector3 &operator /=(const float &scale) { x = x / scale; y = y / scale; z = z / scale; return *this; }
    bool operator ==(const Vector3 &other) { return ( x == other.x && y == other.y && z == other.z); }
    
    //bullet interface
    //Vector3(const btVector3 &v) { x = v.getX(); y = v.getY(); z = v.getZ(); }
    //TODO  i really dont like this...   how to solve the issue of converting our vec3 to btvec3
    inline btVector3 btVector() const { return btVector3(x,y,z); }
    
    inline Vector3 Cross(const Vector3 &b) const
    {
        return Vector3(  ((y * b.z) - (b.y * z)),
                        -((x * b.z) - (b.x * z)),
                         ((x * b.y) - (b.x * y))   );
    }
    Vector3 Normalized()
    {
        float mag = sqrtf((x * x) + (y * y) + (z * z));
        if (mag != 0) return Vector3(x / mag,  y / mag, z / mag); 
        else
        { return *this; }
    }
    inline void Normalize()
    {
        float mag = sqrtf((x * x) + (y * y) + (z * z)); 
        if (mag != 0) { x = x / mag; y = y / mag; z = z / mag; }
    }
   
    inline float Dot(const Vector3 &b) const { return x * b.x + y * b.y + z * b.z; }

    //get angle in radians between 2 vectors
    float AngleBetween(const Vector3 &b) const;
    inline float DegreesBetween(const Vector3 &b) const { return RadToDeg(acos(x * b.x + y * b.y + z * b.z)); }
    inline float MagnitudeSq() const { return (x * x) + (y * y) + (z * z); }
    inline float Magnitude() const { return sqrtf((x * x) + (y * y) + (z * z)); } 

    Vector3 Transform(Matrix4 b) const; //transform it!
    Vector3 Rotate(Matrix4 b) const;  //just rotate
};

std::ostream & operator <<(std::ostream &out, const Vector3 &v);
   

//only case i prolly need this is frustum..
class Vector4
{
public:

    float x;
    float y;
    float z;
    float w;

    Vector4() {};
    Vector4(float xyzw)             { x = y = z = w = xyzw;             }
    Vector4(float _x, float _y,
            float _z, float _w)     { x = _x; y = _y; z = _z; w = _w;   }
};

class GLFrustum
{
public:
    
    GLFrustum() 
    { 
        SetOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);    
    }

        // Set the View Frustum
    GLFrustum(float fFov, float fAspect, float fNear, float fFar)
    { 
        SetPerspective(fFov, fAspect, fNear, fFar); 
        
    }

    GLFrustum(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
    { 
        SetOrthographic(xMin, xMax, yMin, yMax, zMin, zMax);    
    }
    
    void SetOrthographic(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
    void SetPerspective(float fFov, float fAspect, float fNear, float fFar);

    Matrix4 projMatrix;
    
    //untransformed frustum corners
    Vector4 nearUL, nearLL, nearUR, nearLR;
    Vector4 farUL, farLL, farUR, farLR;
    
    //transformed frustum corners
    Vector4 nearULT, nearLLT, nearURT, nearLRT;
    Vector4 farULT, farLLT, farURT, farLRT;
    
    //base and transformed plane equations
    Vector4 nearPlane, farPlane, leftPlane, rightPlane;
    Vector4 topPlane, bottomPlane;
    
};

class Quaternion
{
public:
    float x;
    float y;
    float z;
    float w;
    
    Quaternion operator*(const Quaternion &b) ;
   // Vector3 operator* (const Vector3 &vec);
    Quaternion operator*(const Vector3 &vec)  { return Quaternion( (w * vec.x) + (y * vec.z) -  (z * vec.y),
                                                                        (w * vec.y) + (z * vec.x) -  (x * vec.z),
                                                                        (w * vec.z) + (x * vec.y) -  (y * vec.x),  
                                                                      - (x * vec.x) - (y * vec.y) -  (z * vec.z) );}
    Quaternion operator/(const Quaternion &b) ;
    Quaternion() { Identity(); }
    Quaternion(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }
    
    inline void Identity() { x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f; }
    inline void Copy(Quaternion in) { x = in.x; y = in.y; z = in.z; w = in.w; }
    inline Quaternion GetInverse() { return Quaternion(-x, -y, -z, w); }
    inline float Dot(Quaternion &b) { return ((x*b.x) + (y*b.y) + (z*b.z) + (w*b.w)); }
    inline float Angle() { return acos(w) * 2.0f; }
    Quaternion Slerp(Quaternion &b, float t);
    Quaternion Lerp(Quaternion &b, float t);
    
    void Normalize();
    void Multiply(Quaternion b);
    //apply rotation by angle, along axis
    void RotateEulerDegrees(float pitch, float yaw, float roll);
    //test this...
    void SetAxisAngle(Vector3 v, float angle);
    void SetEurlerAngles(float pitch, float yaw, float roll);
    
    void GetMat4(Matrix4 out);
    float GetAxisAngle(Vector3 &axis);
    //NOTE test this, axis may be wrong.
    Vector3 GetEurlerAngles();
    
    void SetFromAxes(Vector3 right, Vector3 up, Vector3 forward);
    
    Vector3 RotatePoint(const Vector3 &pt);
    inline Quaternion CrossVector(const Vector3 &v)
    {
        return Quaternion(
        (w * v.x) + (y * v.z) - (z * v.y),
        (w * v.y) + (z * v.x) - (x * v.z),
        (w * v.z) + (x * v.y) - (y * v.x),
        (x * v.x) - (y * v.y) - (z * v.z) );
    }
};


//inline void Matrix3GetColumn(Vector
inline void Matrix3Copy(Matrix4 out, Matrix3 in) { memcpy(out, in, sizeof(Matrix3)); }
inline void Matrix4Copy(Matrix4 out, Matrix4 in) { memcpy(out, in, sizeof(Matrix4)); }

//maybe we shouldnt inline.. i dunno, f it
void Matrix3Identity(Matrix3 a); 
void Matrix4Identity(Matrix4 a);

//do we need get/set column?
//extract / inject rotattion?

void Matrix3Multiply(Matrix3 out, Matrix3 a, Matrix3 b);
void Matrix4Multiply(Matrix4 out, Matrix4 a, Matrix4 b);
void Matrix4MultiplyVector4(Vector4 out, Matrix4 a, Vector4 b);

inline void Matrix3CreateScale(Matrix3 out, float x, float y, float z)
{
    Matrix3Identity(out);
    out[0] = x; out[4] = y; out[8] = z;
}

inline void Matrix4CreateScale(Matrix4 out, float x, float y, float z)
{
    Matrix4Identity(out);
    out[0] = x; out[5] = y; out[10] = z;
}

///////////////////////////////////////////////////////////////////////////////
// Extract a rotation matrix from a 4x4 matrix
// Extracts the rotation matrix (3x3) from a 4x4 matrix
inline void Matrix4ExtractRotation(Matrix3 dst, const Matrix4 src)
{       
    memcpy(dst, src, sizeof(float) * 3); // X column
    memcpy(dst + 3, src + 4, sizeof(float) * 3); // Y column
    memcpy(dst + 6, src + 8, sizeof(float) * 3); // Z column
}

inline void Matrix4CreateTranslation(Matrix4 out, float x, float y, float z)
{
    Matrix4Identity(out);
    out[12] = x; out[13] = y; out[14] = z;
}


void Matrix4LookAt(Vector3 eye, Vector3 pt, Vector3 up, Matrix4 out);


void MatrixInvert(Matrix4 mInverse, const Matrix4 m);
void MatrixCreatePerspective(Matrix4 out, float fov, float aspect, float near, float far);
void MatrixCreateOrthographic(Matrix4 out, float left, float right, float bottom, float top, float near, float far);

///point in rect
inline bool ptInRect(const Vector2 &pt, const Vector2 &origin, const Vector2 &dimensions)
{
    return ( (pt.x >= origin.x && pt.x < origin.x + dimensions.x) && 
             (pt.y >= origin.y && pt.y < origin.y + dimensions.y) );
}

///convert point size to pixels
///with freetype-gl this leaves a perfect amount of room at top/bottom of uitextout, 
/// -- may vary on OS??
inline float ptToPx(float pt)
{
    return pt * 1.3334;
}

//calculate face normal from 3 points
inline Vector3 ComputeNormal(Vector3 &a, Vector3 &b, Vector3 &c)
{
    Vector3 v1 = a - b;
    Vector3 v2 = c - b;
    Vector3 out = v1.Cross(v2);
    out.Normalize();;
    return out;
}

inline float Lerp(float start, float end, float value)
{
    return ((1.0f - value) * start) + (value * end);
}
#define interpLinear Lerp
inline static float interpHermite(float start, float end, float value)
{
    return Lerp(start, end, value * value * (3.0f - 2.0f * value));
}

inline static float interpSin(float start, float end, float value)
{
    return Lerp(start, end, sinf(value * PI * 0.5f));
}

inline static float interpCos(float start, float end, float value)
{
    return Lerp(start, end, 1.0f - cosf(value * PI * 0.5f));
}

float interpSmoothStep (float min, float max, float x);

float clampf(float in, float min, float max);

float radiusFromAABB(Vector3 &min, Vector3 &max);

#endif
