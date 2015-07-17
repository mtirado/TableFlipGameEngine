/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef OBJECTMANAGER_H__
#define OBJECTMANAGER_H__

//NOTE this whole thing is a big hack i needed to get the engine rolling
//some things may stay, and some will definitely change.
#include <vector>

#include "Entity.h"

class ObjectManager  //this class is only managing objects like this temporarily.  havent even really thought about what im going to do here yet.
{
private:
    
    //disable functions for singleton use
    ObjectManager();
    ~ObjectManager();
    ObjectManager(const ObjectManager &) {}
    ObjectManager &operator = (const ObjectManager &) { return *this; }


    //the singletons instance.
    static ObjectManager *instance;
    
    //TODO base class plz...  also vector is temp, use a map or something better
    std::vector<Entity *> worldObjects;
    std::vector<Logic *> logicComponents;
    
    //TODO - implement delayed deletions because logic components may trigger removals in update, while iterating the logic list
    //this will be a queue of what to remove, after the update has finished. as to prevent
    //invalidating the update iterator
    std::vector<Logic *> logicRemovalQueue;
    std::vector<Logic *> logicAddQueue;  //adding invalidates iterator as well!
    
    //for now i'm going to use a bool... arrgh
    bool delayedUnload;
    bool delayedLoad;
    void DoLoadWorldFile(const char* filename, ResourceLoadMemInfo *mem = 0);
    std::string worldFile;
    //void RemoveQueue(Logic *logic) { logicRemovalQueue.push_back(logic); }
    //void AddQueue(Logic *logic) { logicAddQueue.push_back(logic); }
    bool inUpdate;
    string unloadTag;

    void (*LoadCallback)(void);
public:
   
   // BVHOctreeNode *testNode;
    //Model fishModel;
       
    //inlines
    inline static ObjectManager *GetInstance() { return instance; }

    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();

    //initialize the render system
    bool Init();
    void Shutdown();
    //update all these foo's
    void Update(float dt);

    void Add(Entity *obj);
    void Remove(Entity *obj);
    void Add(Logic *obj);
    void Remove(Logic *obj);
    vector<Entity *>::iterator RemoveAndDelete(Entity *obj);

    Entity *GetEntity(string _name);
    
    //inline Entity *GetPlayer() { return (Entity *)(&player); }
    //test functions
    void TestSpawn();
    
    //experimenting with this
    //holds resources used for the world format
    std::vector <ModelResource *> staticWorldMeshes;
    std::vector <Entity *> staticWorldEntities; //cause i dont want ghosts floating around the engine, even if it is experimental stuff
    std::vector <Entity *> safeDelete;
    //NOTE yeah i know this is slow, just getting shit working so i can figure out what to do with it.
    //basically thats what this whole class is about, ideally i would like to eliminate it completely
    //TODO if im going this route, need to assign a world tag to all objects, so unloading the world
    //only unloads those objects, for now though, just keep adding them.
    //callback function will be called when the load finished execution in DoLoadWorldFile()
    void LoadWorldFile(const char *filename, void (*OnLoad)(void) = 0);
    
    //NOTE unload the current static world, ideally what i want TODO is be able to load / unload more than one xfw at a time
    void UnloadWorld(std::string tag = "");
    void DoUnload();
};

#endif
