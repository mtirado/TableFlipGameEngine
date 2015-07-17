/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include "Utilities.h"
#include "TestGame.h"

#include "Entity.h"
#include "Camera.h"
#include "Renderer.h"
#include "Input.h"
#include "Resources.h"
#include "Builder.h"
//#include "SoundSystem.h"
#include "Physics.h"
#include <vector>
#include <iostream>
#include <sstream>
#include "bullet/LinearMath/btScalar.h"
#include "ObjectManager.h"
#include "MiscLogic.hpp"
using std::cout;
using std::endl;
using std::vector;

//Model ModelResource;
ModelResource *cubeModel;
ModelResource *weirdModel;
btCollisionShape *cubeShape; 
btCollisionShape *weirdShape; 
//Entity gameObj;
Camera *mainCamera;
Material *matCube;
TextureResource *cubeTex;
TextureResource *blockheadTex;
TextureResource *houseTex;
UITextOut *fpsCounter;
UITextOut *catCounter;

ModelResource *house;

Entity *Player;

AnimatedModelResource *animMesh;
AnimationResource *anim;

//that new ish
BlueprintResource *testBlueprint;
BlueprintResource *playerBlueprint;


void CheckInput();

class JumpingCube : public Entity
{
private:
    unsigned int lastJump;
    ~JumpingCube();
public:
    JumpingCube() { lastJump = 0; }
    void Update()
    {
        if (GetTimecode() > lastJump + 3200)
        {
           // newRb->rigidBody->applyImpulse(fwd * -2800, btVector3(0,0,0));
            Transform3d *t = GetTransform();
            btVector3 up = btVector3(t->GetBack().x, t->GetBack().y, t->GetBack().z);
            up *= -1.0;
            
            GetRigidBody()->rigidBody->setActivationState(1);
            
            GetRigidBody()->rigidBody->applyImpulse(up * 96, btVector3(0,0,0));
            lastJump = GetTimecode();
        }
    }
};



void GameInit()
{
    fpsCounter = new UITextOut("Testing.", false);
    fpsCounter->offset = Vector2(1.0f, 97.0f);
    fpsCounter->dimensions = Vector2(0,0);
    catCounter = new UITextOut("catz", false);
    catCounter->offset = Vector2(80.0f, 97.0f);
    catCounter->dimensions = Vector2(0.0f,0.0f);
    //root window isa default window the size of the screen
    UIManager::GetInstance()->AddToRootWindow(fpsCounter);
    UIManager::GetInstance()->AddToRootWindow(catCounter);
    catCounter->Invalidate();
    fpsCounter->Invalidate();
    //temporary :[s
    mainCamera = new Camera;
    mainCamera->Init();
    mainCamera->GetTransform()->position = Vector3(0.0, 0.0, 0.0);
    Renderer::GetInstance()->camera = mainCamera;
    
    ObjectManager::GetInstance()->LoadWorldFile("Data/Worlds/terrain.xfw");


   //init Debug UI
   DebugWindowInit();
         
}

unsigned int spawnDelay = 75;
unsigned int lastSpawn = 0;
int numCats = 0;
void GameUpdate()
{
    CheckInput();

    DebugWindowUpdate();
}

static float dragX = 0.0;
static float dragY = 0.0;
static float camDist = 4.0f;
static Transform3d camOrbit;
void CheckInput()
{
    float speedMul = 1.0f;
    if (Input::GetInstance()->GetKeyDown(KB_LSHIFT))
        speedMul = 6.0f;
    
    
    if (Input::GetInstance()->GetMouseBtn2Down())
    {
        if (Input::GetInstance()->GetKeyDown(KB_W))
        {
          //  Player->GetCharacterController()->SetForward(speedMul);
        }
        if (Input::GetInstance()->GetKeyDown(KB_S))
        {
           // Player->GetCharacterController()->SetForward(-speedMul);
        }
        if (Input::GetInstance()->GetKeyDown(KB_A))
        {
          //  Player->GetCharacterController()->SetTurn(-1);
        }
        if (Input::GetInstance()->GetKeyDown(KB_D))
        {
          //  Player->GetCharacterController()->SetTurn(1);
        }
        

        
    }
    else
    {
        if (Input::GetInstance()->GetKeyDown(KB_W))
        {
            //Player->GetCharacterController()->SetForward(1.0f);
        }
        if (Input::GetInstance()->GetKeyDown(KB_S))
        {
            //Player->GetCharacterController()->SetForward(-1.0f);
        }
        if (Input::GetInstance()->GetKeyDown(KB_A))
        {
            //Player->GetCharacterController()->SetTurn(-1.0f);
        }
        if (Input::GetInstance()->GetKeyDown(KB_D))
        {
           // Player->GetCharacterController()->SetTurn(1.0f);
        }
    }
    
    
}




void GameShutdown()
{
    DebugWindowShutdown();
 // delete Player;
    mainCamera->DestroyAndRelease();
    //delete matCube;
   // delete fpsCounter;
   // delete cubeShape;
   // for (int i = 0; i < objectList.size(); i++)
    //    delete objectList[i];
}

