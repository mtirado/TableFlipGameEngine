/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef _BUILDER_H_
#define _BUILDER_H_
//how to construct a particular entity

#include "Utilities.h"
#include "Resources.h"
#include "Materials.h"
#include "AnimationResource.h"
#include <Sprite.h>
//#include "CollisionShapes.h"
#include <vector>
#include <string>
class Entity;
class CollisionShape;

//all the information we could ever need about creating an entity and components
//construction objects

struct MeshConstructor
{
    bool staticCollisionMesh; //if static i'm just going to use a concave shape, might be faster if we have a convex option
    ModelResource *mdl;
   
    MeshConstructor() { mdl = 0; staticCollisionMesh = false; }
    ~MeshConstructor() { if (mdl) mdl->RemoveRef(); }
};

struct SpriteConstructor
{
    bool animated;
    bool tiled;
    string tilemapFile;
    int numFrames;
    Vector2 dimensions;
    Vector2 offset;
    SpriteFrame *frames;
    SpriteResource *spriteResource;
    SpriteType spriteType;
   
    float z;
    
    ~SpriteConstructor()
    {
        if (!spriteResource)
        {
            //contained in sprite resource (if the resource was actually created)
         if (numFrames == 1)
                delete frames;
            else if (numFrames > 1)
                delete[] frames;
        }
    }
    SpriteConstructor() { spriteResource = 0; }
};

//only supports md5 right now
struct SkinnedMeshConstructor
{
    Material *mat;
    AnimatedModelResource *animatedModel;
    SkinnedMeshConstructor() {  mat = 0; animatedModel = 0; }
    ~SkinnedMeshConstructor() { if (animatedModel) animatedModel->RemoveRef(); }
};

struct AnimationControllerConstructor
{
    vector<AnimationResource *> animations;
    ~AnimationControllerConstructor() 
    {
        vector<AnimationResource *>::iterator iter = animations.begin();
        while (iter != animations.end())
        {
            (*iter)->RemoveRef();
            iter++;
        }
    }
};

struct RigidBodyConstructor
{ 
    CollisionShape *shape;
    float mass;
    float friction;
    float linearDrag;
    float angularDrag;
    float rollingFriction;
    float restitution;   //bounciness?
};

struct CollisionShapeConstructor
{
    int type; 
    bool isTrigger;
    //all info we need for every type possible
    float sphereRadius;
    Vector3 boxHalfWidths;
};

struct CharacterControllerConstructor
{
    float width;
    float height;
    float stepHeight;
    float yOffset;
    float zOffset;
};


#define MAX_UNIFORM_LINKS 32
#define MAX_ANIMS 256
struct UniformLinkConsructor //for setting up material/shader data transfer
{
    string name;
    void *data;
    ShaderVariables type;
};
struct MaterialConstructor
{
    ShaderResource *shader;
    int numLinks;
    UniformLinkConsructor *uniformLinks;
    MaterialConstructor() { shader = 0; numLinks = 0; uniformLinks = 0; }
    ~MaterialConstructor()
    {
        if (shader)
            shader->RemoveRef();
        
        if (uniformLinks)
        {
            for (int i = 0; i < numLinks; i++)
            {
                if (uniformLinks[i].type == SV_SAMPLER2D)
                {
                    ((TextureResource *)uniformLinks[i].data)->RemoveRef();
                }
                else
                //shader variables are 4 bytes, sooo i think this is safe for all types
                delete[] (int *)uniformLinks[i].data;
            }
            delete[] uniformLinks;
        }
    }
};


class BlueprintResource : public Resource
{
private:
    BlueprintResource();
    ~BlueprintResource()
    {
        if (meshConstructor) delete meshConstructor;
        if (skMeshConstructor) delete skMeshConstructor;
        if (spriteConstructor) delete spriteConstructor;
        if (materialConstructor) delete materialConstructor;
        if (charConstructor) delete charConstructor;
        if (shapeConstructor) delete shapeConstructor;
        if (animControllerConstructor) delete animControllerConstructor;
        if (rbConstructor) delete rbConstructor;
        
    }
public:
    BlueprintResource(int _priority) {  priority = _priority;
            meshConstructor = 0; skMeshConstructor = 0; rbConstructor = 0;
            animControllerConstructor = 0; shapeConstructor = 0; charConstructor = 0;
            materialConstructor = 0; spriteConstructor = 0; instances = 0;
    }
    
        void Delete() { delete this; }
    //base name for entity (will increment on instantiation?)
    std::string name;
    unsigned int instances; //how many have been created.
    
    // i think this will only be needed for saving blueprints
    //TODO implement that someday i guess
    /*std::string modelFile;
    std::string shaderFile;
    std::string textureFile;
    std::string animatedModelFile;
    std::vector<std::string> animationFiles;*/
    
    std::string logicScript;
    
    MeshConstructor *meshConstructor;
    SpriteConstructor *spriteConstructor;
    SkinnedMeshConstructor *skMeshConstructor;
    RigidBodyConstructor *rbConstructor;
    AnimationControllerConstructor *animControllerConstructor;
    CollisionShapeConstructor *shapeConstructor;
    CharacterControllerConstructor *charConstructor;
    MaterialConstructor *materialConstructor;
    
    /*
    ModelResource *modelResource;
    ShaderResource *shaderResource;
    TextureResource *textureResource;
    AnimatedModelResource *animatedModelResource;
    AnimationResource *animationResource;*/
};


class Builder
{
private:
     //disable functions for singleton use
    Builder();
    ~Builder();
    Builder(const Builder &) {}
    Builder &operator = (const Builder &) { return *this;}
    
    //the singletons instance.
    static Builder *instance;
    
public:
    static void InitInstance();
    static void DeleteInstance();
    inline static Builder *GetInstance() { return instance; }
    Entity *ConstructEntity(BlueprintResource *bp, Vector3 position);
    BlueprintResource *CreateBlueprint(const char *filename, int _priority, ResourceLoadMemInfo *mem = 0);
    
    //TODO  do we really need this? if so, implement!
   // Entity *ConstructEntity(unsigned int blueprintId){} //uses ID instead of a string, faster blueprint fetches.
};

#endif
