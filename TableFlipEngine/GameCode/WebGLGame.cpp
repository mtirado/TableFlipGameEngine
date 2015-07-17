/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "WebGLGame.h"
#include "MiscLogic.hpp"
#include "Utilities.h"
#include "Renderer.h"
#include "UserInterface.h"
#include "UIButton.h"
#include "ObjectManager.h"
#include "Builder.h"
#include "SoundSystem.h"
#include "BoundingVolumes.h"
#include <unistd.h>


UIButton *testBtn;
Camera *mainCamera;
BlueprintResource *squidBp;
BlueprintResource *testBp;
BlueprintResource *sharkBp;
BlueprintResource *penguinBp;
BlueprintResource *testAnimBp;
SoundResource *catsMeow = 0;

//ship the camera is attached to, for gravity testing
BlueprintResource *bpCamShip;
BlueprintResource *bpThingy;

Entity *planet = 0;
Entity *player = 0;
Entity *test = 0;

Entity *gpuTest = 0;

unsigned int lastSpawn;
unsigned int spawnDelay = 0;
BVHOctreeNode *octree;
void OnLoad()
{
   
}

void GameInit()
{
    mainCamera = new Camera;
    mainCamera->Init();
    mainCamera->GetTransform()->position = Vector3(0.0, 0.0, 0.0);
    Renderer::GetInstance()->camera = mainCamera;
    //InitUI(0);
    

    octree = OctreeBuild(0, Vector3(0, 0, 0), 333.3333, 6);
    Renderer::GetInstance()->debugOctree = octree;

    for (int i = 0; i < 1295; i++)
    {
        Entity *test = new Entity;
        int range = 100;
        Vector3 pos = Vector3(-range+rand()%(range + range), -range+rand()%(range + range), -range+rand()%(range + range)); 
        test->GetTransform()->position = pos;
        test->GetTransform()->UpdatePosition();
        OctreeInsert(octree, test);
    }

    LogInfo("Loading world file");
    //ObjectManager::GetInstance()->LoadWorldFile("Data/Worlds/terrain.xfw", OnLoad);
    
   // penguinBp = LoadBlueprint("Data/Blueprints/penguin.bp");
    //testBp = LoadBlueprint("Data/Blueprints/box2d.bp");

   // bpThingy = LoadBlueprint("Data/Blueprints/testThing.bp");
    

    //Builder::GetInstance()->ConstructEntity(bpThingy)->GetRigidBody()->rigidBody->get

    //create dat penguin
   /* player = Builder::GetInstance()->ConstructEntity(penguinBp, Vector3(0,-5,20));        
    if (!player)
        LogError("Couldnt load player, catastrophic failure B");
    RockHopper *logic = new RockHopper();
    player->AttachComponent(CMP_LOGIC, logic);
    ObjectManager::GetInstance()->Add(player);
      InitUI((RockHopper *)player->GetLogic());
      ((RockHopper *)player->GetLogic())->Spawn();
      */
     // BlueprintResource *stbp = LoadBlueprint("Data/Blueprints/st-box2d.bp");
     // BlueprintResource *wall = LoadBlueprint("Data/Blueprints/wall.bp");
   //   BlueprintResource *tm = LoadBlueprint("Data/Blueprints/tilemap.bp");
     // Entity *w1 = Builder::GetInstance()->ConstructEntity(wall, Vector3(5, 50, 0));
    // // Entity *w2 = Builder::GetInstance()->ConstructEntity(wall, Vector3(95, 50, 0));
  
    //  Entity *world = Builder::GetInstance()->ConstructEntity(stbp, Vector3(50, 5, 0));

      //   Entity *map = Builder::GetInstance()->ConstructEntity(tm, Vector3(50, 50, -2.0));
   
    LogInfo("adding to object manager");
      //ObjectManager::GetInstance()->Add(w2);
     // ObjectManager::GetInstance()->Add(w1);
     // ObjectManager::GetInstance()->Add(world);
    
  //   ObjectManager::GetInstance()->Add(map);


   // player = Builder::GetInstance()->ConstructEntity(bpThingy, Vector3(0,20,0));
    //((Player *)player->GetLogic())->cam = mainCamera;
    
   // planet = Builder::GetInstance()->ConstructEntity(bpPlanet, Vector3(0,0,0));

    //loadup that gpu test
    //gpuTest = new Entity();
    //gpuTest->name = std::string("gpuTestinnng");
    //gpuTest->AttachComponent(CMP_LOGIC, new GPUTest);
    
}

void GameUpdate()
{
    //UpdateUI();
    mainCamera->moveForce = 0.016120f;
    mainCamera->linearDrag = 0.85f;
    mainCamera->FlyLookControl();

    //attach camera to ship
    //mainCamera->GetTransform()->position = player->GetTransform()->position;
    //mainCamera->Update();



   /* if (Input::GetInstance()->GetKeyDown(KB_SPACE) && GetTimecode() > lastSpawn + spawnDelay)
    {
        BlueprintResource *bp = testBp;
        
        Entity *newObj = Builder::GetInstance()->ConstructEntity(bp, Vector3(43,43,-1.0)); //-1 - -2 is top layer game objects
                
        ObjectManager::GetInstance()->Add(newObj);
        lastSpawn = GetTimecode();

    }*/
    
    //mainCamera->HardLookAt(player->GetTransform()->position);
   // Vector3 camOffset = player->GetTransform()->position + (*player->GetTransform()->back) * 6;
   // camOffset.y += 2;
   // mainCamera->SoftAttach(camOffset, 21);
    
   // UpdateUI();
}


void GameShutdown()
{
    ShutdownUI();
    //mainCamera->DestroyAndRelease();
}
