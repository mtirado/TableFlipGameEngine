/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#include "Input.h"
#include "Utilities.h"
#include "UserInterface.h"
#include <stdio.h>
#include "Renderer.h"
//#include <iostream>
Input *Input::instance = 0;


Input::Input()
{    
}
Input::~Input()
{}

void Input::InitInstance()
{
    if (!instance)
        instance = new Input;
}

void Input::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

bool Input::Init()
{
    textGrabMode = false;
    for (int i = 0; i < NUMKEYS; i++)
    {
        keyState[i] = false;
        keyLastState[i] = false;
        keyCurrentState[i] = false;
    }
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        touchStates[i] = false;
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        touchCursors[i] = Vector2(0,0);
    lastMouseX = 0;
    lastMouseY = 0;
    lastMouseBtn1State = mouseBtn1Down = false;
    lastMouseBtn2State = mouseBtn2Down = false;

    mouseXDelta = 0;
    mouseYDelta = 0;
    mouseXMotion = 0;
    mouseYMotion = 0;
    //also please remember that the XServer manages OS sensitivity unless we are getting raw device input
    mouseXSensitivity = 0.01f;
    mouseYSensitivity = 0.01f;
    asciiReadPos = 0;
    asciiWritePos = 0;
    memset(asciiBuffer, 0, sizeof(asciiBuffer));
    return true;
}

//must be updated before anything gets set, acts like a state reset.
void Input::Update()
{

    mouseXDelta = mouseXMotion;
    mouseYDelta = mouseYMotion;
    mouseXMotion = 0;
    mouseYMotion = 0;

    memcpy(keyLastState, keyCurrentState, NUMKEYS * sizeof(bool));
    memcpy(keyCurrentState, keyState, NUMKEYS * sizeof(bool));
    //go through, and change differing keystates

    lastMouseBtn1State = GetMouseBtn1Down();
    lastMouseBtn2State = GetMouseBtn2Down();

    asciiReadPos = 0;
    asciiWritePos = 0;
    memset(asciiBuffer, 0, sizeof(asciiBuffer));
    pinchDelta = 0.0f;

#if defined(__ANDROID__)

    UpdateGestures();
    for (int id = 0; id < NUM_MULTITOUCH_CURSORS; id++)
        touchLast[id] = touchCursors[id];

#endif
}

void Input::AddAsciiChar(char c)
{
    if (asciiWritePos >= ASCII_BUFFER_SIZE)
        return;
    if (c < ' ' || c > '~')
        return; //32 - 126 ONLY

    asciiBuffer[asciiWritePos++] = c;
}

char Input::GetNextAsciiChar()
{
    if (asciiReadPos >= ASCII_BUFFER_SIZE)
        return 0;

    char ascii = asciiBuffer[asciiReadPos++];
    return ascii;

}
void Input::PointerMoved(void *data)
{

    //for linux...
    /*  mouseX = static_cast<int *>(data)[0];
        mouseY = static_cast<int *>(data)[1];

    //update the delta position
    mouseXDelta = mouseX - lastMouseX;
    mouseYDelta = mouseY - lastMouseY;


    Renderer::GetInstance()->mouseX = mouseX;
    Renderer::GetInstance()->mouseY = mouseY;

    lastMouseX = mouseX;
    lastMouseY = mouseY;*/
}

void Input::PointerAcceleration (void* data)
{

    mouseXMotion += static_cast<float *>(data)[0];
    mouseYMotion += static_cast<float *>(data)[1];

}

//on pc we will have to mimic multi touch with cursor 0
void Input::ButtonPressed (unsigned int btn)
{
    if (btn == 1)
    {
        TouchDown(0, Vector2(mouseXCoords, mouseYCoords));
        mouseBtn1Down = true;
    }
    else
        mouseBtn2Down = true;
}

void Input::ButtonReleased (unsigned int btn)
{
    if (btn == 1)
    {
        mouseBtn1Down = false;
        TouchUp(0, Vector2(mouseXCoords, mouseYCoords));
    }
    else
        mouseBtn2Down = false;
}

void Input::KeyPressed (InputKey key)
{

    if (key < 0 || key >= NUMKEYS) return;
    if (textGrabMode)
    {
        //if its an ascii key, bail out
        if (key >= KB_A && key <= KB_Z)
            return;
    }
    keyState[key] = true;
}

void Input::KeyReleased (InputKey key)
{
    if (key < 0 || key >= NUMKEYS) return;
    keyState[key] = false;
}

bool Input::GetKeyDown(InputKey k)
{
    if (k < 0 || k >= NUMKEYS)
        return false; // invalid input so dont do anything
    return keyState[k];

}
bool Input::GetKeyDownThisFrame(InputKey k)
{
    if (k < 0 || k >= NUMKEYS)
        return false; // invalid input so dont do anything

    if (keyLastState[k] == keyCurrentState[k])
        return false;
    return (keyCurrentState[k]);
}

