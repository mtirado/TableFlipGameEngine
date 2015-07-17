/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef SKELETAL_MESH_H_
#define SKELETAL_MESH_H_
#include "Utilities.h"
#include "Resources.h"

#include "Materials.h"
class Md5Bone;
class Md5Mesh;

//base class for skeletal animation
class SkeletalMesh  : public Engine::Component
{
public: 
    Material *material;
    int numBones;
    //TODO probably need to rename this struct at some point.
    Md5Bone *bones;     //independant skeleton
    int numMeshes;
    Md5Mesh *meshData;  //the meshes
    virtual void UpdatePose() = 0;
    SkeletalMesh() { material = 0; }
    virtual ~SkeletalMesh() {};
    //returns -1 if not found
    int GetBone(const char *name);
    
};

//Md5 format
class SkinnedMesh : public SkeletalMesh
{
private:
    AnimatedModelResource *animatedMesh;    
    SkinnedMesh ();
public:
    
    SkinnedMesh(Entity *ent, Transform3d *trans, AnimatedModelResource *mesh, Material *_material);
    ~SkinnedMesh();
    
    void UpdatePose();
   
    inline const Material &GetMaterial() const { return *material; }
    
};

#endif
