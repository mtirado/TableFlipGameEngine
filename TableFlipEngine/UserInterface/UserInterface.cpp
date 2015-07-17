/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

/**
 * TODO TODO TODO TODO
 * TODO  and etc  TODO
 * TODO TODO TODO TODO 
 *
 * REMOVE FROM WINDOW : add a remove method. 
 * stuff like textout needs to remove its text from window
 * when the textout is removed.
 *
 * improve text boundary enforcement (slight overdraw can occur)
 * text boundaries should wrap a whole word, not just the letter its on
 *
 * create a notification so elements can know that they just lost mousehover
 * finish adding message propagation to UIViews window elements
 *
 */


#include "UserInterface.h"
#include "UIWindow.h"
#include "UITitleBar.h"

#include "Renderer.h"
#include "Input.h"
#include "Utilities.h"
#include "SOIL.h"

UIManager *UIManager::instance = 0;

UIManager::UIManager()
{
    texUI = 0;
}
UIManager::~UIManager()
{
    rootWindow = 0;
}

void UIManager::InitInstance()
{
    if (!instance)
        instance = new UIManager;

}

void UIManager::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void UIManager::Init()
{
    texUI = LoadTexture("Data/Textures/default-ui.png", /*SOIL_LOAD_AUTO*/SOIL_LOAD_RGBA, 
            SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y, 0);
    //use nearest neighbor (point) filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (!texUI)
        LogError("--!!--  could not load ui texture  --!!--");

    drawColor = Vector4(1.0f, 1.0f,  1.0f,  1.0f);
    if (!matUIElement.LoadShader("Data/Shaders/UI.shader"))
    {
        LogError("error loading UI shader");
    }

    matUIElement.RegisterProjectionMatrix("ProjectionMatrix");
    //matUIElement.RegisterUniform("color", SV_FLOAT4, &drawColor);
    //matUIElement.RegisterUniform("aspectScale", SV_FLOAT2, &drawColor);
    //matUIElement.RegisterUniform("frameColor", SV_FLOAT4, &drawColor);
    matUIElement.RegisterUniform("sampler", SV_SAMPLER2D, texUI);


    hasHover = 0;
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        hasFocus[i] = 0;

    //textInvalidated = true;

    //create default root window, for static ui elements
    rootWindow = 0;
    rootWindow = new UIWindow;
    windows.push_back(rootWindow);

    //renderer is flagged ready
    gRenderer->FinishInit();

    SetRootWindowDimensions(gRenderer->width, gRenderer->height);
    rootWindow->MoveWindow(Vector2(0,0));
}


UIWindow *UIManager::CreateWindow (wchar_t *title, Vector2 _position, Vector2 _dimensions)
{
    UIWindow * newWin = new UIWindow;
    windows.push_back(newWin);
    newWin->dimensions = _dimensions;
    newWin->MoveWindow(_position);
    //create a title bar
    if (title)
    {
        UITitleBar *titleBar = new UITitleBar;
        titleBar->text.output = TString<wchar_t>(title);
        titleBar->text.SetFont("Data/Fonts/Consolas.ttf");
        titleBar->text.SetFontSize(16);
        titleBar->text.SetGamma(2.8);
        //TODO resize window beneath titlebar
        titleBar->offset = Vector2(0,newWin->dimensions.y - 25.0f);
        titleBar->SetDimensions(Vector2(newWin->dimensions.x, 25.0f));
        newWin->AddElement(titleBar);
        titleBar->text.SetOffset( Vector2(titleBar->offset.x + titleBar->getDimensions().x / 4.85,
                    titleBar->offset.y + titleBar->getDimensions().y * 0.25));
    }

    return newWin;
    //TODO maybe you should implement the windoe ID's
    return 0;
}

void UIManager::AddWindow (UIWindow* win)
{
    windows.push_back(win);
}

/**
 *  the way the stenciling works is window gets drawn with next id
 *  the view increments the stencil value to clip its content
 *  and then decrements the value back to its base window state
 *  only tested with one level of views...
 */
void UIManager::DrawWindows()
{

    //TODO  may need this?  but double invalidation FUCKS freetype-gl up real bad..
    //rootWindow->MoveWindow(Vector2(0,0)); //invalidate the "screen" window

    //glDisable(GL_CULL_FACE);
    //first draw "pass" will enumerate the textElement objects in all windows

    //i started the ui as bottom left oriented like an asshole, so use CW winding now
    glFrontFace(GL_CW);

    //glEnable(GL_BLEND); 
    //TODO z ordering
    glDisable(GL_DEPTH_TEST);

    //stencils
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    winStencilId = 0;
    for (int i = 0; i < windows.size(); i++)
    {
        //increment but dont wrap
        if (++winStencilId >= MAX_WINDOWS)
            winStencilId = MAX_WINDOWS; 


        windows[i]->Draw();
        // glUseProgram(0);
    }
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    glFrontFace(GL_CCW);


    //     glEnable(GL_BLEND);
    //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 

    glDisable(GL_BLEND);

}

void UIManager::Shutdown()
{
    for (int i = 0; i < windows.size(); i++)
        delete windows[i];
}

