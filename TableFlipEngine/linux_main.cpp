/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


//#include "object.h"

#include "Utilities.h"
#include "Scripting.h"
#include <SOIL.h>
#include "SoundSystem.h"
#include "Renderer.h"
#include "Input.h"

#include "Physics.h"

#include "TestGame.h"
//#include "WebGLGame.h"

#include "ObjectManager.h"
#include "UserInterface.h"
#include "Resources.h"
#include "Builder.h"

#include <stdlib.h>
#include <iostream>
using namespace std;

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <unistd.h>
#include <cstring>
#include <GLEW/glew.h>
//#include <GL/gl.h>
#include <GL/glx.h>
//#include <GL/glu.h>
#define WIDTH  700 //1366/2
#define HEIGHT 700 // 768/2

#define TITLE "this is a window."

SoundSystem     *soundSystem = 0;
Renderer        *renderer = 0;
Input           *input = 0;
Scripting       *scripting = 0;
Physics         *physics = 0;
ObjectManager   *objectManager = 0;
UIManager       *uiManager = 0;
Builder         *builder = 0;

GLfloat         rotQuad = 0.0f;
/* attributes for a single buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */                       
static int attrListSgl[] =                                             
{                                                                      
    GLX_RGBA, GLX_RED_SIZE, 4,                                         
    GLX_GREEN_SIZE, 4,                                                 
    GLX_BLUE_SIZE, 4,                                                  
    GLX_DEPTH_SIZE, 16,
    GLX_STENCIL_SIZE, 8,
    None                                                               
};                                                                     
 
/* attributes for a double buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */                       
static int attrListDbl[] =                                             
{                                                                      
    GLX_RGBA, GLX_DOUBLEBUFFER,                                        
    GLX_RED_SIZE, 4,                                                   
    GLX_GREEN_SIZE, 4,                                                 
    GLX_BLUE_SIZE, 4,                                                  
    GLX_DEPTH_SIZE, 16,
    GLX_STENCIL_SIZE, 8,
    None                                                               
}; 

typedef struct {
    Display *dpy;
    int screen;  
    Window win;  
    GLXContext ctx;
    XSetWindowAttributes attr;
    Bool fs;                  
    Bool doubleBuffered;      
    //XF86VidModeModeInfo deskMode;
    int x, y;                    
    unsigned int width, height;  
    unsigned int depth;                                                                                                                                        
} GL_Window;                                                                                                                                                    
 
GL_Window GLWin; 

Display *display = NULL;
Window window;
int screen;
GLXContext context;
XSetWindowAttributes winAttrs;
bool fullscreen = false;
bool doubleBuffered;
int x, y;
unsigned int width, height, depth;


void initGL();
bool createWindow();
void renderGL();
void destroyWindow();
void resizeGL(unsigned int width, unsigned int height);

