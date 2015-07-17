/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */

#include "Resources.h"
#include "Utilities.h"

#include "ShaderResource.h"
#include "ModelResource.h"
#include "AnimationResource.h"
#include "Builder.h"
#include "Entity.h"
#include "Materials.h"
#include "TMXLoader.h"
#include "Builder.h"
//we need some sort of limits
#define MAX_TEXTURE_COUNT 1000
#define MAX_SHADER_COUNT 1000
#define MAX_MODEL_COUNT 1000
#define MAX_ANIMATED_MODEL_COUNT 1000
#define MAX_ANIMATION_COUNT 1000
#define MAX_BLUEPRINT_COUNT 1000
#define MAX_SOUNDS_COUNT 1000
#define MAX_SPRITES_COUNT 1000

//using namespace std;

//this is an APK resource loader provided by the NDK's native-activity
#if defined(__ANDROID__)
void SetAndroidAssetManager(AAssetManager *mgr)
{
	G_AssetManager = mgr;
}
AAssetManager *GetAndroidAssetManager()
{
    return G_AssetManager;
}
#endif


//TODO - TEST THIS!!  most notably references and priorities
TextureResource *LoadTexture( const char *filename, int forceChannels, unsigned int reuseID, unsigned int flags, int _priority)
{
    //check to see if texture is already loaded
    std::map<std::string, TextureResource*, StringCompareForMap>::iterator iter = Textures.find(std::string(filename));
    
    if (iter != Textures.end())
    {
        TextureResource *tex = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (tex->GetPriority() > _priority)
            tex->SetPriority(_priority);
        tex->AddRef();
        return tex;
    }
    else
    { 
        //try to load texture
    	LOGOUTPUT << "Loading Texture : " << filename;
        LogInfo();
        GLuint texID = 0;
        
       // int width, height, channels;
       
#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(G_AssetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        unsigned int size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size];
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        texID = SOIL_load_OGL_texture_from_memory(buff, size, forceChannels, reuseID, flags);
        delete []buff;
#else
        texID = SOIL_load_OGL_texture(filename, forceChannels, reuseID, flags);
#endif
        if (texID == 0) //error, dag!
        {
            LogError("Couldnt load texture from file:");
            LogError(filename);
            return 0;
        }
        else
        {

            //create the new texture
            TextureResource *newTex = new TextureResource(filename, texID, _priority);
            if (!newTex)
            {
                LogError("error creating new texture resource");
                LogError(filename);
                return 0;
            }
            newTex->AddRef();
            //insert, and return
            Textures[filename] = newTex;

            LogInfo("Load File Success!!");

          //  newTex->SetWidth(width);
          //  newTex->SetHeight(height);
          //  newTex->SetChannels(channels);
           
            return newTex;   
        }
    }
    
 
    LogError("Something completely fucked up happened...");
    return 0;
}

ModelResource* LoadModelPrimitive (ModelPrimitive primType, int _priority)
{
    //TODO rewrite primitives if the engine needs them
    LogError("Model Primitives need ot be rewritten!  so do that.");
    return 0;
    //check validity
    if (primType < 0 || primType > MDL_NUM_PRIMITIVES)
        return 0;
}


ModelResource *LoadModel(const char *filename, bool collisionMesh, int _priority)
{
    if (!filename)
        return 0;

    LogInfo("Loading Model...");
    
    //check if its already loaded using primitive strings
    map<string, ModelResource *, StringCompareForMap>::iterator iter = Models.find(filename);
    
    //found it
    if (iter != Models.end())
    {
        ModelResource *model = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (model->GetPriority() > _priority)
            model->SetPriority(_priority);
        model->AddRef();
        return model;
    }
    else
    {
        ModelResource *model = new ModelResource(_priority);
        bool loaded = false;

#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(G_AssetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        unsigned int size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size];
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        ResourceLoadMemInfo mInfo;
        mInfo.size = size;
        mInfo.mem = buff;
        loaded = model->LoadObj(filename, collisionMesh, &mInfo);
        delete []buff;
#else
       loaded = model->LoadObj(filename, collisionMesh);
#endif
            
        if (!loaded)
        {
            model->Delete();
            return 0;
        }

        Models[filename] = model;
        return model;
    }
    return 0;
}

