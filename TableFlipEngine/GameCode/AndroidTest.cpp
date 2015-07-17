/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "TestGame.h"
#include "Builder.h"
#include "ObjectManager.h"

UIWindow *wndMain;
UITextOut *txtMain;
BlueprintResource *bpAgency;
BlueprintResource *bpBox;
BlueprintResource *bpLander;
BlueprintResource *bpThing;
BlueprintResource *bpTerrain;

//terrain object (imaginary?)
Entity *terrainObj=0;

Camera *mainCamera;
Entity *homeBase = 0;
Entity *lander = 0;
Entity *thing = 0;

ShaderResource *particleShader =0;
void OnLoad()
{
   
    homeBase = ObjectManager::GetInstance()->GetEntity("HomeBase");
    mainCamera->SetOrbit(lander);
    mainCamera->distance = 30;

}

void GameInit()
{
    
    //this does seem a little silly to still be like this...
    mainCamera = new Camera;
    mainCamera->Init();
    mainCamera->GetTransform()->position = Vector3(0.0, 0.0, 0.0);
    Renderer::GetInstance()->camera = mainCamera;

    
    wndMain = new UIWindow();
    wndMain->dimensions = Vector2(80, 15);
    wndMain->MoveWindow(Vector2(10, 0));
    wndMain->color[0] = 0.1;wndMain->color[1] = 0.1;wndMain->color[2] = 0.2;wndMain->color[3] = 1.0;
    //UIManager::GetInstance()->AddWindow(wndMain);

    txtMain = new UITextOut();
    txtMain->offset = Vector2(33,40);
    txtMain->output.str("HEY HEY, HOPE THIS WORKS!");
    wndMain->AddElement(txtMain);

    //bpAgency = LoadBlueprint("Data/Blueprints/agency.bp");
    //Entity *ag = gBuilder->ConstructEntity(bpAgency, Vector3(0,0,-200));

    bpBox = LoadBlueprint("Data/Blueprints/box.bp");
    bpLander = LoadBlueprint("Data/Blueprints/lander.bp");
    bpThing = LoadBlueprint("Data/Blueprints/testThing.bp");

    lander = gBuilder->ConstructEntity(bpLander, Vector3(0,2000,0));
    thing = gBuilder->ConstructEntity(bpThing, Vector3(0,0,0));
    lander->AttachEntity(thing, Vector3(10,20,20));
    

    //ObjectManager::GetInstance()->LoadWorldFile("Data/Worlds/world.xfw", OnLoad);
    particleShader = LoadShaderResource("Data/Shaders/ParticleStream.shader");
    lander->AttachComponent(CMP_PARTICLE_SYSTEM, particleShader);
    lander->GetParticleSystem()->Init(100000);

    bpTerrain = LoadBlueprint("Data/Blueprints/TestPlanet.bp");
    terrainObj = gBuilder->ConstructEntity(bpTerrain, Vector3(0.0, 0.0, 0.0));
    //lander->GetRigidBody()->rigidBody->setFlags(BT_ENABLE_GYROPSCOPIC_FORCE);
    
}

void GameUpdate()
{

    float modPitch = mainCamera->GetTransform()->GetBack().AngleBetween(lander->GetTransform()->GetBack());
    float modRoll = mainCamera->GetTransform()->GetRight().AngleBetween(lander->GetTransform()->GetRight());

    Vector3 atPos = lander->GetTransform()->position - lander->GetTransform()->GetBack() * -25;
    
    mainCamera->HardAttach(atPos, lander->GetTransform()->rotation);
    if (gInput->GetKeyDown(KB_Q))
    {
        mainCamera->GetTransform()->RotateRadians(-1.57079633, 0.0, 0.0);
    }
    if (gInput->GetKeyDown(KB_Z))
    {
        mainCamera->GetTransform()->RotateRadians(1.57079633, 0.0, 0.0);
    }
   // mainCamera->GetTransform()->RotateRadians(
    //    modPitch, 0.0, modRoll
   // );

    
    Vector3 vel = Vector3( lander->GetRigidBody()->rigidBody->getLinearVelocity() );
    float vmag = vel.Magnitude();
    //vel =  vel.Normalized() * lookScale;
    
    
    //Vector3 camtoLand = mainCamera->GetTransform()->position - lander->GetTransform()->position;
    //float rads = camtoLand.AngleBetween(landTarg);

    Vector3 landTarg;
    if (vmag > 19.0)
        landTarg = lander->GetTransform()->position + (vel.Normalized() *
        ( Lerp( 0.0, 80.0, (vmag-19.0)*0.33 / 70.0 ) ) / 6.0 );
    else /*if (vmag < 21.23)*/
        landTarg = lander->GetTransform()->position;
   // else
     //   landTarg = lander->GetTransform()->position + (vel.Normalized() *
       //     Lerp( 0.0, 2.0, 1.0 - vmag ) );

    //mainCamera->GetTransform()->LookAt(landTarg);
    
    //mainCamera->SetOrbit(lander);
    txtMain->output.str("");
    txtMain->output.clear();
    txtMain->output  << GetFPS() << " - fps";
    txtMain->Invalidate();
    
    float zoom = gInput->GetPinchDelta();
    if (zoom != 0.0f)
    {
        mainCamera->distance += zoom;
    }

    if (gInput->GetMouseBtn2JustPressed())
    {
        Entity *e = gBuilder->ConstructEntity(bpBox, mainCamera->GetTransform()->position);
        Vector3 force = mainCamera->GetTransform()->GetBack() * -1000.0;
        e->GetRigidBody()->rigidBody->applyCentralImpulse(force.btVector());
    }


}

void GameShutdown()
{
}