bool createWindow()
{
        XVisualInfo *vi = NULL;
        Colormap cmap;
        Atom wmDelete;

      
        //open connection to xserver
        display = XOpenDisplay(NULL);
        if (!display)
        {       
                cout << "Cannot connect to X Server (XOpenDisplay(0))\n";
                return -1;
        }

        //grab default screen
        screen = DefaultScreen(display);
       
        //rendering attributes "visual info"                                       
        vi = glXChooseVisual(display, screen, attrListDbl);                     
        if (vi == NULL)                                                         
        {                                                                       
                vi = glXChooseVisual(display, screen, attrListSgl);                 
                doubleBuffered = False;                                             
                cout << "singlebuffered rendering will be used, no doublebuffering available\n";
        }                                                                                   
        else                                                                                
        {                                                                                   
                doubleBuffered = True;                                                          
                cout << "doublebuffered rendering available\n";                                 
        }  
        context = glXCreateContext(display, vi, 0, GL_TRUE); 
        cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
        
        winAttrs.colormap = cmap;                                                            
        winAttrs.border_pixel = 0; 

        /* create a window in window mode*/                                  
        winAttrs.event_mask = PointerMotionMask| ExposureMask | KeyPressMask | KeyReleaseMask | 
                                ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;                                             
        window = XCreateWindow(display, RootWindow(display, vi->screen),     
            0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,      
            CWBorderPixel | CWColormap | CWEventMask, &winAttrs);     
        
        
        
        //set XI input masks for raw motion
        XIEventMask mask;
        mask.mask_len = XIMaskLen(XI_RawMotion);
       /* mask.deviceid = XIAllMasterDevices;
        mask.mask_len = XIMaskLen(XI_RawMotion);
        mask.mask = calloc(mask.mask_len, sizeof(char));
        XISetMask(mask.mask, XI_ButtonPress);
        XISetMask(mask.mask, XI_ButtonRelease);  //TODO is this even needed??
        
        XISelectEvents(display, window, &mask, 1);*/
        
        mask.deviceid = XIAllMasterDevices;
        mask.mask = (unsigned char *)calloc(mask.mask_len, sizeof(char));
        memset(mask.mask, 0, mask.mask_len);
        XISetMask(mask.mask, XI_RawMotion);
        XISelectEvents(display, DefaultRootWindow(display), &mask, 1);
        free(mask.mask);
        
        
        
        
        /* only set window title and handle wm_delete_events if in windowed mode */
        wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);                 
        XSetWMProtocols(display, window, &wmDelete, 1);                            
        XSetStandardProperties(display, window, TITLE,                             
            TITLE, None, NULL, 0, NULL);                                           
        XMapRaised(display, window);     

         /* connect the glx-context to the window */                                    
        glXMakeCurrent(display, window, context);                                      
        if (glXIsDirect(display, context))                                             
                cout << "DRI enabled\n";                                                   
        else                                                                           
                cout << "no DRI available\n";                                              
    //initGL();                        
/*      window = XCreateWindow(display, DefaultRootWindow(display), 0, 0,
                                        200, 100, 0,
                                        CopyFromParent, CopyFromParent, CopyFromParent,
                                        NULL, 0);
        XMapWindow(display, window);
        XFlush(display);
        sleep(10);
*/

        
        return true;
}



void destroyWindow() 
{                    
    if( context )    
    {                
        if( !glXMakeCurrent(display, None, NULL))
        {                                        
            cout << "Could not release drawing context.\n";
        }                                                  
        /* destroy the context */  
        glXDestroyContext(display, context);     
        
        context = NULL;                                    
    }                                                      
    /* switch back to original desktop resolution if we were in fullscreen */
  /*  if( fullscreen )                                                         
    {                                                                        
        XF86VidModeSwitchToMode(display, screen, &desktopMode);              
        XF86VidModeSetViewPort(display, screen, 0, 0);                       
    }          */                                                              
    XCloseDisplay(display);                                                  
}                                                                            

int main(int argc, char ** argv)
{
    InitMasterTimecode();
    XEvent event;   
    XGenericEvent gevt;
    Bool done = False;          
        
    width = WIDTH;
    height = HEIGHT;
 
    //always before we init GL, obviously
    createWindow();
    int xi_opcode, infoevent, error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &infoevent, &error)) {
           cout << "X Input extension not available.\n";
              return -1;
    }

    SoundSystem::InitInstance();
    soundSystem = SoundSystem::GetInstance();
    soundSystem->Initialize();
    
    //init input
    Input::InitInstance();
    input = Input::GetInstance();
    input->Init();
   // input->Init(display, window);
    //XAllowEvents(display, SyncBoth, CurrentTime);
    