ShaderResource  *LoadShaderResource(const char *filename, int _priority) //shaders should probably all be level 0 as there will not be many of them anyway
{
     //check to see if texture is already loaded
    map<string, ShaderResource*, StringCompareForMap>::iterator iter = Shaders.find(filename);
    
    if (iter != Shaders.end())
    {
        ShaderResource *shader = iter->second;
        //if we are setting with lower priority value, update that shaders priority level..
        if (shader->GetPriority() > _priority)
            shader->SetPriority(_priority);
        shader->AddRef();
        return shader;
    }
    else
    {
        LOGOUTPUT << "\n######################################################################\n";
        LOGOUTPUT << "Loading Shader: " << filename << "\n";
        LogInfo();
#if defined(__ANDROID__) 
        AAsset* asset = AAssetManager_open(G_AssetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        size_t size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size+1];//fmemopen needs an extra byte
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        ResourceLoadMemInfo mInfo;
        mInfo.size = size;
        mInfo.mem = buff;
        ShaderResource *shader = LoadShaderFile((const char*)buff, _priority, &mInfo);
        delete []buff;
#else
       ShaderResource *shader = LoadShaderFile(filename, _priority, 0);
       LogInfo("\n######################################################################\n");
         
#endif
        if (shader == 0) //error, dag!
        {
            LogError("Couldnt load shader from file:");
            LogError(filename);
            return 0;
        }
        else
        {
            //insert, and return
            Shaders[filename] = shader;
            shader->AddRef();      
            LogInfo("Load Shader File Success!!");

            return shader;   
        }
    }
    
 
    LogError("Something completely fucked up happened loading a shader resource...");
    return 0;
}



AnimatedModelResource *LoadAnimatedMesh(const char *filename, int _priority)
{
    if (!filename)
        return 0;
    
    //check if its already loaded using primitive strings
    map<string, AnimatedModelResource *, StringCompareForMap>::iterator iter = AnimatedMeshes.find(filename);
    
    //found it
    if (iter != AnimatedMeshes.end())
    {
        AnimatedModelResource *model = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (model->GetPriority() > _priority)
            model->SetPriority(_priority);
        model->AddRef();
        return model;
    }
    else
    {
        AnimatedModelResource *model = new AnimatedModelResource(_priority);
        bool loaded = false;
#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(G_AssetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        unsigned int size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size];
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        ResourceLoadMemInfo mInfo;
        mInfo.size = size;
        mInfo.mem = buff;
        loaded = model->LoadMd5(filename, &mInfo);
        delete []buff;
#else
       loaded = model->LoadMd5(filename);
#endif
     
            if (!loaded)
            {
                model->Delete();
                return 0;
            }
       
        
        AnimatedMeshes[filename] = model;
        return model;
    }
    return 0;
}


AnimationResource *LoadAnimation(const char *filename, int _priority)
{
    if (!filename)
        return 0;
    
    //check if its already loaded using primitive strings
    map<string, AnimationResource *, StringCompareForMap>::iterator iter = Animations.find(filename);
    
    //found it
    if (iter != Animations.end())
    {
        AnimationResource *anim = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (anim->GetPriority() > _priority)
            anim->SetPriority(_priority);
        anim->AddRef();
        return anim;
    }
    else
    {
        AnimationResource *anim = new AnimationResource(_priority);
        if (!anim->LoadAnimation(filename))
            return 0;
     
        Animations[filename] = anim;
        return anim;
    }
    return 0;
}


BlueprintResource *LoadBlueprint(const char *filename, int _priority)
{
     if (!filename)
        return 0;
    
    //check if its already loaded using primitive strings
    map<string, BlueprintResource *, StringCompareForMap>::iterator iter = Blueprints.find(filename);
    
    //found it
    if (iter != Blueprints.end())
    {
        BlueprintResource *bp = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (bp->GetPriority() > _priority)
            bp->SetPriority(_priority);
        bp->AddRef();
        return bp;
    }
    else
    {
#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(G_AssetManager, filename, AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        unsigned int size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size];
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        ResourceLoadMemInfo mInfo;
        mInfo.size = size;
        mInfo.mem = buff;
        BlueprintResource *bp = Builder::GetInstance()->CreateBlueprint(filename, _priority, &mInfo);
        delete []buff;
#else
        BlueprintResource *bp = Builder::GetInstance()->CreateBlueprint(filename, _priority);
#endif
        if (!bp)
        {
            LOGOUTPUT << "Could not load blueprint: " << filename;
            LogError();
            return 0;
        }
        Blueprints[filename] = bp;
        return bp;
    }
    return 0;
}

