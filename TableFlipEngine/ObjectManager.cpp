/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Utilities.h"
#include "ObjectManager.h"
#include "Scripting.h"
#include "Renderer.h"
#include "Resources.h"
#include "Builder.h"
//#include "SoundSystem.h"
using std::vector;
ObjectManager *ObjectManager::instance = 0;

//Model pond;
GLuint pondTex = -1;
ObjectManager::ObjectManager()
{

}
ObjectManager::~ObjectManager()
{}

void ObjectManager::InitInstance()
{
    if (!instance)
        instance = new ObjectManager;
}

void ObjectManager::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

bool ObjectManager::Init()
{
    //testNode = OctreeBuild(Vector3(0.0f, 0.0f, 0.0f), 200.0f, 2);
    inUpdate = false;
    delayedUnload = false;
    delayedLoad = false; 
    return true;
}

Entity* ObjectManager::GetEntity(string _name)
{
    for (int i = 0; i < worldObjects.size(); i++)
        if (worldObjects[i]->name == _name)
            return worldObjects[i];

        return 0;
}


void ObjectManager::Add (Entity* obj)
{ 
    worldObjects.push_back(obj); 
}

void ObjectManager::Remove (Entity* obj)
{
    vector<Entity *>::iterator iter;
    
    for (iter = worldObjects.begin(); iter != worldObjects.end(); iter++)
    {
        if (*iter == obj)
        {
            Logic *logic = obj->GetLogic();
            if (logic)
                Remove(logic);
            worldObjects.erase(iter);
            return;            
        }
    }
}

void ObjectManager::Add (Logic* logic)
{
    if (inUpdate)
        logicAddQueue.push_back(logic);
    else
        logicComponents.push_back(logic);
}

void ObjectManager::Remove (Logic* obj)
{
    vector<Logic *>::iterator iter;
    
    for (iter = logicComponents.begin(); iter != logicComponents.end(); iter++)
    {
        if (*iter == obj)
        {
            if (inUpdate)
                logicRemovalQueue.push_back(obj); //add to queue and come back when we arent in main update loop
            else
                logicComponents.erase(iter);
            return;            
        }
    }
}

vector<Entity *>::iterator ObjectManager::RemoveAndDelete (Entity* obj)
{
    vector<Entity *>::iterator iter;
    
    for (iter = worldObjects.begin(); iter != worldObjects.end(); iter++)
    {
        if (*iter == obj)
        {
            //(*iter)->DestroyAndRelease(); //never call delete on this, script engine no-no
            //good thing i ripped the script engine out.
            //TODO safe removal should make sure we are not mid update when shit gets deleted
            Logic *logic = obj->GetLogic();
            if (logic)
                Remove(logic);
            (*iter)->DestroyAndRelease();
            return worldObjects.erase(iter);
            
                      
        }
    }
    //TODO this is just plain wrong i feel like, 
    return iter;
}

void ObjectManager::Update(float dt)
{
    //TODO how to optimize this...  many ways, its just thrown together for now
    
    inUpdate = true;
    
   // Scripting * scrpt = Scripting::GetInstance();
    //player.Update(dt);

    vector<Logic *>::iterator iter;
    Logic *logic;
    for (iter = logicComponents.begin(); iter != logicComponents.end() ; iter++ )
    {
        //update logic
        logic = (*iter);
        if (logic->isScripted())
        {
#if defined(PC_PLATFORM) && !defined(__APPLE__)
            //Scripting::GetInstance()->CallUpdate(logic->entity->GetScriptCtrlAddress());
#endif
        }
        else
        {
            //logic wants dt in seconds not millis
            logic->Update(dt);
        }
    }
    
    //animations
    vector<Entity *>::iterator objIter;
    //this is bad, TODO do not iterate all the f'n objects !!!
     for (objIter = worldObjects.begin(); objIter != worldObjects.end(); objIter++)
    {
        if ((*objIter)->GetAnimationController())
        {
            (*objIter)->GetAnimationController()->Update(dt);      
            (*objIter)->GetSkeletalMesh()->UpdatePose();
        }
    }
    
    
    inUpdate = false;
    if (delayedUnload)
        DoUnload();
    
    if (delayedLoad)
    {
        //URRRRGGGHH ANDROID
#if defined(__ANDROID__)
        AAsset* asset = AAssetManager_open(GetAndroidAssetManager(), worldFile.c_str(), AASSET_MODE_UNKNOWN);
        if (asset == 0)
            LogError("Android Asset Load Error, check path?");

        unsigned int size = AAsset_getLength(asset);
        unsigned char *buff = new unsigned char[size];
        AAsset_read(asset, buff, size);
        AAsset_close(asset);
        ResourceLoadMemInfo mInfo;
        mInfo.size = size;
        mInfo.mem = buff;
        DoLoadWorldFile(worldFile.c_str(), &mInfo);
        delete []buff;
#else
        DoLoadWorldFile(worldFile.c_str());
#endif
    }
   
   while (safeDelete.size())
   {
       Entity *delMe = safeDelete.back();
       safeDelete.pop_back();
       
       //need to end all collisions before we delete.
       Physics::GetInstance()->RemoveFromContacts(delMe);
       RemoveAndDelete(delMe);
   }

   while (logicRemovalQueue.size())
   {
       Remove(logicRemovalQueue.back());
       logicRemovalQueue.pop_back();
   }
   while (logicAddQueue.size())
   {
       Add(logicAddQueue.back());
       logicAddQueue.pop_back();
   }
       
       
    
}


