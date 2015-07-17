/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__

#include "Renderer.h"

enum UIElementId 
{ 
    ELEM_DEFAULT = 0, ELEM_UIVIEW, ELEM_UITEXTOUT,
    ELEM_UISCROLLBAR          
};

///draws a debug rectangle representing position+dimensions
///and other misc debug type things
//#define UI_DEBUG_MODE

class UIWindow;

/** IMPORTANT! allocate elements on heap since the windows
 * use them as linked list nodes, and frees them in destructor.
 * never allocate UIElements on the stack!!
 * unless it is never added to a window/view
 * TODO: make factory.
 */
class UIElement
{
friend class UIManager;
friend class UIWindow;
protected:
    UIWindow *parentWindow; //every element is inside a window.
    bool      retainFocus;  //hold focus after mouse up. (ex: text edit box)
    bool      hidden;
    GLuint    vboPos;       //positoins
    GLuint    vboTc;        //texcoords
    

    //for debugging only
#if defined(UI_DEBUG_MODE)
    GLuint    vboDbgRect;
    GLuint    vboDbgTc;

    void DebugDraw();
#endif



    Vector2   position;     //screen position is parent window + offset.
    Vector2   aspectScale;
    Vector2   dimensions;
    short     elementId;   //default is 0

    //for any user specified actions  supply your own action id's
    void (*actionCallback)(UIElement *self, int id);

    //use this for storing entity pointers, or other misc. data
    void *data;
public:
    UIElement *next; //windows use as linked list 

    //for multi touch tracking
    int cursorId;
        
    ///offset from root window coordinates (top left)
    Vector2   offset;
    Vector4   color;          //background color
    bool      scrolls;        //should scrollbars affect this?

    //called when element is added to window, it sets parent pointer
    virtual void AddToWindow(UIWindow *wnd);
    ///called once, when mouse is pressed
    virtual void MouseDown(Vector2 pt){};
    ///TODO -- never actually finished this one
    virtual void MouseUp();
    ///called when element gains hover status
    virtual void MouseOver(){}
    ///called when element loses mouse hover
    virtual void MouseOut(){}
    //function will get called while the element is in focus.
    virtual void FocusFunc(){}
    virtual void DropFocus(){}
    virtual void HoverFunc(){}
    ///called by move window, addelement, etc. updates screen position
    virtual void Invalidate();
    virtual void Draw(){}

    ///calls glGenBuffers for the VBO's
    virtual void CreateGLBuffers();

    //things like text may need to be adjusted manually (margins, and minor corrections)
    //TODO really should use top left orientation for UI, it makes some things easier...
    
    virtual void SetDimensions(Vector2 dim) 
    { dimensions = dim; }
    virtual void SetOffset(Vector2 pos) 
    {offset = pos; }
    inline  void SetColor(float r, float g, float b, float a = 1.0) 
    {color = Vector4(r,g,b,a); }
    inline  void SetColor(Vector4 c) 
    {color = c; }
    
    //don't call this unless you know what its doing...
    //position is usually managed by window except for special cases...
    inline void SetBasePosition(Vector2 pos) { position = pos; }
    inline void SetActionCallback(void (*ac)(UIElement *, int)) 
    { actionCallback = ac; }
    
    inline void setData(void *d) { data = d;    }
    inline void *getData()       { return data; }  
    //sets and gets
    inline bool     retainsFocus()  { return retainFocus;           }
    inline void     Hide()          { hidden = true;                }
    inline void     Show()          { hidden = false; Invalidate(); }
    inline bool     isHidden()      { return hidden;                }
    inline short    getElementId()          { return elementId;     }
    inline const Vector2 &getAspectScale()  { return aspectScale;   }
    inline const Vector2 &getPosition()     { return position;      }
    inline const Vector2 &getDimensions()   { return dimensions;    }

    UIElement();
    ~UIElement();

};



#endif