SoundResource *LoadSound(const char *filename, SoundTypes type, int _priority)
{
    if (!filename)
        return 0;
    
    if (type < 0 || type >= SND_NUMTYPES)
    {
        LogError("invalid sound type.");
        return 0;
    }
    
     //check if its already loaded using primitive strings
    map<string, SoundResource *, StringCompareForMap>::iterator iter = Sounds.find(filename);
    
    //found it
    if (iter != Sounds.end())
    {
        SoundResource *snd = iter->second;
        //if we are setting with lower priority value, update that textures priority level..
        if (snd->GetPriority() > _priority)
            snd->SetPriority(_priority);
        snd->AddRef();
        return snd;
    }
    else
    {
    	//TODO get this working for android
       /* SoundResource *snd = 0;
        
        if (type == SNDTYPE_WAV)
        {
            //load new resource file, if fails delete and return null
            snd = new SoundResource;
            if (!snd->LoadSoundWav(filename))
            {
                snd->Delete();
                snd = 0;
            }
            
            Sounds[filename] = snd;
            return snd;
        }*/
    }
    return 0;
}
SpriteResource::~SpriteResource()
{  /*if (positions) delete[] positions; if (texCoords) delete[]texCoords;*/
    if (numFrames > 1) delete[] frames;
    else if (numFrames == 1) delete frames;
    if (vertPositions > 0) glDeleteBuffers(1, &vertPositions);
    if (vertTexCoords > 0) glDeleteBuffers(1, &vertTexCoords);
    if (material) delete material;
}