bool UIManager::CheckMouseClick (Vector2 pt, int cursorId)
{
    //     if (cursorId < 0)
    //     {
    //         LogError("check mouse click URSOR ID IS INVALID!!!!!!");
    //         return false;
    //     }

    //XLIB has mouse coords at top left orientation, translate to bottom left
    //and on apple we must translate coords before we get here :[ think different!
    // pt.y = Renderer::GetInstance()->height - pt.y;

    // cout << "screen coords: <" << pt.x << ", " << pt.y << ">\n";

    //float w = Renderer::GetInstance()->width;
    // float h = Renderer::GetInstance()->height;

    //translate mouse coords to 0 - 1000 space
    //pt.x = (pt.x * 1000) / w;
    // pt.y = (pt.y * 1000) / h;
    bool inWindow = false;
    // cout << "%% coords : <" << pt.x << ", " << pt.y << ">\n";
    //LogError("Entering Check Click Loop");
    //go through all windows and check if coordinates fall inside of an active window
    for (int i = 0; i < windows.size(); i++)
    {
        UIWindow *window = windows[i];
        if (ptInRect(pt, window->GetPosition(), window->dimensions))
        {
            if (window != rootWindow)
                inWindow = true;

            //if in window, and not in element, drop all focus
            UIElement *elem = window->MouseDown(pt);
            //see if this element is already focused on, drop that focus before we add it
            //i  guess this could be pretty wasteful if we allow many cursors...
            //i forget why i do this, maybe it was a multi touch thing
            if (elem)
            {
                for (int z = 0; z < NUM_MULTITOUCH_CURSORS; z++)
                {
                    if (hasFocus[z] == elem)
                    {
                        DropFocus(z);
                    }
                }

                // char buff[64];
                // sprintf(buff, "Grabbing Focus id: %d", cursorId);
                // LogError(buff);
                if(hasFocus[cursorId])
                    hasFocus[cursorId]->DropFocus();

                hasFocus[cursorId] = elem;
                elem->cursorId = cursorId;

                //TODO bug: double mouse down occurs on outter window scrollbars
                //not a big deal, just noting the bug and acknowledging it should probably
                //be fixed. if mousedown call is moved to window, 
                elem->MouseDown(pt);

                //
                //
                //root window should only return true here if we hit an element.
                //if (window != rootWindow)
                return true;
            }
            //if (inWindow)
            //   return true;
        }
    }
    //drop all focus
    for (int z = 0; z < NUM_MULTITOUCH_CURSORS; z++) { DropFocus(z);}
    return inWindow;
}


void UIManager::Update()
{
    Vector2 pt;
    pt.x = Input::GetInstance()->GetXCoords(0);
    pt.y = Input::GetInstance()->GetYCoords(0);

    UIElement *hover = 0;
    bool noHover = true;
    for (int i = 0; i < windows.size(); i++)
    {
        UIWindow *window = windows[i];
        window->Update();
        if (ptInRect(pt, window->GetPosition(), window->dimensions))
        {
            hover = window->MouseHover(pt);
            //if its a new hover elem
            if (hover)
            {
                //is it a new hover?
                if (hasHover != hover)
                {
                    //call mouseout for old elem
                    if (hasHover)
                        hasHover->MouseOut();
                    //set new elem and call mouse over
                    hasHover = hover;
                    if (hover)
                        hover->MouseOver();
                }
                noHover = false;
                break;
            }      
        }    
    }

    //nothing found, mouse out!
    if (noHover)
    {
        if (hasHover)
            hasHover->MouseOut();
        hasHover = 0;
    }

    //call hover func
    if (hasHover)
        hasHover->HoverFunc();
    


    //run focus funcs after window updates (so new text can be buffered)
    //if escape is hit, drop all focus
    if (Input::GetInstance()->GetKeyDownThisFrame(KB_ESC))
    {
        for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        {
            DropFocus(i);
        }
    }
    else
    {
        for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
        {
            if (hasFocus[i])
            {
                hasFocus[i]->FocusFunc();
            }
        }
    }

}

void UIManager::DropFocus(int id)
{
    if (hasFocus[id])
    {
        hasFocus[id]->DropFocus();
        hasFocus[id]->cursorId = -1;
        hasFocus[id] = 0;
    }
}

void UIManager::DropFocus(UIElement* elem)
{
    for (int z = 0; z < NUM_MULTITOUCH_CURSORS; z++)
    {
        if (hasFocus[z] == elem)
        {
            elem->DropFocus();
            hasFocus[z]->cursorId = -1;
            hasFocus[z] = 0;
        }
    }
}

//sets to focus slot 0
void UIManager::GrabFocus(UIElement* elem)
{
    if (hasFocus[0])
        hasFocus[0]->DropFocus();

    hasFocus[0] = elem;
    hasFocus[0]->cursorId = 0;
}

void UIManager::MouseReleased(int touchId)
{
    if (hasFocus[touchId])
    {

        hasFocus[touchId]->Invalidate();
        //if the element retains focus, dont drop it
        //drop will be handled in some other fashion (escape key should be the failsafe drop all focus)
        //on mobile, touch input on same element again or elsewhere perhaps should drop the focus?
        //this allows things like text input boxes to hold focus for keypresses, cursor movement, etc
        hasFocus[touchId]->MouseUp();
        if (!hasFocus[touchId]->retainsFocus())
            DropFocus(touchId);
    }
}


UIWindow *UIManager::GetRootWindow() { return rootWindow; }
void UIManager::AddToRootWindow(UIElement *elem) { rootWindow->AddElement(elem); }
void UIManager::SetRootWindowDimensions(float w, float h)
{
    rootWindow->dimensions.x = w;
    rootWindow->dimensions.y = h;

}


void UIManager::InvalidateElements()
{
    for (int i = 0; i < windows.size(); i++)
    {
        windows[i]->InvalidateElements();
    }
}
