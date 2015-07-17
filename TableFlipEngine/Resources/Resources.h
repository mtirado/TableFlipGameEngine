/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef RESOURCES_H__
#define RESOURCES_H__

//NO HACKS ALLOWED IN HERE!!!!!!!!!
//very very very very very important
//I REPEAT THIS IS A HACK FREE ZONE!

//watch this memory like a hawk, all refs given out and whatnot.
#include "Utilities.h"
#include "ShaderResource.h"
#include "ModelResource.h"
#include "SoundResource.h"

struct SpriteConstructor;
class BlueprintResource;
class AnimatedModelResource;
class AnimationResource;
class Entity;
class Material;
#include <stdio.h>

#if defined(__ANDROID__)
#include <android_native_app_glue.h>
static AAssetManager* G_AssetManager = 0;
void SetAndroidAssetManager(AAssetManager *mgr);
AAssetManager *GetAndroidAssetManager();
#endif


#include "SOIL.h"

#include <string>
#include <vector>
#include <map>
//using std::string;
//using std::map;
//using std::vector;



/*  TODO  restructure resources to allow for easier creation of new types that wil work on android and ios!
You can still use stdio.h in an NDK project. #include and pound away.

The only trick is getting the path to your application's private data folder.
It's available as state->activity->internalDataPath in android_main. 
Outside of that path, the app has no filesystem rights.
*/
#if defined(PC_PLATFORM)

#endif

//might be a good idea to have a default texture, an obvious one.
class TextureResource : public Resource
{
private:
    GLuint texID;
    TextureResource();
    //moved this in here FOR YOUR OWN GOOD!!
    ~TextureResource() { if (texID != 0) glDeleteTextures(1, &texID); }
public:
    TextureResource(const char *filename, GLuint _texID, int _priority) { FileName = filename; texID = _texID; priority = _priority; references = 0; }
    
    //DO NOT EVER CALL THIS
    void Delete()                { delete this; }
    
    inline GLuint GetTexId()               { return texID; }
    
    //inline void SetWidth(int w)         { width = w; }
    //inline void SetHeight(int h)        { height = h; }
    //inline void SetChannels(int c)      { channels = c; }
    
    //check out the SOIL channel and flag options, 
    //bool LoadFromFile(const char *filename, int forceChannels, unsigned int reuseID, unsigned int flags);
    //bool LoadFromMemory(const char *filename, int forceChannels, unsigned int reuseID, unsigned int flags);
};

struct SpriteFrame
{
    Vector2 texcoords; //bottom left (because the whole engine is bottom left oriented, not changing now)
    Vector2 dimensions; //width height
    //TODO scaling collision shapes?
};

#define MAX_TILE_LAYERS 4

struct TileMap
{
    bool loaded; //is it loaded yet?
    int width;
    int height;
    int tileWidth;
    int tileHeight;
    int numLayers;
    short *layerData;
    
    //TODO multiple textures?
    string texture;
    int imageWidth;
    int imageHeight;
    
    GLuint vertCount[MAX_TILE_LAYERS];
    GLuint vertPositions[MAX_TILE_LAYERS];
    GLuint vertTexCoords[MAX_TILE_LAYERS];
    
    TileMap() {loaded = false;}
    ~TileMap() {if (loaded) 
               { delete[] layerData; glDeleteBuffers(numLayers, vertPositions); 
                                     glDeleteBuffers(numLayers, vertTexCoords);
               }}
};

class SpriteResource : public Resource
{
    friend class Renderer;
private:
    unsigned int numVerts;
   // Vector3 *positions;
   // Vector2 *texCoords;
    Vector2 halfWidths; //rectangle dimensions, if needed per frame add to sprite frame.
    int numFrames;
    //defines the animation frames
    SpriteFrame *frames;
    
    //static models will all share these buffers
    GLuint vertPositions;
    GLuint vertTexCoords;
    SpriteResource(){}
    ~SpriteResource();
    TileMap map;
    bool LoadTilemap(const char *filename);
public:
    SpriteResource (Material *mat, SpriteConstructor *ctr, int _priority);
                            
    void Delete()               { delete this; }
    GLuint getVertPositions()   { return vertPositions; }
    GLuint getVertTexcoords()   { return vertTexCoords; }
    int getNumVerts()           { return numVerts; }
    int getNumFrames()          { return numFrames; }
    bool isTiled()              { return map.loaded; }
    
    //TODO this is NOT good, and materials need to be better designed, the batching, sharing, unique material states, all that!
    Material *material;
};

class StringCompareForMap
{
public:
    
    //hopefully this will not get called often.....
    const bool operator()(const std::string &a, const std::string &b) const
    {
        return a.compare(b) < 0;
    }
};

//needs string identifier so we can look up in the map
enum ModelPrimitive { MDL_CUBE = 0, MDL_SPHERE, MDL_NUM_PRIMITIVES };
/*static const char *primStrings[] = {"__CUBE__",
"__SPHERE__"};*/



//this might be ok, loading rsources is not time critical usually...
static std::map<std::string, TextureResource *, StringCompareForMap> Textures;
static std::map<std::string, ModelResource *, StringCompareForMap> Models;
static std::map<std::string, ShaderResource *, StringCompareForMap> Shaders;
static std::map<std::string, SoundResource *, StringCompareForMap> Sounds;
static std::map<std::string, AnimatedModelResource *, StringCompareForMap> AnimatedMeshes;
static std::map<std::string, AnimationResource *, StringCompareForMap> Animations;
static std::map<std::string, BlueprintResource *, StringCompareForMap> Blueprints;
static std::map<std::string, SpriteResource *, StringCompareForMap> Sprites;

//default priority 0, do not delete untill resource manager shuts down
//TODO implement priority levels??
//size is because of damn android and having to load from resources memory :[
TextureResource         *LoadTexture(const char *file, int forceChannels, unsigned int reuseID, unsigned int flags, int _priority = 0);
ModelResource           *LoadModel(const char *file, bool collisionMesh = false, int _priority = 0);
ModelResource           *LoadModelPrimitive(ModelPrimitive primType, int _priority = 0);
ShaderResource          *LoadShaderResource(const char *file, int _priority = 0); //shaders should probably all be level 0 as there will not be many of them anyway
SoundResource           *LoadSound(const char *filename, SoundTypes type, int _priority = 0);
AnimatedModelResource   *LoadAnimatedMesh(const char *file, int _priority = 0);
AnimationResource       *LoadAnimation(const char *file, int _priority = 0);
BlueprintResource       *LoadBlueprint(const char *file, int _priority = 0);
SpriteResource          *LoadSpriteResource(std::string name, Material *mat, SpriteConstructor *ctr,  int _priority = 0);
//clean resources on this level and above (level 0 cleans all resources)
void CleanResources(int level);

//will attempt to unload resources only if there are no active references
void UnloadModelResource(const char *filename);
//add reference checker, garbage collectionish kinda thingamajogger.



#endif