SpriteResource::SpriteResource ( Material* mat, SpriteConstructor *ctr, int _priority ) : Resource ( _priority )
{
    material = mat;
    frames = ctr->frames; 
    numFrames = ctr->numFrames;
    priority = _priority; 
    
    Vector2 adjustedPosition = ctr->offset;

    if (!ctr->tiled)
    {
        glGenBuffers(1, &vertPositions);
        glGenBuffers(1, &vertTexCoords);
        halfWidths = ctr->dimensions;
        numVerts = 6;
        float _posData[6][2] =  {
                                {  adjustedPosition.x- halfWidths.x,  adjustedPosition.y- halfWidths.y },
                                {  adjustedPosition.x+ halfWidths.x,  adjustedPosition.y- halfWidths.y },
                                {  adjustedPosition.x+ halfWidths.x,  adjustedPosition.y+ halfWidths.y },
                                {  adjustedPosition.x+ halfWidths.x,  adjustedPosition.y+ halfWidths.y },
                                {  adjustedPosition.x- halfWidths.x,  adjustedPosition.y+ halfWidths.y },
                                {  adjustedPosition.x- halfWidths.x,  adjustedPosition.y- halfWidths.y }
                                };
        Vector2 *posData = new Vector2[numFrames * numVerts];
        Vector2 *tcData = new Vector2[numFrames * numVerts]; //num verts will pretty much always be 6 i think
        int z = 0; 
        for (int i = 0; i < numFrames; i++)
        {
            memcpy(&posData[z], _posData, numVerts * sizeof(Vector2));
            tcData[z].x = frames[i].texcoords.x;                          tcData[z++].y = frames[i].texcoords.y;
            tcData[z].x = frames[i].texcoords.x + frames[i].dimensions.x; tcData[z++].y = frames[i].texcoords.y;
            tcData[z].x = frames[i].texcoords.x + frames[i].dimensions.x; tcData[z++].y = frames[i].texcoords.y + frames[i].dimensions.y;
            tcData[z].x = frames[i].texcoords.x + frames[i].dimensions.x; tcData[z++].y = frames[i].texcoords.y + frames[i].dimensions.y;
            tcData[z].x = frames[i].texcoords.x;                          tcData[z++].y = frames[i].texcoords.y + frames[i].dimensions.y;
            tcData[z].x = frames[i].texcoords.x;                          tcData[z++].y = frames[i].texcoords.y;
        }
                        
        //send position data to gpu
        glBindBuffer(GL_ARRAY_BUFFER, vertPositions);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_POSITION0), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * numFrames * numVerts, posData, GL_STATIC_DRAW); //change to dynamic if you need to animate
        
        //send texcoord data to gpu
        glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_TEXCOORD0)); //vertpos location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_TEXCOORD0), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * numFrames * numVerts, tcData, GL_STATIC_DRAW); //change to dynamic if you need to animate

        delete[] tcData;
        delete[] posData;
    }
    else
    {
        if(!LoadTilemap(ctr->tilemapFile.c_str()))
        {
            LogError("could not load tmx file");
            return;
        }
        numFrames = 0;
        
        glGenBuffers(map.numLayers, map.vertPositions);
        glGenBuffers(map.numLayers, map.vertTexCoords);
        //use the sprite shader
        material = new Material;
        
        ShaderResource *shader = LoadShaderResource("Data/Shaders/Sprite.shader");
        material->LoadShader(shader);  //this will add a reference to the shader for us
        material->RegisterUniform("WorldMatrix", SV_ENTITY_TRANSFORM, (void *)1);  //TODO really realllly need to figure out shared materials soon here. 
        material->RegisterViewMatrix("ViewMatrix");
        material->RegisterProjectionMatrix("ProjectionMatrix");
        
        //get just the filename, and put in data path
        string File = map.texture;
        int startIdx = File.size();
        while (startIdx > 0)
        {
            if (File[startIdx] == '/')
                break;
            startIdx--;
        }    
        File.erase(0, startIdx);
        string resPath = string("Data/Textures") + File;
        TextureResource *tex = LoadTexture(resPath.c_str(), /*SOIL_LOAD_AUTO*/SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
                                    SOIL_FLAG_INVERT_Y /*| SOIL_FLAG_MIPMAPS */ | SOIL_FLAG_TEXTURE_REPEATS, priority);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        material->RegisterUniform("sampler", SV_SAMPLER2D, tex);    
        
        halfWidths = Vector2(5, 5);
        
        int numCells = map.width * map.height;
        numVerts = numCells * 6 ;
        Vector2 *posData = new Vector2[numVerts];
        Vector2 *tcData = new Vector2[numVerts];
        
        //positions start at the center of the upper left tile
        adjustedPosition.x = (map.width-1) * - halfWidths.x;
        adjustedPosition.y = (map.height-1) * halfWidths.y;
        float _posData[12];
        float _tcData[12];
        Vector2 start = adjustedPosition;
        
        //hax
        float yBias = 0.0000022;
        //float xBias = 0.000042; //when changing aspect ratios, theres some funkiness, texcoords bleeding over cells
        
        //texcoords
        Vector2 tcPos = Vector2(0.0, 1.0 + yBias);
        Vector2 tcInc = Vector2( 1.0 / (map.imageWidth / map.tileWidth) ,  1.0 / (map.imageHeight / map.tileHeight));
        
        unsigned int idx = 0;
        unsigned int tilmapCols = map.imageWidth / map.tileWidth;
        for (int layer = 0; layer < map.numLayers; layer++)
        {
            int vertCounter = 0;
            for (int y = 0; y < map.height; y++)
            {
                for (int x = 0; x < map.width; x++)
                {
                    short tile = map.layerData[idx++];
                    if (tile < 0)
                    {
                        //numVerts -= 6; //remove unused tiles from the buffers
                        adjustedPosition.x += halfWidths.x * 2;
                        continue;
                    }
                    _posData[0] = adjustedPosition.x - halfWidths.x;
                    _posData[1] = adjustedPosition.y - halfWidths.y; 
                    _posData[2] = adjustedPosition.x + halfWidths.x;  
                    _posData[3] = adjustedPosition.y - halfWidths.y; 
                    _posData[4] = adjustedPosition.x + halfWidths.x;  
                    _posData[5] = adjustedPosition.y + halfWidths.y;
                    _posData[6] = adjustedPosition.x + halfWidths.x; 
                    _posData[7] = adjustedPosition.y + halfWidths.y; 
                    _posData[8] = adjustedPosition.x - halfWidths.x; 
                    _posData[9] = adjustedPosition.y + halfWidths.y; 
                    _posData[10] =adjustedPosition.x - halfWidths.x;
                    _posData[11] =adjustedPosition.y - halfWidths.y;
                    
                    tcPos.x = (float)(tile % tilmapCols) * tcInc.x ;
                    tcPos.y = 1.0 - ((float)(tile / tilmapCols) * tcInc.y -yBias );
                    
                    //bottom left
                    _tcData[0] = tcPos.x;
                    _tcData[1] = tcPos.y - tcInc.y;
                    //bottom right
                    _tcData[2] = tcPos.x + tcInc.x;
                    _tcData[3] = tcPos.y - tcInc.y;
                    //top right
                    _tcData[4] = tcPos.x + tcInc.x; 
                    _tcData[5] = tcPos.y;
                    //top right
                    _tcData[6] = tcPos.x + tcInc.x; 
                    _tcData[7] = tcPos.y;
                    //top left
                    _tcData[8] = tcPos.x;
                    _tcData[9] = tcPos.y;
                    //bottom left
                    _tcData[10] = tcPos.x;
                    _tcData[11] = tcPos.y - tcInc.y;
                    
                    memcpy(&posData[vertCounter], _posData, sizeof(float) * 12);
                    memcpy(&tcData[vertCounter], _tcData, sizeof(float) * 12);
                  
                    adjustedPosition.x += halfWidths.x * 2;
                    vertCounter+=6;
                }
                adjustedPosition.x = start.x;
                adjustedPosition.y -= halfWidths.y * 2;
            }
            adjustedPosition = start;
            //send position data to gpu
            glBindBuffer(GL_ARRAY_BUFFER, map.vertPositions[layer]);
            glEnableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
            glVertexAttribPointer(material->GetAttributeLocation(ATR_POSITION0), 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vertCounter, posData, GL_STATIC_DRAW); //change to dynamic if you need to animate
        
            //send texcoord data to gpu
            glBindBuffer(GL_ARRAY_BUFFER, map.vertTexCoords[layer]);  
            glEnableVertexAttribArray(material->GetAttributeLocation(ATR_TEXCOORD0)); //vertpos location    
            glVertexAttribPointer(material->GetAttributeLocation(ATR_TEXCOORD0), 2, GL_FLOAT, GL_FALSE, 0, 0);    
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vertCounter, tcData, GL_STATIC_DRAW); //change to dynamic if you need to animate

            map.vertCount[layer] = vertCounter;
        }          
       
        delete[] tcData;
        delete[] posData;
    }
}

