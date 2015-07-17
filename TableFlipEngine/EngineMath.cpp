/*
 * contact: mtirado418@gmail.com
 *
 */ 
#include "EngineMath.h"


static Matrix3 ident3 = 
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static Matrix4 ident4 = 
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

void Matrix3Identity (Matrix3 a) { memcpy(a, ident3, sizeof(Matrix3)); }
void Matrix4Identity (Matrix4 a) { memcpy(a, ident4, sizeof(Matrix4)); }

//column major!
//0  3  6
//1  4  7
//2  5  8
void Matrix3Multiply (Matrix3 out, Matrix3 a, Matrix3 b)
{
    float a0 = a[0]; float a1 = a[3]; float a2 = a[6];
    out[0] = a0 * b[0] + a1 * b[3] + a2 * b[6];
    out[3] = a0 * b[1] + a1 * b[4] + a2 * b[7];
    out[6] = a0 * b[2] + a1 * b[5] + a2 * b[8];  

}


//column major!
//0  4  8  12  
//1  5  9  13
//2  6  10 14 
//3  7  11 15
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  out[(col<<2)+row]
void Matrix4Multiply (Matrix4 out, Matrix4 a, Matrix4 b)
{
        for (int i = 0; i < 4; i++) {
                float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
                P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
                P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
                P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
                P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
        }    
}

void Matrix4MultiplyVector4(Vector4 out, Matrix4 a, Vector4 b)
{
    out.x  = a[0] * b.x;
    out.x += a[1] * b.y;
    out.x += a[2] * b.z;
    out.x += a[3] * b.w;
    
    out.y  = a[4] * b.x;
    out.y += a[5] * b.y;
    out.y += a[6] * b.z;
    out.y += a[7] * b.w;
    
    out.z  = a[8] * b.x;
    out.z += a[9] * b.y;
    out.z += a[10] * b.z;
    out.z += a[11] * b.w;
    
    out.w  = a[12] * b.x;
    out.w += a[13] * b.y;
    out.w += a[14] * b.z;
    out.w += a[15] * b.w;
}

// 3x3 determinant
float DetIJ(const Matrix4 m, const int i, const int j)
{
int x, y, ii, jj;
float ret, mat[3][3];

x = 0;
for (ii = 0; ii < 4; ii++)
    {
    if (ii == i) continue;
    y = 0;
    for (jj = 0; jj < 4; jj++)
        {
        if (jj == j) continue;
        mat[x][y] = m[(ii*4)+jj];
        y++;
        }
    x++;
    }

ret =  mat[0][0]*(mat[1][1]*mat[2][2]-mat[2][1]*mat[1][2]);
ret -= mat[0][1]*(mat[1][0]*mat[2][2]-mat[2][0]*mat[1][2]);
ret += mat[0][2]*(mat[1][0]*mat[2][1]-mat[2][0]*mat[1][1]);

return ret;
}

// Invert matrix
void MatrixInvert(Matrix4 mInverse, const Matrix4 m)
    {
    int i, j;
    float det, detij;

    // calculate 4x4 determinant
    det = 0.0f;
    for (i = 0; i < 4; i++)
        {
        det += (i & 0x1) ? (-m[i] * DetIJ(m, 0, i)) : (m[i] * DetIJ(m, 0,i));
        }
    det = 1.0f / det;

    // calculate inverse
    for (i = 0; i < 4; i++)
        {
        for (j = 0; j < 4; j++)
            {
            detij = DetIJ(m, j, i);
            mInverse[(i*4)+j] = ((i+j) & 0x1) ? (-detij * det) : (detij *det); 
            }
        }
    }
    
    
/* 
Let E be the 3d column vector (eyeX, eyeY, eyeZ).
Let C be the 3d column vector (centerX, centerY, centerZ).
Let U be the 3d column vector (upX, upY, upZ).
Compute L = C - E.
Normalize L.
Compute S = L x U.
Normalize S.
Compute U' = S x L.

M is the matrix whose columns are, in order:

(S, 0), (U', 0), (-L, 0), (-E, 1)  (all column vectors)*/
    