//     Scripting::InitInstance();
//     scripting = Scripting::GetInstance();
//    
//     if (!scripting->Init())
//     {
//         cout << "Script Init Failed!!!!\n\n";
//         return -123;
//     }
    

    Physics::InitInstance();
    physics = Physics::GetInstance();
    physics->Init();
    
     //TODO: consolidate this maybe??
    Renderer::InitInstance();
    renderer = Renderer::GetInstance();
    if (!renderer->Init())
    {
        LogError("Couldnt init renderer, catastrophe!");
        return -666;
    }
    
   
    

    
    ObjectManager::InitInstance();
    objectManager = ObjectManager::GetInstance();
    objectManager->Init();
    
    
    
    Builder::InitInstance();
    builder = Builder::GetInstance();
    
    renderer->ResizeWindow(width, height);
  
    UIManager::InitInstance();
    uiManager = UIManager::GetInstance();
    uiManager->Init();
    //init??
   // uiManager->CreateWindow("some name", Vector2(10,10), Vector2(30,30));
    bool WaitingForMouseUp = false;
    
    /*XGrabPointer(display, 
                 window,
                 1, 
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask , 
                 GrabModeAsync,
                 GrabModeAsync, 
                 None,
                 None,
                 CurrentTime);*/
 
    int mouseCoords[2];
    float mouseAccel[2];
    double acceleratedX = 0.0f;
    double acceleratedY = 0.0f;
    KeySym k;
    
    unsigned int lastTime = GetTimecode();
    XGenericEventCookie *cookie;
    
    double *valuator;
    double *raw_valuator;
    XIRawEvent *rawEvent;
    
    
    //init game code
    GameInit();

        

    /* wait for events and eat up cpu. ;-) */
    while (!done)                            
    {
        UpdateMasterTimecode();
        
        //acts sort of like a state reset.
        input->Update(); // MUST BE UPDATED BEFORE WE SET ANYTHING!!!
        
        cookie = &event.xcookie;
        /* handle the events in the queue */ 
        while (XPending(display) > 0)        
        {                                    
            XNextEvent(display, &event); 
            
            //handle raw input if we got any, XI2 stuffs
            if (cookie->type == GenericEvent && cookie->extension == xi_opcode &&
                XGetEventData(display, cookie))
            {
                //we got an XI2 event.
                switch(cookie->evtype)
                {
                    case XI_RawMotion:
                        
                        rawEvent = (XIRawEvent *)cookie->data;
                        raw_valuator = rawEvent->raw_values;
                        valuator = rawEvent->valuators.values;
                        float xAxis, yAxis;
                       // cout << "    device: " << rawEvent->deviceid << "   " << rawEvent->sourceid;
                        mouseAccel[0] = 0.0f;
                        mouseAccel[1] = 0.0f;
                        
                        acceleratedX = 0.0f;
                        acceleratedY = 0.0f;
                        //valuators are axis i think[0] = x  [1] = y
                        for (int i = 0; i < rawEvent->valuators.mask_len * 8; i++)
                        {
                            if (XIMaskIsSet(rawEvent->valuators.mask, i))
                            {
                               // cout << "  acceleration on valuator " << i <<"  " <<  *valuator - *raw_valuator << "\n";
                               // printf("MOUSE_X : %d    MOUSY_Y : %d", rawEvent->, event->event_y);
                                if (i == 0)
                                {
                                    mouseAccel[0] = (/**valuator -*/ *raw_valuator);
                                    acceleratedX = *valuator - *raw_valuator;
                                }
                                if (i == 1)
                                {
                                    mouseAccel[1] = (/**valuator - */*raw_valuator);
                                    acceleratedY = *valuator - *raw_valuator;
                                }
                                valuator++;
                                raw_valuator++;
                            }
                          
                        }
                       // input->mouseXAccelerated = acceleratedX;
                        //input->mouseYAccelerated = acceleratedY;
                        input->PointerAcceleration(mouseAccel);
                       
                        
                        break;
                    
                    default:
                        break;
                }
            }
            XFreeEventData(display, cookie);
            switch (event.type)              
            {
                case MotionNotify:
                    
                    //using raw motion now  ^ ^
                   // Input::GetInstance()->mouseXCoords = event.xmotion.x;
                   
                   //talk about dirty a HACK
                   //if (fabs(input->GetXAxis()) > 0.001 && fabs(input->GetYAxis()) > 0.001)
                   {
                      // char buff[96];
                      // sprintf(buff, "Set Cursor - X: %f  |  Y: %f", input->GetXAxis(), input->GetYAxis());
                     // LogError(buff);
                        input->SetTouchCoords(0, Vector2( event.xmotion.x, event.xmotion.y));
                   }
                   
                    
                break;
                case Expose:                 
                        if (event.xexpose.count != 0)
                            break;                   
                    //renderGL();                      
                        break;                       
                    case ConfigureNotify:            
                        /* call resizeGL only if our window-size changed */
                        if ((event.xconfigure.width != GLWin.width) ||
                            (event.xconfigure.height != GLWin.height))
                        {
                            width = event.xconfigure.width;
                            height = event.xconfigure.height;
                            renderer->ResizeWindow(width, height);
                        }
                        break;
                /* exit in case of a mouse button press */
                case ButtonPress:
                    
                    if (!uiManager->CheckMouseClick(Vector2(event.xbutton.x, event.xbutton.y), 0))
                    {
                        input->SetTouchCoords(0, Vector2( event.xbutton.x, event.xbutton.y));
                        input->ButtonPressed((unsigned int)event.xbutton.button);
                        
                        //TODO real hacky.. 
                        //input->mouseClickX = event.xbutton.x;
                       // input->mouseClickY = event.xbutton.y;
                        //grab the cursor TODO this doesnt work without raw input, IMPLEMENT IT
                        //unless you want to call XWarpCursor  *puke*!
                        
                        
                    }
                    else
                    {
                       
                        WaitingForMouseUp = true;
                    }
                    
                    XGrabPointer(display, 
                                    window,
                                    1, 
                                    /*PointerMotionMask |*/ ButtonPressMask | ButtonReleaseMask , 
                                    GrabModeAsync,
                                    GrabModeAsync, 
                                    window,
                                    None,
                                    CurrentTime);
                  
                    break;
                    
                case ButtonRelease:
                    input->SetTouchCoords(0, Vector2( event.xbutton.x, event.xbutton.y));
                    input->ButtonReleased(((unsigned int)event.xbutton.button));
                    
                    //if (WaitingForMouseUp)
                    {
                        //call mouse up, on currently focused element
                       // uiManager->MouseReleased(0);
                    }
                    
                    WaitingForMouseUp = false;
                    
                    XUngrabPointer(display, CurrentTime);
                    
                    break;
                case KeyPress:
                    k = XLookupKeysym(&event.xkey,0);
                    //send caps version!
                    if(event.xkey.state & (ShiftMask))
                    {
                        input->AddAsciiChar(ShiftModifier(k));
                    }
                    else if (event.xkey.state & (LockMask))
                    {
                        KeySym upper, lower;
                        XConvertCase(k, &lower, &upper);
                        if(event.xkey.state & (ShiftMask))
                            upper = ShiftModifier(upper);
                        input->AddAsciiChar((char)upper); //send this for text input
                    }
                    else if (k < 255)
                        input->AddAsciiChar((char)k);
                    //if its ascii, reset it to 0 and pass to engine input
                    if (k >= ' ' && k <= '`') //32 - 96  assumes standard ascii latin 1?
                        input->KeyPressed((InputKey)k);
                    else if (k >= 'a' && k <= 'z') //convert case
                        input->KeyPressed((InputKey)(k - 32));
                    else if (k == XK_Escape)
                    {
                        input->KeyPressed(KB_ESC);
                        done = true;
                    }
                    else if (k == XK_Return)
                        input->KeyPressed(KB_RETURN);
                    else if (k == XK_BackSpace)
                        input->KeyPressed(KB_BACKSPACE);
                    else if (k == XK_Delete)
                        input->KeyPressed(KB_DELETE);
                    else if (k == XK_F9)
                        input->KeyPressed(KB_F9);
                    else if (k == XK_F12)
                        input->KeyPressed(KB_F12);
                    else if (k == XK_F11)
                        input->KeyPressed(KB_F11);
                    else if (k == XK_Up)
                        input->KeyPressed(KB_UP);
                    else if (k == XK_Down)
                        input->KeyPressed(KB_DOWN);
                    else if (k == XK_Left)
                        input->KeyPressed(KB_LEFT);
                    else if (k == XK_Right)
                        input->KeyPressed(KB_RIGHT);
                    else if (k == XK_Shift_L)
                        input->KeyPressed(KB_LSHIFT);
                    else if (k == XK_space)
                        input->KeyPressed(KB_SPACE);
                    else if (k == XK_Control_L)
                        input->KeyPressed(KB_LCTRL);
                   
                    break;
                case KeyRelease:
                    k = XLookupKeysym(&event.xkey,0);
                    if (k >= ' ' && k <= '`') //32 - 96  assumes standard ascii latin 1?
                        input->KeyReleased((InputKey)k);
                    else if (k >= 'a' && k <= 'z') //convert case
                        input->KeyReleased((InputKey)(k - 32));
                    else if (k == XK_Escape)
                        input->KeyReleased(KB_ESC);
                    else if (k == XK_Return)
                        input->KeyReleased(KB_RETURN);
                    else if (k == XK_BackSpace)
                        input->KeyReleased(KB_BACKSPACE);
                    else if (k == XK_Delete)
                        input->KeyReleased(KB_DELETE);
                    else if (k == XK_F9)
                        input->KeyReleased(KB_F9);
                    else if (k == XK_F12)
                        input->KeyReleased(KB_F12);
                    else if (k == XK_F11)
                        input->KeyReleased(KB_F11);
                    else if (k == XK_Up)
                        input->KeyReleased(KB_UP);
                    else if (k == XK_Down)
                        input->KeyReleased(KB_DOWN);
                    else if (k == XK_Left)
                        input->KeyReleased(KB_LEFT);
                    else if (k == XK_Right)
                        input->KeyReleased(KB_RIGHT);
                    else if (k == XK_Shift_L)
                        input->KeyReleased(KB_LSHIFT);
                    else if (k == XK_space)
                        input->KeyReleased(KB_SPACE);
                     else if (k == XK_Control_L)
                        input->KeyReleased(KB_LCTRL);
                    break;
                case ClientMessage:
                    if (strcmp(XGetAtomName(display, event.xclient.message_type),
                               "WM_PROTOCOLS") == 0)
                    {
                        done = True;
                    }
                    break;
                default:
                    break;
            }
        }
        
        soundSystem->Update();


        uiManager->Update();
       
        //NOTE: character controller is in physics update, any animation adjustments will be done there
        //and it is correct for its update to be after the animation systems update, just apply extra rotations
        physics->Update(GetDTSeconds());
        //put this after physics so any transform overrides will be valid.
        objectManager->Update(GetDTSeconds());
        GameUpdate();
        //renderGL();
        renderer->PreRender();
        renderer->Render3D();
        renderer->Render2D();
        renderer->PostRender();
        
        if (doubleBuffered)                                
        {                                                  
            glXSwapBuffers(display, window);               
        } 
        
        
        //reset cursor position
          //set cursor to center
        if (Input::GetInstance()->GetMouseBtn1Down())
        {
           // if (Input::GetInstance()->GetXAxis() != 0.0f || Input::GetInstance()->GetYAxis() != 0.0f)
           // cout <<  " mouse X: " << mouseAccel[0] << "mouse Y: " << mouseAccel[1] << "\n";
           XWarpPointer(display, 0, window, 0, 0, 0, 0, width / 2, height / 2);
        }
        
       // cout << "glGetVersion() : " << glGetString(GL_VERSION) << "\n";
        usleep(500);
    }
 
  //testing something... 
    GameShutdown();
 
    
    objectManager->Shutdown();
   
    input->Shutdown();
    soundSystem->Shutdown();
    //scripting->Shutdown();
    physics->Shutdown();
    uiManager->Shutdown();
    renderer->Shutdown();
 
    
    
    input->DeleteInstance();
   // scripting->DeleteInstance();
    soundSystem->DeleteInstance();
    
    objectManager->DeleteInstance();
    uiManager->DeleteInstance();
    renderer->DeleteInstance();
    physics->DeleteInstance();
    builder->DeleteInstance();
    
    //make sure nothing deletes level 0 textures before this!
    CleanResources(0); //clean all resources!
   // soundSystem->DeleteInstance();
    
//     //NOTE this is causing a hang on window close.... FIXME
//     destroyWindow();


    LogInfo("Returning 0");
    return 0;
}

