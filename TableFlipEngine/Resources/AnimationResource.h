/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef ANIMATION_RESOURCE_H__
#define ANIMATION_RESOURCE_H__

#include "Utilities.h"
#include "Materials.h"
#include "Mesh.h"
#include "Transform3d.h"
//#include "Components.h"

// Model Structs
struct Md5Weight
{
    int index;
    float bias;
    Vector3 position;
};

struct Md5Tri
{
    unsigned int index[3];
};

struct Md5Vert
{
    float s;
    float t;
    int startWeight;
    unsigned int weightCount;
};

struct Md5Bone
{
    char name[32];
    int parent;
    Vector3 position;
    Quaternion rotation;
};

//MD5 Format
struct Md5Mesh : public Mesh
{
public:    
    //md5 data
   
    unsigned int numWeights;
    Md5Vert     *verts;
    Md5Tri      *tris;
    Md5Weight   *weights;
    Md5Mesh() { memset(this, 0, sizeof(Md5Mesh)); isDynamic = true; isIndexed = true; }
};

//MD5 Model format
class AnimatedModelResource : public Resource
{
private:
    
    AnimatedModelResource();
    ~AnimatedModelResource();
    
    void Destroy();
public:
    AnimatedModelResource(int _priority) { priority = _priority; }
    void Delete() { delete this; }
    
    //md5Mesh is derived from mesh component
    Md5Mesh *meshes;
    Md5Bone *bones;
    
    //all materials for submeshes point here
    Material material;
    
    unsigned int numMeshes;
    unsigned int numBones;
    //TODO: add loading logic that combines submeshes with the same material
    bool LoadMd5(const char *filename, ResourceLoadMemInfo *mem = 0);
    void UpdatePose();
};


//anim structs
struct Md5BoneInfo
{
    int parent;
    int flags;
    int startIndex;
    char name[32];
};

struct Md5KeyBone
{
    Vector3 pos;
    Quaternion orientation;
};

struct Md5BoundingBox
{
    Vector3 min,  max;
};



//holds a single animation
class AnimationResource : public Resource
{
private:
    ~AnimationResource();
    
    AnimationResource(){}
public:
    AnimationResource (int _priority = 0);
    void Delete() { delete this; }
    int numFrames;
    int numBones;
    int frameRate;   //frames per second   
    float frameTime; //how long a frame lasts in Milliseconds
    std::string name;
    Md5Bone **keyframes;
    Md5BoundingBox *boundingBoxes;
    
    void BuildKeyframe(const Md5BoneInfo *boneInfo, Md5Bone *baseFrame, 
                       const float *frameData, int numBones, Md5Bone *outKeyframe);
    bool LoadAnimation(const char *filename, ResourceLoadMemInfo *mem = 0);
};

//loader states
#define LOADER_STATE_VERSION    1
#define LOADER_STATE_NUMJOINTS  2
#define LOADER_STATE_NUMMESHES  3
#define LOADER_STATE_JOINTS     4
#define LOADER_STATE_MESHES     5
#define LOADER_MESH_VERTS       6
#define LOADER_MESH_TRIS        7
#define LOADER_MESH_WEIGHTS     8
#define LOADER_MESH_SHADER      9
#define LOADER_ANIM_NUMFRAMES   10
#define LOADER_ANIM_FRAMERATE   11
#define LOADER_ANIM_COMPONENTS  12
#define LOADER_ANIM_HIERARCHY   13
#define LOADER_ANIM_BASEFRAME   14
#define LOADER_ANIM_KEYFRAME    15
#define LOADER_ANIM_BOUNDS      16

#endif
