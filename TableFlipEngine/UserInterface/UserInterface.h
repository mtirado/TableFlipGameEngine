/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef USERINTERFACE_H__
#define USERINTERFACE_H__
#include "EngineMath.h"
#include "Renderer.h"
#include "Resources.h"
#include "Input.h"
#include <vector>
#include <map>
//#include <map>
#include <string>
#include "TString.h"
#include <sstream>
//using std::map;
using std::vector;
using std::string;
using std::map;

//8bit stencil buffer max
#define MAX_WINDOWS 255

#include "UIElement.h"
class UIWindow;
//class UIRootWindow;

//BOTTOM LEFT ORIENTED
//contains the windows, which contain UI elements
class UIManager
{
private:
     //disable functions for singleton use
    UIManager();
    ~UIManager();
    UIManager(const UIManager &) {}
    UIManager &operator = (const UIManager &) { return *this; }

    UIWindow *rootWindow;
    //the singletons instance.
    static UIManager *instance;
    vector<UIWindow *> windows;

    //we're going to render all the text in a window in 2 draws
    //one for dynamic (constantly changing) text, and one for static text
    //anytime text is added, the windows text buffers must be invalidated


public:

    
    Material matUIElement;    //set shader vars through this
    TextureResource *texUI;
    Vector4 textColor;
    Vector4 drawColor;  //bakground color
    Vector4 frameColor;

    unsigned char winStencilId;
    //maps to multitouch id's
    UIElement *hasFocus[NUM_MULTITOUCH_CURSORS];
    UIElement *hasHover; //mouse hover (no multitouch)

    inline static UIManager *GetInstance() { return instance; }
    static void InitInstance();
    static void DeleteInstance();

    void Init();
    void Update();
    void DrawWindows();
    //pass blank title for no title bar
    void AddWindow(UIWindow *win);
    void Shutdown();

    //goes through all windows and checks if mouse input happened there
    bool CheckMouseClick(Vector2 pt, int cursorId);
    //current focused element should know the mouse has been released
    void MouseReleased(int id);

    //cancels focus
    void DropFocus(int id);
    void DropFocus(UIElement *elem);

    //puts element in focus
    void GrabFocus(UIElement *elem);

    UIWindow *GetRootWindow();
    void AddToRootWindow(UIElement *elem);
    //inline void AddTextElement(UIElement *elem) { textElements.push_back(elem); }
    //inline bool DrawingTextNow() { return drawingText; }
    void SetRootWindowDimensions(float w, float h);
    void InvalidateElements();

    UIWindow *CreateWindow (wchar_t *title, Vector2 _position, Vector2 _dimensions);
//     inline void SetDrawColor(Vector4 c)
//     {
//         drawColor= c;
//         matUIElement.SetUniformData("color", &drawColor);
//         matUIElement.SendUniformData();
//     }
//     inline void SetFrameColor(Vector4 c)
//     {
//         frameColor= c;
//         matUIElement.SetUniformData("frameColor", &frameColor);
//         matUIElement.SendUniformData();
//     }
//     inline void SetTextColor(float r, float g, float b, float a)
//     {
//         textColor= Vector4(r,g,b,a);
//     }

    inline int GetPositionLocation() { 
        return matUIElement.GetAttributeLocation(ATR_POSITION0); }

    inline int GetTexcoordLocation() { 
        return matUIElement.GetAttributeLocation(ATR_TEXCOORD0); }
   // inline Vector2 getRootAspectMultiplier() { return rootAspectMultiplier; }
   
};







//retains focus and listens for keyboard input.
//TODO add a cursor
// class UITextIn : public UITextOut
// {
// private:
//     bool hasFocus; //is currently it being edited?
//     
// public:
//     UITextIn() { hasFocus = false; retainFocus = true; }
//     virtual void FocusFunc();
//     //TODO probably an enterfocus
//     virtual void DropFocus() { hasFocus = false; output.str(""); }
//     inline bool inFocus() { return hasFocus; }
// };
// 
// //wraps text
// class UITextArea : public UITextOut
// {
// public:
//     void Invalidate();
//     //sets the text, and adjusts to fit within dimensions
//     void SetText(string str);
// };












#endif
