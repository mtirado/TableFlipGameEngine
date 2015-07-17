/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


//include the cocoa view header file
#import "osx_main.h"

//#include "object.h"

#include "Utilities.h"
#include <SOIL.h>
#include "SoundSystem.h"
#include "Renderer.h"
#include "Input.h"
//#include "Scripting.h"
#include "Physics.h"
#include "TestGame.h"
#include "ObjectManager.h"
#include "UserInterface.h"
#include "Resources.h"
#include "Builder.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

//#include <X11/Xlib.h>
//#include <X11/extensions/XInput2.h>
#include <unistd.h>
#include <cstring>
//#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glx.h>
//#include <GL/glu.h>

#import <QuartzCore/QuartzCore.h>
#import <Carbon/Carbon.h>

#define TITLE "Codename Tableflip"
static bool G_EngineInitialized = false;

SoundSystem     *soundSystem = 0;
Renderer        *renderer = 0;
Input           *input = 0;
//Scripting       *scripting = 0;
Physics         *physics = 0;
ObjectManager   *objectManager = 0;
UIManager       *uiManager = 0;
Builder         *builder = 0;

unsigned int width, height, depth;

float winX, winY, winWidth, winHeight;
unsigned long lastTime = 0;
float lastMousePos[2];
void ResizeWindow(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
	
	if (renderer)
	{
		renderer->ResizeWindow(width, height);
	}
}                                                                           

bool InitEngine(unsigned int _width, unsigned int _height)
{
    InitMasterTimecode();

	height = _height;
	width = _width;
    lastMousePos[0] = 0.0f;
    lastMousePos[1] = 0.0f;
   // InitGL();	
    
    SoundSystem::InitInstance();
    soundSystem = SoundSystem::GetInstance();
    soundSystem->Initialize();
    
    //init input
    Input::InitInstance();
    input = Input::GetInstance();
    input->Init();
    // input->Init(display, window);
    //XAllowEvents(display, SyncBoth, CurrentTime);
    
    /* Scripting::InitInstance();
        scripting = Scripting::GetInstance();

        if (!scripting->Init())
        {
        cout << "Script Init Failed!!!!\n\n";
        return -123;
        }*/
    
    

    Renderer::InitInstance();
    renderer = Renderer::GetInstance();
    if (!renderer->Init())
    {
        LogError("Couldnt initialize renderer");
        return false;
    }
    
    
    Physics::InitInstance();
    physics = Physics::GetInstance();
    physics->Init();
    
    ObjectManager::InitInstance();
    objectManager = ObjectManager::GetInstance();
    objectManager->Init();
    
    UIManager::InitInstance();
    uiManager = UIManager::GetInstance();
    uiManager->Init();
    
    Builder::InitInstance();
    builder = Builder::GetInstance();
    
    //init??
	// uiManager->CreateWindow("some name", Vector2(10,10), Vector2(30,30));
    
    renderer->ResizeWindow(width, height);
    //init game code
    GameInit();
	G_EngineInitialized = true;
    return 0;
}


void UpdateEngine()
{
    //TODO seems a bit hacky, tryin to get it workin right quick
    if (!G_EngineInitialized)
        return;
	UpdateMasterTimecode();
	
	//acts sort of like a state reset.
	input->Update(); // MUST BE UPDATED BEFORE WE SET ANYTHING!!!
    soundSystem->Update();
	uiManager->Update();
    
    physics->Update(0.001f * (float)(GetTimecode() - lastTime));
	objectManager->Update((float)(GetTimecode() - lastTime));
	
	GameUpdate();
	lastTime = GetTimecode();
	//renderGL();
	renderer->PreRender();
	renderer->Render3D();
	renderer->Render2D();
	renderer->PostRender();
	
		
	//reset cursor position
	//set cursor to center
	if (Input::GetInstance()->GetMouseBtn1Down())
	{
        //this prevents mouse form moving
		//CGAssociateMouseAndMouseCursorPosition(false);
		
        
        //CGPoint mPos;
		//mPos.x = winX + winWidth / 2;
		//mPos.y = winY + winHeight / 2;
		//CGWarpMouseCursorPosition(mPos);
		// if (Input::GetInstance()->GetXAxis() != 0.0f || Input::GetInstance()->GetYAxis() != 0.0f)
		// cout <<  " mouse X: " << mouseAccel[0] << "mouse Y: " << mouseAccel[1] << "\n";
		//XWarpPointer(display, 0, window, 0, 0, 0, 0, width / 2, height / 2);
	}
	
	// cout << "glGetVersion() : " << glGetString(GL_VERSION) << "\n";
	//usleep(4000);
	
}