bool SpriteResource::LoadTilemap ( const char* filename )
{
#if defined(PC_PLATFORM)
    TMXLoader tmx;
    if (!tmx.LoadMap(filename))
        return false;

    map.height = tmx.mapHeight;
    map.width = tmx.mapWidth;
    map.layerData = tmx.layerData; //we are responsible for layerData
    map.numLayers = tmx.numLayers;
    map.tileHeight = tmx.tileHeight;
    map.tileWidth = tmx.tileWidth;
    map.texture = tmx.textureFile;
    map.imageHeight = tmx.imageHeight;
    map.imageWidth = tmx.imageWidth;
    map.loaded = true;
    
    return true;
#endif
    return false;
}


SpriteResource *LoadSpriteResource(std::string name, Material *mat, SpriteConstructor *ctr, int _priority)
{
    map<std::string, SpriteResource *, StringCompareForMap>::iterator iter = Sprites.find(name);
    if (iter != Sprites.end())
    {
        SpriteResource *sprite = iter->second;
        if (sprite->GetPriority() > _priority)
            sprite->SetPriority(_priority);
        sprite->AddRef();
        return sprite;
    }
    else
    {
        if (!mat)
            return 0;
        SpriteResource *sprite = new SpriteResource(mat, ctr, _priority);
        Sprites[name] = sprite;
        return sprite;
    }
}