bool Input::GetKeyUpThisFrame(InputKey k)
{
    if (k < 0 || k >= NUMKEYS)
        return false; // invalid input so dont do anything

    if (keyLastState[k] == keyCurrentState[k])
        return false;
    return (!keyCurrentState[k]);
}

void Input::TouchDown ( int id, Vector2 pos )
{
    if (id < 0 || id >= NUM_MULTITOUCH_CURSORS)
        LogError("TOUCH DOWN ERROR!");
    else
    {
        //char buff[64];
        //sprintf(buff, "touch down id: %d", id);
        //LogError(buff);
        touchCursors[id] = pos;
        touchLast[id] = pos; //first touch, disregard previously stored  touch
        touchStates[id] = true;

        //         if (!UIManager::GetInstance()->CheckMouseClick(Vector2(pos.x, pos.y), id))
        //         {
        //         	//means we clicked and did not hit an element
        //         }

    }

}

void Input::TouchUp ( int id, Vector2 pos )
{
    if (id < 0 || id >= NUM_MULTITOUCH_CURSORS)
        LogError("TOUCH UP ERROR!");
    else
    {
        //  char buff[64];
        //sprintf(buff, "touch up id: %d", id);
        //LogError(buff);

        //mouseXCoords = pos.x;
        // mouseYCoords = pos.y;
        //touchCursors[id] = touchLast[id];

        //touchLast[id] = pos;
        touchStates[id] = false;

        UIManager::GetInstance()->MouseReleased(id);
        //UIManager::GetInstance()->DropFocus(id);
    }    
}

void Input::TouchMove ( int id, Vector2 pos )
{
    if (!touchStates[id] || id < 0 || id >= NUM_MULTITOUCH_CURSORS)
        LogError("TOUCH MOVE ERROR!");
    else
    {
        //LOGOUTPUT << "mov.x: " << pos.x << " | mov.y: " << pos.y;
        //LogInfo();
        //touchLast[id] = touchCursors[id]; // moved to update
        mouseXCoords = pos.x;
        mouseYCoords = pos.y;
        touchLast[id] = touchCursors[id];
        touchCursors[id] = pos;

        //TODO write some good "PROPER" touch input code!
        float mouseAccel[2];
        //NOTE swipe controls should feel different, inverted from normal mouse delta
        mouseAccel[0] = pos.x - touchLast[id].x;
        mouseAccel[1] = pos.y - touchLast[id].y;

        PointerAcceleration(mouseAccel);
    }    

}

void Input::UpdateGestures()
{
    pinchDelta = 0.0f;
    int numTouches = 0;
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        if (touchStates[i])
            numTouches++;


    //only really care about pinch right now
    //find the two points farthest from eachother
    if (numTouches < 2)
        return;
    int idxA = 0;
    int idxB = 0;
    float maxDist = 0;
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
    {
        if (!touchStates[i])
            continue;
        Vector2 a = touchCursors[i];
        for (int j = 0; j < NUM_MULTITOUCH_CURSORS; j++)
        {
            if (!touchStates[j])
                continue;
            //NOTE maybe could optimize by just turning off signed bit, without if checks?
            Vector2 b = touchCursors[j];
            Vector2 dist;
            if (a.x > b.x)
                dist.x = a.x - b.x;
            else
                dist.x = b.x - a.x;
            if (a.y > b.y)
                dist.y = a.y - b.y;
            else
                dist.y = b.y - a.y;

            if (dist.x + dist.y > maxDist)
            {
                idxA = i;
                idxB = j;
                maxDist = dist.x + dist.y;
            }
        }
    }

    Vector2 lastDist = touchLast[idxA] - touchLast[idxB];
    Vector2 curDist  = touchCursors[idxA] - touchCursors[idxB];


    pinchDelta = (curDist.MagnitudeSq() - lastDist.MagnitudeSq()) * mouseXSensitivity;

    //float threshold = 0.001;
    /*if (dMag < 0.0)
      {
    //pinching
    LogInfo("pinching");
    }
    else if (dMag > 0.0) 
    {
    //expanding
    LogInfo("expanding");
    }*/


    //get deltas, in reference to center of screen outward 
    //Vector2 halfScreen = Vector2(Renderer::GetInstance()->width/2, Renderer::GetInstance()->height/2);
    //Vector2 toA = halfScreen - (touchCursors[idxA] - touchLast[idxA]);
    //Vector2 toB = halfScreen - (touchCursors[idxB] - touchLast[idxB]);


}

/*Vector2 Input::GetNormalizedCoords(int id)
{
    Vector2 pt;
    pt.x = GetXCoords(id);
    pt.y = GetYCoords(id);
    //translate to bottom left
    //pt.y = gRenderer->height - pt.y;
    pt.x = (pt.x * 1000.0) / gRenderer->width;
    pt.y = (pt.y * 1000.0) / gRenderer->height;
    return pt;
}*/


void Input::Shutdown()
{

}
