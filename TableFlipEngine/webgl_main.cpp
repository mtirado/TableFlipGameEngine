/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include <stdio.h>
#include <EGL/egl.h>
//#include <GLES2/gl2.h>
#include <SDL/SDL.h>
//#include <SDL/SDL_opengl.h>
#include <emscripten.h>


#include "Input.h"
#include "Renderer.h"
#include "Utilities.h"
#include "WebGLGame.h"
#include "UserInterface.h"
#include "SoundSystem.h"
#include "ObjectManager.h"
#include "Physics.h"
#include "Builder.h"
bool fullscreen = false;
SDL_Surface *screen;
//singleton city
SoundSystem     *soundSystem = 0;
Renderer        *renderer = 0;
Input           *input = 0;
//Scripting       *scripting = 0;
Physics         *physics = 0;
ObjectManager   *objectManager = 0;
UIManager       *uiManager = 0;
Builder         *builder = 0;

int windowWidth = 800;
int windowHeight = 600;

int currentWidth = windowWidth;
int currentHeight = windowHeight;

bool Init(int w, int h);
void Update();
void Shutdown();

bool running = false;
unsigned int lastTime = 0;

#define MAX_KEYDOWN 8
//more input BS because sdl events are super wack
//will store keydown state
struct KeyHax
{
    bool inUse; //free for use?
    int keycode;
    bool down;
    KeyHax() { inUse = false;}
};

KeyHax keyHax[MAX_KEYDOWN];
int keysDown = 0;
//putting this at the bottom so i dont have to look at another ugly switch
void KeyUp(int keycode);
void KeyDown(int keycode); 
//this is gettin ugly quick
void SetKeyHackDown(int keycode)
{
    if (keysDown == MAX_KEYDOWN)
        return;
    int idx = MAX_KEYDOWN;  //set to invalid
    //see if key is already down
    for (int i = 0; i < MAX_KEYDOWN; i++)
    {
        if(keyHax[i].keycode == keycode && keyHax[i].inUse == true)
        {
           // KeyDown(keycode);
            return; //found key, AND its down.
        }
        else if (keyHax[i].inUse == false)
            idx = i; //store last unused slot
    }
    if (idx < MAX_KEYDOWN)
    {
        KeyDown(keycode);
        keyHax[idx].keycode = keycode;
       // keyHax[idx].down = true;  //dont even need this ish
        keyHax[idx].inUse = true; //this shit is DOWN BRO
        keysDown++;
    }        
    
}
void SetKeyHackUp(int keycode)
{
    if (keysDown <= 0)
    {
        LogError("input error, no key down");
        keysDown = 0;
        return;
    }
    for (int i = 0; i < MAX_KEYDOWN; i++)
    {
        if (keyHax[i].keycode == keycode && keyHax[i].inUse == true)
        {
            KeyUp(keycode);
            keyHax[i].inUse = false; //dont taze me
            keysDown--;
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    InitMasterTimecode();
    lastTime = GetTimecode();
    int width, height;
    width = windowWidth;
    height = windowHeight;
    

    if (Init(width, height))
    {
        running = true;
        printf("Initialized, entering main loop.\n");
    }
    if (running)
    {
        emscripten_set_main_loop(Update, 0, 1);
        Shutdown();
    }

    return 0;
}

int result = 1;
float lastX = 0.0f;
float lastY = 0.0f;
float mouseAccel[2];
float mouseSensitivity = 4.0f * 150.0f;
void SDLEventHandler() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
    //printf("got event %d\n", event.type);
    switch(event.type) 
    {
      case SDL_MOUSEMOTION: 
      {
        SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
        //assert(m->state == 0);
        int x, y;
        //SDL_GetMouseState(&x, &y);  
        //assert(x == m->x && y == m->y);
       
        if (fullscreen) //because SDL is doing something funky with mouse input in fullscreen? in X11 at least.
        {
            mouseAccel[0] = (((float)(m->xrel / (float)currentWidth )  * (mouseSensitivity / 1.5)  ) );
            mouseAccel[1] = (((float)(m->yrel / (float)currentHeight ) * (mouseSensitivity / 1.5)  ) );
        }
        else
        {
            mouseAccel[0] = (((float)(m->xrel / (float)currentWidth )  * mouseSensitivity  ) );
            mouseAccel[1] = (((float)(m->yrel / (float)currentHeight ) * mouseSensitivity ) );
        }
        input->PointerAcceleration(mouseAccel);
        
        lastX = x / (float)currentWidth;
        lastY = y / (float)currentHeight;
        break;
      }
      case SDL_MOUSEBUTTONDOWN: 
      {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        if (!uiManager->CheckMouseClick(Vector2(m->x, m->y), 0))
        {
            //UI loses focuse
            uiManager->DropFocus(0);
            
            input->ButtonPressed(((unsigned int)m->button));
            input->SetTouchCoords(0, Vector2( m->x, m->y));
        }
         
        
        
        break;
      }
      case SDL_MOUSEBUTTONUP: 
      {
        SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
        //printf("button up: %d,%d  %d,%d\n", m->button, m->state, m->x, m->y);
        input->ButtonReleased(((unsigned int)m->button));
        input->SetTouchCoords(0, Vector2( m->x, m->y));
        
        // Remove another click we want to ignore
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN) == 1);
        assert(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONUP) == 1);
        break;
      }
      case SDL_KEYDOWN:
      {
            SetKeyHackDown(event.key.keysym.sym);//switch (event.key.keysym.sym) 
      break;
      }
      case SDL_KEYUP:
      {
            SetKeyHackUp(event.key.keysym.sym);      
      break;
      }
      case SDL_VIDEORESIZE:
      {
        SDL_ResizeEvent *r = (SDL_ResizeEvent*)&event;
        currentWidth = r->w;
        currentHeight = r->h;
        SDL_SetVideoMode( currentWidth, currentHeight, 16, SDL_OPENGL | SDL_RESIZABLE );
        renderer->ResizeWindow(currentWidth, currentHeight);
        
        fullscreen = !fullscreen;
      break;
      }
      default: /* Report an unhandled event */
        printf("I don't know what this event is!\n");
        break;
    }
  }
}