void CleanResources ( int level )
{
    LogInfo("Cleaning Resources...");
    //go through all resources, if priority >= level, delete it, regardless of reference count.
    
    //remove these in a SMART order, everything is reference counter so a model could contain textures, and shaders, etc..
    //and blueprints contain everything
    map<string, BlueprintResource*, StringCompareForMap>::iterator bpIter = Blueprints.begin();
    map<string, BlueprintResource*, StringCompareForMap>::iterator bpDeleteMe;
    int count = 0;
    LogInfo("Cleaning blueprints");
    while(bpIter != Blueprints.end())
    {
        bpDeleteMe = bpIter;
        bpIter++;

        bpDeleteMe->second->Delete();
        Blueprints.erase(bpDeleteMe);

        count++;
        if (count >= MAX_BLUEPRINT_COUNT)
        {
            LogError("Exceeded Max Blueprints!?!?");
            break;
        }
    }
    
    map<string, ModelResource*, StringCompareForMap>::iterator modelIter = Models.begin();
    map<string, ModelResource*, StringCompareForMap>::iterator modelDeleteMe;
    count = 0;
    LogInfo("Cleaning models");
    while(modelIter != Models.end())
    {
        modelDeleteMe = modelIter;
        modelIter++;
        
        modelDeleteMe->second->Delete();
        Models.erase(modelDeleteMe);
        count++;
        if (count >= MAX_MODEL_COUNT)
        {
            LogError("Exceeded Max Models!?!?");
            break;
        }
    }
    
    map<string, AnimatedModelResource*, StringCompareForMap>::iterator animMdlIter = AnimatedMeshes.begin();
    map<string, AnimatedModelResource*, StringCompareForMap>::iterator animMdlDeleteMe;
    count = 0;
    LogInfo("Cleaning animated meshes");
    while(animMdlIter != AnimatedMeshes.end())
    {
        animMdlDeleteMe = animMdlIter;
        animMdlIter++;
        
        animMdlDeleteMe->second->Delete();
        AnimatedMeshes.erase(animMdlDeleteMe);
        count++;
        if (count >= MAX_ANIMATED_MODEL_COUNT)
        {
            LogError("Exceeded Max animated models!?!?");
            break;
        }
    }
    
    map<string, AnimationResource*, StringCompareForMap>::iterator animIter = Animations.begin();
    map<string, AnimationResource*, StringCompareForMap>::iterator animDeleteMe;
    count = 0;
    LogInfo("Cleaning animations");
    while(animIter != Animations.end())
    {
        animDeleteMe = animIter;
        animIter++;
        
        animDeleteMe->second->Delete();
        Animations.erase(animDeleteMe);
        count++;
        if (count >= MAX_ANIMATION_COUNT)
        {
            LogError("Exceeded Max animations!?!?");
            break;
        }
    }
    
    
    
    
    map<string, SpriteResource*, StringCompareForMap>::iterator sprIter = Sprites.begin();
    map<string, SpriteResource*, StringCompareForMap>::iterator sprDeleteMe;
    count = 0;
    LogInfo("Cleaning sprites");
    while(sprIter != Sprites.end())
    {
        sprDeleteMe = sprIter;
        sprIter++;
        
        sprDeleteMe->second->Delete();
        Sprites.erase(sprDeleteMe);
        count++;
        if (count >= MAX_SPRITES_COUNT)
        {
            LogError("Exceeded Max Sprites!?!?");
            break;
        }
    }
    

    
    map<string, SoundResource*, StringCompareForMap>::iterator sndIter = Sounds.begin();
    map<string, SoundResource*, StringCompareForMap>::iterator sndDeleteMe;
    count = 0;
    LogInfo("Cleaning sounds");
    while(sndIter != Sounds.end())
    {
        sndDeleteMe = sndIter;
        sndIter++;
        
        ((Resource *)sndDeleteMe->second)->Delete();
        Sounds.erase(sndDeleteMe);
        count++;
        if (count >= MAX_SOUNDS_COUNT)
        {
            LogError("Exceeded Max Sounds!?!?");
            break;
        }
    }
    
    map<string, TextureResource*, StringCompareForMap>::iterator texIter = Textures.begin();
    map<string, TextureResource*, StringCompareForMap>::iterator texDeleteMe;
    count = 0;
    LogInfo("Cleaning textures");
    while(texIter != Textures.end())
    {
        texDeleteMe = texIter;
        texIter++;

        texDeleteMe->second->Delete();
        Textures.erase(texDeleteMe);
        count++;
        if (count >= MAX_TEXTURE_COUNT)
        {
            LogError("Exceeded Max Textures!?!?");
            break;
        }
    }
    
    map<string, ShaderResource*, StringCompareForMap>::iterator shaderIter = Shaders.begin();
    map<string, ShaderResource*, StringCompareForMap>::iterator shaderDeleteMe;
    count = 0;
    LogInfo("Cleaning shaders");
    while(shaderIter != Shaders.end())
    {
        shaderDeleteMe = shaderIter;
        shaderIter++;
        
        shaderDeleteMe->second->Delete();
        Shaders.erase(shaderDeleteMe);
        count++;
        if (count >= MAX_SHADER_COUNT)
        {
            LogError("Exceeded Max Shaders!?!?");
            break;
        }
    }

    LogInfo("Resources Cleaned!");
    
}