void ShutdownEngine()
{
	GameShutdown();
	
    objectManager->Shutdown();
    
	
    input->Shutdown();
	 soundSystem->Shutdown();
	// scripting->Shutdown();
    physics->Shutdown();
    uiManager->Shutdown();
    renderer->Shutdown();
    
    input->DeleteInstance();
	// scripting->DeleteInstance();
	 soundSystem->DeleteInstance();
    physics->DeleteInstance();
    objectManager->DeleteInstance();
    uiManager->DeleteInstance();
    renderer->DeleteInstance();
    builder->DeleteInstance();
    
    //make sure nothing deletes level 0 textures before this!
    CleanResources(0); //clean all resources!
	// soundSystem->DeleteInstance();
    
  
}

void MouseDown(int btn, float x, float y)
{
    y = Renderer::GetInstance()->height - y;
	//because xlib and cocoa ans quartz dont all agree on window origin
	//isFlipped please work..∫ HURAY!
	//y = Renderer::GetInstance()->height - y;
	if (!uiManager->CheckMouseClick(Vector2(x, y), 0))
	{
		//UI loses focuse
		uiManager->DropFocus(0);
		input->ButtonPressed(btn);
		input->SetTouchCoords(0, Vector2( x, y));
	}
	else 
	{
		//we clicked outside of a UI element
	}
    lastMousePos[0] = x;
    lastMousePos[1] = y;

}
void MouseUp(int btn, float x, float y)
{
    y = Renderer::GetInstance()->height - y;
	//return mouse control to cursor
	CGAssociateMouseAndMouseCursorPosition(true);
	//y = Renderer::GetInstance()->height - y;

	input->ButtonReleased(btn);
	input->SetTouchCoords(0, Vector2( x, y));
    lastMousePos[0] = x;
    lastMousePos[1] = y;
}

float mouseAccel[2];
void MouseMove(float x, float y)
{
    y = Renderer::GetInstance()->height - y;
	mouseAccel[0] = x - lastMousePos[0];
	mouseAccel[1] = y - lastMousePos[1];
    //LOGOUTPUT << "accel: x: " << mouseAccel[0] << " y: " << mouseAccel[1];
    //LogInfo();
	
	input->PointerAcceleration(mouseAccel);
	//input->SetTouchCoords(0, Vector2( x, y));
    
    lastMousePos[0] = x;
    lastMousePos[1] = y;
}

void KeyPressed(unsigned short key)
{
	switch(key)
    {
        case kVK_Space:
            input->KeyPressed(KB_SPACE);
            break;
        case kVK_LeftArrow:
            input->KeyPressed(KB_LEFT);
            break;
        case kVK_RightArrow:
            input->KeyPressed(KB_RIGHT);
            break;
        case kVK_DownArrow:
            input->KeyPressed(KB_DOWN);
            break;
        case kVK_UpArrow:
            input->KeyPressed(KB_UP);
            break;
        case kVK_F12:
            input->KeyPressed(KB_F12);
            break;
            
            //TODO WHY DO SHIFT AND CONTROL NOT WORK??
        case kVK_Control:
            input->KeyReleased(KB_LCTRL);
            break;
            
            //ansii keys
            
        case kVK_ANSI_W:
            input->KeyPressed(KB_W);
            break;
        case kVK_ANSI_A:
            input->KeyPressed(KB_A);
            break;
        case kVK_ANSI_S:
            input->KeyPressed(KB_S);
            break;
        case kVK_ANSI_D:
            input->KeyPressed(KB_D);
            break;
            
        default:
            break;
    }
    

	
}
void KeyReleased(unsigned short key)
{
	switch(key)
    {
        case kVK_Space:
            input->KeyReleased(KB_SPACE);
            break;
        case kVK_LeftArrow:
            input->KeyReleased(KB_LEFT);
            break;
        case kVK_RightArrow:
            input->KeyReleased(KB_RIGHT);
            break;
        case kVK_DownArrow:
            input->KeyReleased(KB_DOWN);
            break;
        case kVK_UpArrow:
            input->KeyReleased(KB_UP);
            break;
        case kVK_F12:
            input->KeyReleased(KB_F12);
            break;
        case kVK_Control:
            input->KeyReleased(KB_LCTRL);
            break;
            
            //ansii keys
            
        case kVK_ANSI_W:
            input->KeyReleased(KB_W);
            break;
        case kVK_ANSI_A:
            input->KeyReleased(KB_A);
            break;
        case kVK_ANSI_S:
            input->KeyReleased(KB_S);
            break;
        case kVK_ANSI_D:
            input->KeyReleased(KB_D);
            break;

        default:
            break;
    }


}


void SetWindowRect(float x, float y, float w, float h)
{
	winX = x;
	winY = y;
	winWidth = w;
	winHeight = h;
}
