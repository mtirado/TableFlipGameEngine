/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UI_VIEW_H__
#define __UI_VIEW_H__

#include "UIWindow.h"



/**this is just a window in disguise as an element,
 * plus a custom draw that doesnt clear stencil buff.
 * this allows it to be nested inside windows/views
 * use it to implement scrolling views, and whatnot 
 * can also sort its contents
 *
 * IMPORTANT! allocate elements on heap since the windows
 * uses them as linked list nodes, and frees them in destructor.
 * never allocate UIElements on the stack!!
 * * */

class UIView : public UIElement
{
private:
protected:
    UIWindow   theWin;
    UIElement *focusElement; //for focus
    UIElement *hoverElement;

public:

    UIView();

    //set before drawing, parents stencil id
    unsigned char winStencilId;
    /**whatever events are received should be
     * sent to the appropriate element if any
     * inside of this window/view */
    virtual void HoverFunc();
    virtual void FocusFunc();
    virtual void DropFocus();
    virtual void MouseUp();
    virtual void MouseOut();
    virtual void MouseOver();
    virtual void Invalidate();
    
    ///draw must call all other draw functions in order
    void Draw(); //try not to mess this up ...
    void DrawWindow();
    void DrawElems();
    ///override this one for drawing before text
    virtual void DrawPreText();
    void DrawText();
    ///this is called by DrawText
    virtual void DrawPostText();
    //find the element for focusing on
    virtual void MouseDown(Vector2 pt);

    void CreateScrollbars(bool horizon, bool vert);
    void CreateFrame();
    void SetBGTexcoords(Vector2 tc);
    void SetDimensions(Vector2 dim);
};

#endif