void ObjectManager::Shutdown()
{
   // Scripting * scrpt = Scripting::GetInstance();
    int i;
    for (i = 0; i < worldObjects.size(); i++)
    {
         LOGOUTPUT << i << " removing entity: " << worldObjects[i]->name;
    LogInfo();
       //TODO make this scripting safe,  just cleaning up while i test
       worldObjects[i]->DestroyAndRelease();
    }
    worldObjects.clear();
  
    LOGOUTPUT << "Deleted " << i << " Entities!";
    LogInfo();
   // OctreeDeleteChildren(testNode);
  //  delete testNode;
}


void ObjectManager::TestSpawn()
{
   /* Fish *newObj = new Fish();
    newObj->Init();
    newObj->Position.y = 12.0f;
    newObj->CopyModelData(&fishModel);
    newObj->scriptObject = Scripting::GetInstance()->CreateScriptObject("GoldenFish", newObj);
    if (newObj->scriptObject)
        Add(newObj); 
    else
        delete newObj;*/
}

void ObjectManager::LoadWorldFile(const char* filename, void (*OnLoad)(void))
{
    delayedLoad = true;
    worldFile = filename;
    LoadCallback = OnLoad;
}

void ObjectManager::DoLoadWorldFile(const char* filename, ResourceLoadMemInfo *mem)
{
#if defined(__ANDROID__)
    FILE * file = fmemopen(mem->mem, mem->size, "r");
#else
    FILE * file = fopen(filename, "r");
#endif
    if( file == NULL ){
        LogError("Impossible to open the file !\n");
        return;
    }

    char buff[128];
    char stringRead[128];
    float floatRead[4];
        
    //get just the filename for the tag plz.
    int rewindit = 0;
    unsigned int len = (unsigned int)strlen(filename);
    for (int i = len; i >=0; i--)
    {
        
        if (filename[i] == '/')
            break;
        else
            rewindit = i;
    }
    std::string tag = &filename[rewindit];
    // first 2 lines are comments eat em up
    //fgets(buff, sizeof(buff), file);
   // if (!fgets(buff, sizeof(buff), file))     
   // {
   //     LogError("Corrupt or incomplete XFW file?");
   //     return;
    //}
    
    static unsigned int meshCount = 1; //TODO this is kindof a hack i think, bettr work..
   
    while( 1 ){//TODO oh yeah and infinite loops are bad news bears.
        
        //chomp chomp
        if (!fgets(buff, sizeof(buff), file))     
        {
            LogError("Corrupt or incomplete XFW file? top of \n^Not an actual error FIXTHIS!");
            break;
        }
        
        int res = sscanf(buff, "object: %s\n", stringRead);
        if (res == EOF) //TODO i think this should be <=0
            break; // EOF = End Of File. Quit the loop.

        
        //read entity types
        if ( strcmp( stringRead, "entity" ) == 0 )
        {
	 
            if (!fgets(buff, sizeof(buff), file))     
            {
                LogError("Corrupt or incomplete XFW file? entity");
                return;
            }
            //blueprint: %s
            sscanf(buff, "blueprint: %s", stringRead);
            std::string filepath = std::string("Data/Blueprints/") + stringRead;
            filepath += ".bp";
            
            if (!fgets(buff, sizeof(buff), file))     
            {
                LogError("Corrupt or incomplete XFW file? blueprint");
                return;
            }
            //position
            sscanf(buff, "position: %f %f %f", &floatRead[0], &floatRead[1], &floatRead[2]);
            Vector3 pos = Vector3(floatRead[0], floatRead[1], floatRead[2]);
            LOGOUTPUT << "building new entity: " << filepath << " - At position: " << pos.x << ", " << pos.y << ", " << pos.z;
            LogInfo();
            BlueprintResource *bp = LoadBlueprint(filepath.c_str());
            Entity *newEnt = Builder::GetInstance()->ConstructEntity(bp, pos);
            newEnt->worldTag = tag;
            LOGOUTPUT << "Entity built: " << newEnt->name;
            LogInfo();
            this->Add(newEnt);
            
        }
        else if ( strcmp( stringRead, "static_mesh") == 0)
        {
            LogWarning("loading static mesh");
            size_t fpos;
            fpos = ftell(file);
            if (!fgets(buff, sizeof(buff), file))     
            {
                LogError("Corrupt or incomplete XFW file? static mesh");
                return;
            }
            
            //shader
            sscanf(buff, "shader: %s", stringRead);
            std::string shaderFile = std::string("Data/Shaders/");
            shaderFile += stringRead;
            fpos = ftell(file);
            
            if (!fgets(buff, sizeof(buff), file))     
            {
                LogError("Corrupt or incomplete XFW file? shader");
                return;
            }
            
            //texture        
            std::string texFile;
            if (sscanf(buff, "texture: %s", stringRead) == 1)
                texFile = std::string("Data/Textures/") + stringRead;
            else
                fseek(file, fpos, SEEK_SET); //no texture, rewind line
            if (strcmp(stringRead, "default") == 0)
                texFile = "";
            std::ostringstream name;
            name << "static_mesh" << meshCount++;
           
           
            ModelResource *newMdl = new ModelResource(0);
            newMdl->LoadXFWStream(file, name.str().c_str(), true); //hardcoded everything to have collision mesh at the moment.
           
            //add to the static world
            //create house
            Entity *newObj = new Entity;
            newObj->name = name.str();
            newObj->worldTag = tag;
            Material *mat = new Material;
            mat->LoadShader(shaderFile.c_str());
            Mesh *newMesh = new Mesh(newObj, newObj->GetTransform(), newMdl, mat);

            TextureResource *newTex = 0;
            if (texFile.length() > 0)
                newTex = LoadTexture(texFile.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
                SOIL_FLAG_INVERT_Y | SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS, 0);  //TODO these are png loader flags... needs moar dynamic
           
            newObj->AttachComponent(CMP_MESH, newMesh);
            newObj->GetTransform()->Update();
            
           
            newObj->GetMesh()->material->SetEntity(newObj); //needed for trigger callbacks i believe
           
            newObj->GetMesh()->material->RegisterViewMatrix("ViewMatrix");
           
            newObj->GetMesh()->material->RegisterProjectionMatrix("ProjectionMatrix");
           
            if (!newObj->GetMesh()->material->RegisterUniform("WorldMatrix", SV_ENTITY_TRANSFORM, 0))
            {
                LogError("Failed to register world matrix");
                return;
            }

            if (newTex)
                newObj->GetMesh()->material->RegisterUniform("sampler", SV_SAMPLER2D, newTex);
           
            staticWorldEntities.push_back(newObj);
            staticWorldMeshes.push_back(newMdl);
            
            Physics::GetInstance()->AddStaticGeometry(name.str().c_str(), Vector3(0,0,0), newMdl);
            
        }
    }
    
    fclose(file);
    
    delayedLoad = false;

    //callback, where initialization can take place
    if (LoadCallback)
        LoadCallback();
}

