/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef MESH_H_
#define MESH_H_
#include "Utilities.h"
#include "Resources.h"
//collection of vertices to be rendered,
class Mesh  : public Engine::Component
{
protected:
    //holds mesh data, dynamic mesh will have a copy of its own data.
    ModelResource *model;   
    Mesh() { material = 0; }
public:
    
    bool isDynamic;
    bool isIndexed;
    bool hidden;
    Material *material; 
      
    //mesh data if mesh if dynamic
    unsigned int numVerts;
    unsigned int numTris;
    //mesh data 
    Vector3 *positions;
    Vector3 *normals;
    Vector2 *texCoords;
    GLuint  *indices;
    //GL buffer id's
    GLuint vertPositions;
    GLuint vertNormals;
    GLuint vertTexCoords;
    GLuint triIndices;
    GLuint barycentricPositions;
    
    void SetMaterial(Material &mat);
    
    //sends buffer data to GL
    void PrepareMaterial();
    void FinishMaterial();
    
    inline btTriangleMesh *GetCollisionMesh() { return model->collisionMesh; }
    
    //NOTE passing material reference will call copy constructor
    Mesh(Entity *ent, Transform3d *trans, ModelResource *mdl, Material &mat, bool indexed = false, bool dynamic = false);
    //but passing pointer, will just straight set the pointer, shader reference counting done through loadShader
    Mesh(Entity *ent, Transform3d *trans, ModelResource *mdl, Material *mat, bool indexed = false, bool dynamic = false);
    ~Mesh();

    inline float GetMinX() { return model->GetMinX(); }
    inline float GetMaxX() { return model->GetMaxX(); }
    inline float GetMinY() { return model->GetMinY(); }
    inline float GetMaxY() { return model->GetMaxY(); }
    inline float GetMinZ() { return model->GetMinZ(); }
    inline float GetMaxZ() { return model->GetMaxZ(); }
    
};

#endif