void Matrix4LookAt ( Vector3 eye, Vector3 pt, Vector3 up, Matrix4 out )
{
    Vector3 L = pt - eye;
    L.Normalize();
    
    Vector3 S = L.Cross(up);
    S.Normalize();
    
    Vector3 U = S.Cross(L);
    
    Matrix4Identity(out);
    out[0] = S.x; 
    out[4] = S.y;
    out[8] = S.z;
    out[1] = U.x;
    out[5] = U.y;
    out[9] = U.z;
    out[2] = -L.x;
    out[6] = -L.y;
    out[10] = -L.z;
    out[12] = -eye.x;
    out[13] = -eye.y;
    out[14] = -eye.z;
            
}


Vector3 Vector3::Transform (Matrix4 b) const
{
    return Vector3( b[0] * x + b[4] * y + b[8]  * z + b[12],
                    b[1] * x + b[5] * y + b[9]  * z + b[13],
                    b[2] * x + b[6] * y + b[10] * z + b[14]  );
   
}

Vector3 Vector3::Rotate (Matrix4 b) const
{
    return Vector3( b[0] * x + b[3] * y + b[6] * z,
                    b[1] * x + b[4] * y + b[7] * z,
                    b[2] * x + b[5] * y + b[8] * z  );
}

float Vector3::AngleBetween(const Vector3 &b) const
{
    //is the division really needed?
    float cosa = Dot(b);// / sqrt(MagnitudeSq() * b.MagnitudeSq());
    if (cosa >= 1.0f)
        return 0.0f;
    else if (cosa <= -1.0f)
        return M_PI;
    else
        return acos(cosa);
        
}

std::ostream & operator <<(std::ostream &out, const Vector2 &v)
{
    out << "<" << v.x << ", " << v.y << ">";
    return out;
}

std::ostream & operator <<(std::ostream &out, const Vector3 &v)
{
    out << "<" << v.x << ", " << v.y << ", " <<v.z << ">";
    return out;
}

#define TOLERANCE 0.00001f


