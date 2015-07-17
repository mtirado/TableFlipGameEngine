/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UITITLEBAR_H__
#define __UITITLEBAR_H__

#include "UserInterface.h"
#include "UITextOut.h"
//display window name, and move functionality
class UITitleBar : public UIElement
{
    Vector2 clickedOffset;
public:

    UITextOut text;
   // void MouseUp();

    UITitleBar();

    virtual void SetOffset(Vector2 pos);
    virtual void AddToWindow(UIWindow* wnd);
    virtual void Invalidate();
    void MouseDown(Vector2 pt);
    void FocusFunc();
    void Draw();
};

#endif