bool Init(int w, int h)
{
    
    
   if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

   // SDL_putenv("SDL_MOUSE_RELATIVE=0");
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*

    screen = SDL_SetVideoMode( w, h, 16, SDL_OPENGL | SDL_RESIZABLE  ); // *changed*
    if ( !screen ) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    CheckGLErrors();
    LogInfo("Initializing Engine Subsystems");
    //FIRE UP THE SINGLETONRAY

    SoundSystem::InitInstance();
    soundSystem = SoundSystem::GetInstance();
    if (!soundSystem->Initialize())
        LogError("Could not Initialize SoundSystem");
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
    
     //TODO: consolidate this maybe??
    CheckGLErrors();
    Renderer::InitInstance();
    renderer = Renderer::GetInstance();
    if (!renderer->Init())
    {
        LogError("Couldnt init renderer, catastrophe!");
        return false;
    }
    CheckGLErrors();
    LogInfo("Rendering System Initialized.");

    Physics::InitInstance();
    physics = Physics::GetInstance();
    physics->Init();
    CheckGLErrors();
    LogInfo("Physics Initialized.");

    ObjectManager::InitInstance();
    objectManager = ObjectManager::GetInstance();
    objectManager->Init();
    CheckGLErrors();
    LogInfo("ObjectManager Initialized.");

    UIManager::InitInstance();
    uiManager = UIManager::GetInstance();
    uiManager->Init();
    CheckGLErrors();
    LogInfo("UI Initialized.");
    //NOTE: AFTER ALL OTHER INIT!!!

    Builder::InitInstance();
    builder = Builder::GetInstance();
    LogInfo("Builder ... built!");

    CheckGLErrors();
    renderer->ResizeWindow(w, h);
    

    GameInit();
    CheckGLErrors();
    LogInfo("Game Initialized");
    return true;

}

//to help pinpoint opengl errors
bool dbgOnce = true;