void Quaternion::Normalize()
{
    float magSq = (x * x) + (y * y) + (z * z) + (w * w);
    if (fabs(magSq) > TOLERANCE && fabs(magSq - 1.0f) > TOLERANCE)
    {
        float mag = sqrtf(magSq);
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
}

Quaternion Quaternion::operator/(const Quaternion& b)
{
    return Quaternion(x*b.w - y*b.z + z*b.y - w*b.x,
                x*b.z + y*b.w - z*b.x - w*b.y,
               -x*b.y + y*b.x + z*b.w - w*b.z,
                x*b.x + y*b.y + z*b.z + w*b.w);
}

//NOT COMMUTATIVE
Quaternion Quaternion::operator* (const Quaternion& b)
{
    
    return Quaternion(  (x * b.w) + (w * b.x) + (y * b.z) - (z * b.y),
                        (y * b.w) + (w * b.y) + (z * b.x) - (x * b.z),
                        (z * b.w) + (w * b.z) + (x * b.y) - (y * b.x),
                        
                        (w * b.w) - (x * b.x) - (y * b.y) - (z * b.z)   );
}


Vector3 Quaternion::RotatePoint(const Vector3 &pt)
{
    Quaternion inv, tmp, res;
    //inv = Quaternion(-x, -y, -z, w);
    inv = Quaternion(-x, -y, -z, w);
    inv.Normalize();
    
    tmp = (*this) * pt;
    res = tmp * inv;
    return Vector3(res.x, res.y, res.z);
}

//input vector MUST be normalized.
void Quaternion::SetAxisAngle (Vector3 v, float angle)
{
    //Vector3Normalize(v);
    float sinA;
    angle *= 0.5f;
    sinA = sinf(angle);
    
    
    x = v.x * sinA;
    y = v.y * sinA;
    z = v.z * sinA;
    w = cosf(angle);
    
    Normalize();
}

void Quaternion::RotateEulerDegrees(float pitch, float yaw, float roll)
{
    Quaternion tmp;
  
     //convert to degrees, and div by 2
    float y = yaw   * PI_DIV_180 / 2.0f;
    float p = pitch * PI_DIV_180 / 2.0f;
    float r = roll  * PI_DIV_180 / 2.0f;
    
    float c1 = cosf(p);
    float s1 = sinf(p);
    
    float c2 = cosf(y);
    float s2 = sinf(y);
    
    float c3 = cosf(r);
    float s3 = sinf(r);
    
    float c1c2 = c1 * c2;
    float s1s2 = s1 * s2;
    
   
    tmp.z = (c1c2 * s3 - s1s2 * c3);
    tmp.y = c1 * s2 * c3 + s1 * c2 * s3;
    tmp.x = s1 * c2 * c3 - c1 * s2 * s3;
    
    //w
    tmp.w = (c1c2 * c3 + s1s2 * s3);
    
    tmp.Normalize();
    
    
    
    *this = *this * tmp;
    
    Normalize();
}

void Quaternion::SetEurlerAngles (float pitch, float yaw, float roll)
{
    //convert to degrees, and div by 2
    float y = yaw   * PI_DIV_180 / 2.0f;
    float p = pitch * PI_DIV_180 / 2.0f;
    float r = roll  * PI_DIV_180 / 2.0f;
    
    float c1 = cosf(p);
    float s1 = sinf(p);
    
    float c2 = cosf(y);
    float s2 = sinf(y);
    
    float c3 = cosf(r);
    float s3 = sinf(r);
    
    float c1c2 = c1 * c2;
    float s1s2 = s1 * s2;
    
   
    z = (c1c2 * s3 - s1s2 * c3);
    y = c1 * s2 * c3 + s1 * c2 * s3;
    x = s1 * c2 * c3 - c1 * s2 * s3;
    
    //w
    w = (c1c2 * c3 + s1s2 * s3);
    
    Normalize();
}

void Quaternion::GetMat4(Matrix4 out)
{
    //QuaternionNormalize(in);
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;
        
    out[0] = 1.0f - 2.0f * (y2 + z2);
    out[1] = 2.0f * (xy - wz);
    out[2] = 2.0f * (xz + wy);
    out[3] = 0.0f;
    out[4] = 2.0f * (xy + wz);
    out[5] = 1.0f - 2.0f * (x2 + z2);
    out[6] = 2.0f * (yz - wx);
    out[7] = 0.0f;
    out[8] = 2.0f * (xz - wy);
    out[9] = 2.0f * (yz + wx);
    out[10] = 1.0f - 2.0f * (x2 + y2);
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0.0f;
    out[15] = 1.0f;    
}

float Quaternion::GetAxisAngle (Vector3 &axis)
{
//     float scale = sqrtf(x * x + y * y + z * z);
//     axis.x = x / scale;
//     axis.y = y / scale;
//     axis.z = z / scale;

    float scale = sqrtf(1 - w*w);
    axis.x = x / scale;
    axis.y = y / scale;
    axis.z = z / scale;
    
    return acosf(w) * 2.0f;
}

Vector3 Quaternion::GetEurlerAngles()
{
    Vector3 ret;
    double sqw = w*w;
    double sqx = x*x;
    double sqy = y*y;
    double sqz = z*z;
    double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    double test = x*y + z*w;
    if (test > 0.499*unit) { // singularity at north pole
        ret.y = 2 * atan2(x,w);
        ret.z = M_PI/2;
        ret.x = 0;
        return ret;
    }
    if (test < -0.499*unit) { // singularity at south pole
        ret.y = -2 * atan2(x,w);
        ret.z = -M_PI/2;
        ret.x = 0;
        return ret;
    }
    ret.y = atan2(2*y*w-2*x*z , sqx - sqy - sqz + sqw);
    ret.z = asin(2*test/unit);
    ret.x = atan2(2*x*w-2*y*z , -sqx + sqy - sqz + sqw);
    return ret;
}

Quaternion Quaternion::Slerp ( Quaternion& b, float t )
{
    Quaternion out;
    if (t <= 0.0f)
    {
        out = *this;
        return out;
    }

    if (t >= 1.0f)
    {
        out = b;
        return out;
    }

    //compute cosine using dot product.
    float cosOmega = Dot(b);

    /* if negative dot, use -q1.  two quaternions q and -q
    represent the same rotation, but may produce
    different slerp.  we chose q or -q to rotate using
    the acute angle. */
    float q1w = b.w;
    float q1x = b.x;
    float q1y = b.y;
    float q1z = b.z;

    if (cosOmega < 0.0f)
    {
        q1w = -q1w;
        q1x = -q1x;
        q1y = -q1y;
        q1z = -q1z;
        cosOmega = -cosOmega;
    }

        /* compute interpolation fraction, checking for quaternions
    almost exactly the same */

    float k0, k1;

    //they're really close, so linearly interpolate
    if (cosOmega > 0.9899f)
    {
        k0 = 1.0f - t;
        k1 = t;
    }
    else
    {
        float sinOmega = sqrt(1.0f - (cosOmega * cosOmega));
        float omega = atan2(sinOmega, cosOmega);
        float oneOverSinOmega = 1.0f / sinOmega;

        k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
        k1 = sin(t *omega) * oneOverSinOmega;
    }

    
    out.x = (k0 * x) + (k1 * q1x);
    out.y = (k0 * y) + (k1 * q1y);
    out.z = (k0 * z) + (k1 * q1z);
    out.w = (k0 * w) + (k1 * q1w);

    return out;
}

Quaternion Quaternion::Lerp ( Quaternion& b, float t )
{
    Quaternion out;
    if (t <= 0.0f)
    {
        out = *this;
        return out;
    }

    if (t >= 1.0f)
    {
        out = b;
        return out;
    }

    //compute cosine using dot product.
    float cosOmega = Dot(b);

    /* if negative dot, use -q1.  two quaternions q and -q
    represent the same rotation, but may produce
    different slerp.  we chose q or -q to rotate using
    the acute angle. */
    float q1w = b.w;
    float q1x = b.x;
    float q1y = b.y;
    float q1z = b.z;

    if (cosOmega < 0.0f)
    {
        q1w = -q1w;
        q1x = -q1x;
        q1y = -q1y;
        q1z = -q1z;
        cosOmega = -cosOmega;
    }
  
    float k0 = 1.0f - t;
    float k1 = t;
 
    out.x = (k0 * x) + (k1 * q1x);
    out.y = (k0 * y) + (k1 * q1y);
    out.z = (k0 * z) + (k1 * q1z);
    out.w = (k0 * w) + (k1 * q1w);

    return out;
}

void Quaternion::SetFromAxes(Vector3 right, Vector3 up, Vector3 forward)
{
    
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".
    float kRot[3][3];
   
    kRot[0][0] = right.x;
    kRot[1][0] = right.y;
    kRot[2][0] = right.z;

    kRot[0][1] = up.x;
    kRot[1][1] = up.y;
    kRot[2][1] = up.z;

    kRot[0][2] = forward.x;
    kRot[1][2] = forward.y;
    kRot[2][2] = forward.z;


    float fTrace = kRot[0][0]+kRot[1][1]+kRot[2][2];
    float fRoot;

    if ( fTrace > 0.0 )
    {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = sqrtf(fTrace + 1.0f);  // 2w
        w = 0.5f*fRoot;
        fRoot = 0.5f/fRoot;  // 1/(4w)
        x = (kRot[2][1]-kRot[1][2])*fRoot;
        y = (kRot[0][2]-kRot[2][0])*fRoot;
        z = (kRot[1][0]-kRot[0][1])*fRoot;
    }
    else
    {
        // |w| <= 1/2
        static int s_iNext[3] = { 1, 2, 0 };
        int i = 0;
        if ( kRot[1][1] > kRot[0][0] )
            i = 1;
        if ( kRot[2][2] > kRot[i][i] )
            i = 2;
        int j = s_iNext[i];
        int k = s_iNext[j];

        fRoot = sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
        float* apkQuat[3] = { &x, &y, &z };
        *apkQuat[i] = 0.5f*fRoot;
        fRoot = 0.5f/fRoot;
        w = (kRot[k][j]-kRot[j][k])*fRoot;
        *apkQuat[j] = (kRot[j][i]+kRot[i][j])*fRoot;
        *apkQuat[k] = (kRot[k][i]+kRot[i][k])*fRoot;
    }
}



void MakeOrthographicMatrix(Matrix4 mProjection, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
{
    //NOTE this creates an ortho matrix, where the nearest object is zMax, and farthest is zMin
    Matrix4Identity(mProjection);
    float width = xMax - xMin;
    float height = yMax - yMin;
    float depth = zMax - zMin;
    mProjection[0] = 2.0f / (width);
    mProjection[5] = 2.0f / (height);
    mProjection[10] = -1.0f / (depth);
    
    mProjection[12] = -(xMax + xMin)/(width);
    mProjection[13] = -(yMax + yMin)/(height);
    mProjection[14] = -zMin / (depth);
    mProjection[15] = 1.0f;
}


void GLFrustum::SetOrthographic (float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
{
    MakeOrthographicMatrix(projMatrix, xMin, xMax, yMin, yMax, zMin, zMax);

    // Fill in values for untransformed Frustum corners
    // Near Upper Left
    nearUL.x = xMin; nearUL.y = yMax; nearUL.z = zMin; nearUL.w = 1.0f;

    // Near Lower Left
    nearLL.x = xMin; nearLL.y = yMin; nearLL.z = zMin; nearLL.w = 1.0f;

    // Near Upper Right
    nearUR.x = xMax; nearUR.y = yMax; nearUR.z = zMin; nearUR.w = 1.0f;

    // Near Lower Right
    nearLR.x = xMax; nearLR.y = yMin; nearLR.z = zMin; nearLR.w = 1.0f;

    // Far Upper Left
    farUL.x = xMin; farUL.y = yMax; farUL.z = zMax; farUL.w = 1.0f;

    // Far Lower Left
    farLL.x = xMin; farLL.y = yMin; farLL.z = zMax; farLL.w = 1.0f;

    // Far Upper Right
    farUR.x = xMax; farUR.y = yMax; farUR.z = zMax; farUR.w = 1.0f;

    // Far Lower Right
    farLR.x = xMax; farLR.y = yMin; farLR.z = zMax; farLR.w = 1.0f;
}

void GLFrustum::SetPerspective (float fFov, float fAspect, float fNear, float fFar)
{
    float xmin, xmax, ymin, ymax;       // Dimensions of near clipping plane
    float xFmin, xFmax, yFmin, yFmax;   // Dimensions of far clipping plane

    // Do the Math for the near clipping plane
    ymax = fNear * float(tan( fFov * PI / 360.0 ));
    ymin = -ymax;
    xmin = ymin * fAspect;
    xmax = -xmin;

    // Construct the projection matrix
    Matrix4Identity(projMatrix);
   
    
    projMatrix[0] = (2.0f * fNear)/(xmax - xmin);
    projMatrix[5] = (2.0f * fNear)/(ymax - ymin);
    projMatrix[10] = ((fFar + fNear)/(fNear -fFar));
    projMatrix[11] = -1.0f;
    projMatrix[14] = ((2.0f * fFar * fNear)/(fNear - fFar));
    projMatrix[15] = 0.0f;
    // Do the Math for the far clipping plane
    yFmax = fFar * float(tan(fFov * PI / 360.0));
    yFmin = -yFmax;
    xFmin = yFmin * fAspect;
    xFmax = -xFmin;

    // Fill in values for untransformed Frustum corners
    // Near Upper Left
    nearUL.x = xmin; nearUL.y = ymax; nearUL.z = -fNear; nearUL.w = 1.0f;

    // Near Lower Left
    nearLL.x = xmin; nearLL.y = ymin; nearLL.z = -fNear; nearLL.w = 1.0f;

    // Near Upper Right
    nearUR.x = xmax; nearUR.y = ymax; nearUR.z = -fNear; nearUR.w = 1.0f;

    // Near Lower Right
    nearLR.x = xmax; nearLR.y = ymin; nearLR.z = -fNear; nearLR.w = 1.0f;

    // Far Upper Left
    farUL.x = xFmin; farUL.y = yFmax; farUL.z = -fFar; farUL.w = 1.0f;

    // Far Lower Left
    farLL.x = xFmin; farLL.y = yFmin; farLL.z = -fFar; farLL.w = 1.0f;

    // Far Upper Right
    farUR.x = xFmax; farUR.y = yFmax; farUR.z = -fFar; farUR.w = 1.0f;

    // Far Lower Right
    farLR.x = xFmax; farLR.y = yFmin; farLR.z = -fFar; farLR.w = 1.0f;
}

//min, max, values
//returns between 0-1
float interpSmoothStep ( float min, float max, float x )
{
   // x = clampf(x, min, max);
   if (x > max)
       x = max;
   else if (x < min)
       x = min;
   
    float v1 = (x-min)/(max-min);
    float v2 = (x-min)/(max-min);
    return -2*v1 * v1 *v1 + 3*v2 * v2;
}

float clampf(float in, float min, float max)
{
    if (in > max)
        return max;
    else if (in < min)
        return min;
    else
        return in;
}

float radiusFromAABB(Vector3 &min, Vector3 &max)
{
    float x = abs(min.x - max.x);
    float y = abs(min.y - max.y);
    float z = abs(min.z - max.z);

    if (x > y && x > z)
        return x/2.0f;
    else if (y > x && y > z)
        return y/2.0f;
    else
        return z/2.0f;
}
