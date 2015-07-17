/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef MODEL_RESOURCE__H__
#define MODEL_RESOURCE__H__
#include "Utilities.h"
#include "btBulletDynamicsCommon.h"

#include <vector>


class ModelResource : public Resource
{
private:

    //stores the material passed in from being material
    //so we can call finish material without having to know..
    Vector2 minmaxX;
    Vector2 minmaxY;
    Vector2 minmaxZ;
       
    //use the resource creation functions...  doy
    ModelResource();
    ~ModelResource();
    
public:
    inline void Delete() { delete this; }
    
public:

    bool isIndexed;
    
    unsigned int numVerts;
    
    Vector3 *positions;
    Vector3 *barycentric;
    Vector3 *normals;
    Vector2 *texCoords;
    btTriangleMesh *collisionMesh;

    //static models will all share these buffers
    GLuint vertPositions;
    GLuint vertNormals;
    GLuint vertTexCoords;
    GLuint barycentricPositions;

    //used for quad wireframe to correct aspect ratio
    float widthToHeightAspect;
    
    ModelResource(int _priority) : Resource(_priority)
    {
        barycentric = 0; barycentricPositions = 0;
        positions = 0; normals = 0; texCoords = 0; 
        vertPositions = 0; ; vertNormals = 0; vertTexCoords = 0;
        numVerts = 0; collisionMesh = 0; isIndexed = false;
                                                         
    }
    
    
    
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    
    //file on disk, and mesh to fill out              //generate collision mesh?
    bool LoadObj(const char *filename, bool _collisionMesh = false, ResourceLoadMemInfo *mem = 0);
    bool LoadXFWStream(FILE* file, const char *filename, bool _collisionMesh = false);
    bool LoadCube(bool skybox = false);  //skybox = facing inward
    bool LoadCubeTriStrip();
    GLuint GetBarycentric(); //generates them on demand
    
    inline float GetMinX() { return minmaxX.x; }
    inline float GetMaxX() { return minmaxX.y; }
    inline float GetMinY() { return minmaxY.x; }
    inline float GetMaxY() { return minmaxY.y; }
    inline float GetMinZ() { return minmaxZ.x; }
    inline float GetMaxZ() { return minmaxZ.y; }
    
    
};


#endif