void Update()
{
    if (dbgOnce) LogInfo("Update TOP");
    //TODO maybe i should try the "sdl" timer if it has a more reliable DT? 
    UpdateMasterTimecode();

    
    input->Update(); // MUST BE UPDATED BEFORE WE SET ANYTHING!!!
    SDLEventHandler();  //set new input state
    
    soundSystem->Update();
    if (dbgOnce) LogInfo("ui manager update");
    uiManager->Update();
    float ms = GetTimecode() - lastTime;
  //  if (ms > 70.0f)
    {
       // LOGOUTPUT << "frame skipped, dt too high("<< ms << "ms). if this persists, try closing other tabs or browser windows";
      //  LogWarning();
        
    }
   // else
    {
        physics->Update(0.001f * (float)(GetTimecode() - lastTime)); 
        //object update goes after, so any transform overrides will stick.
        if (dbgOnce) LogInfo("object manager update");
        objectManager->Update(ms);
        if (dbgOnce) LogInfo("game update");
        GameUpdate();
    }
    lastTime = GetTimecode();

    if (dbgOnce) LogInfo("pre render");
    renderer->PreRender();
    if (dbgOnce) LogInfo("render 3d");
    renderer->Render3D();
    if (dbgOnce) LogInfo("render 2d");
    renderer->Render2D();
    if (dbgOnce) LogInfo("post render");
    renderer->PostRender();

    if (dbgOnce) LogInfo("swap buffer");
    SDL_GL_SwapBuffers();
    //eglSwapBuffers(display, surface);

    if (dbgOnce) LogInfo("Update BOTTOM");

    dbgOnce = false;
}



void Shutdown()
{
    GameShutdown();

    objectManager->Shutdown();

    input->Shutdown();
    soundSystem->Shutdown();
    //scripting->Shutdown();
    physics->Shutdown();
    uiManager->Shutdown();
    renderer->Shutdown();
 
    
    
    input->DeleteInstance();
    //scripting->DeleteInstance();
    soundSystem->DeleteInstance();
    physics->DeleteInstance();
    objectManager->DeleteInstance();
    uiManager->DeleteInstance();
    renderer->DeleteInstance();
    builder->DeleteInstance();
    
    //make sure nothing deletes level 0 textures before this!
    CleanResources(0); //clean all resources!

    SDL_Quit();
}

void KeyUp(int keycode)
{
    switch (keycode) 
    {
        case SDLK_RIGHT: 
            input->KeyReleased(KB_RIGHT);
        break;
        case SDLK_LEFT: 
            input->KeyReleased(KB_LEFT);
        break;
        case SDLK_DOWN: 
            input->KeyReleased(KB_DOWN);
        break;
        case SDLK_UP: 
            input->KeyReleased(KB_UP);
        break;
        case SDLK_SPACE: 
            input->KeyReleased(KB_SPACE);
        break;
        case SDLK_a: 
            input->KeyReleased(KB_A);
        break;
        case SDLK_d: 
            input->KeyReleased(KB_D);
        break;
        case SDLK_w: 
            input->KeyReleased(KB_W);
        break;
        case SDLK_s: 
            input->KeyReleased(KB_S);
        break;
        case SDLK_q: 
            input->KeyReleased(KB_Q);
        break;
        default: 
        break;
    }
}
void KeyDown(int keycode)
{
    switch (keycode) 
    {
        case SDLK_RIGHT: 
            input->KeyPressed(KB_RIGHT);
        break;
        case SDLK_LEFT: 
            input->KeyPressed(KB_LEFT);
        break;
        case SDLK_DOWN: 
            input->KeyPressed(KB_DOWN);
        break;
        case SDLK_UP: 
            input->KeyPressed(KB_UP);
        break;
        case SDLK_SPACE: 
            input->KeyPressed(KB_SPACE);
        break;
        
        case SDLK_a: 
            input->KeyPressed(KB_A);
        break;
        case SDLK_d: 
            input->KeyPressed(KB_D);
        break;
        case SDLK_w: 
            input->KeyPressed(KB_W);
        break;
        case SDLK_s: 
            input->KeyPressed(KB_S);
        break;
        case SDLK_q: 
            input->KeyPressed(KB_Q);
        break;
            default: 
        break;
    }
}
