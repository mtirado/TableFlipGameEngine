#include "Game.h"
#include "Utilities.h"
#include "Renderer.h"
#include "UserInterface.h"
#include "UIButton.h"
/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "ObjectManager.h"
#include "Builder.h"
#include "SoundSystem.h"
#include "BoundingVolumes.h"
#include "Octree-Orb.hpp"
#include <unistd.h>

Camera *mainCamera;
BVHOctreeNode *octree;

//level load callback
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
    
    
    octree = OctreeBuild(0, Vector3(0, 0, 0), 204.3333, 5);
    Renderer::GetInstance()->debugOctree = octree;

    for (int i = 0; i < 1995; i++)
    {
        Entity *test = new Entity;
        OctreeOrb  *logic = new OctreeOrb;
        logic->root = octree;
        test->AttachComponent(CMP_LOGIC, logic); 
        int range = 100;
        Vector3 pos = Vector3(-range+rand()%(range + range), -range+rand()%(range + range), -range+rand()%(range + range)); 
        test->GetTransform()->position = pos;
        test->GetTransform()->UpdatePosition();
        OctreeInsert(octree, test);
    }       
}

void GameUpdate()
{
    //UpdateUI();
    mainCamera->moveForce = 0.016120f;
    mainCamera->linearDrag = 0.85f;
    mainCamera->FlyLookControl();
}


void GameShutdown()
{
    ShutdownUI();
    //mainCamera->DestroyAndRelease();
}