void ObjectManager::DoUnload()
{
    std::vector<Entity *>::iterator iter = staticWorldEntities.begin();
    while(iter != staticWorldEntities.end())
    {   
        Entity *ent = *iter;
        
        if (unloadTag.size())
        {
            if (ent->worldTag != unloadTag)
            {
                iter++;
                continue;
            }
        }
        iter = staticWorldEntities.erase(iter);
  
        Logic *logic = ent->GetLogic();
        if (logic)
            Remove(logic);
       
        Physics::GetInstance()->RemoveStaticGeometry(ent->name);
        ent->DestroyAndRelease();
        
    }
    iter = worldObjects.begin();
    while(iter != worldObjects.end())
    {   
        Entity *ent = *iter;
        
        if (unloadTag.size())
        {
            if (ent->worldTag != unloadTag)
            {
                iter++;
                continue;
            }
        }
        iter = worldObjects.erase(iter);
        ent->DestroyAndRelease();
        
    }
    //XXX not even going to screw around with these right now...
    /*std::vector<ModelResource *>::iterator mIter = staticWorldMeshes.begin();
    while(iter != staticWorldEntities.end())
    {   
        ModelResource *mdl = *mIter;
        
        if (unloadTag.size())
        {
            if (mdl->worldTag != unloadTag)
            {
                mIter++;
                continue;
            }
        }
        mIter = staticWorldEntities.erase(mIter);   
        mdl->RemoveRef();
        
    }*/
    
    delayedUnload = false;
}

void ObjectManager::UnloadWorld(std::string tag)
{
   delayedUnload = true;
   unloadTag = tag;
    //TODO  probably should have some sort of tagging method to identify entities loaded
    //from the xfw file, this is where i would search for the tag, and delete those entities
    //that should not remain in scene.
}

