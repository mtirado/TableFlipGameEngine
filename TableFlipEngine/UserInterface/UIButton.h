/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#include "UserInterface.h"
#include "UITextOut.h"

class UIButton : public UIElement
{
private:
    bool wasDown;
    bool down;
    bool hovering;
    Vector2 textOffset;
    Vector2 *frameVerts;
public:

    UIButton();
    ~UIButton();
    
    TString<wchar_t> normalText;
    TString<wchar_t> hoverText;
    UITextOut text;

    //override class if you want focusfunctionality
    virtual void FocusFunc() {}

    virtual void MouseDown(Vector2 pt);
    virtual void HoverFunc();
    void MouseUp();

    //returns button down state
    inline bool isDown() { 
        return down; 
    }
    void SetText(const TString<wchar_t> &txt) { 
        normalText = txt;
        Invalidate();
    }
    void SetHoverText(const TString<wchar_t> &txt) {
        hoverText  = txt;
        Invalidate(); 
    }
    void SetTextOffset(Vector2 off) 
    { 
        textOffset = off;
        Invalidate();
    }
    void SetFontSize(float pt)
    {
        text.SetFontSize(pt);
    }
    //returns true the frame the button is released
    //*NOTE*this is assuming Pressed() is called every frame,
    //unknown behavior if otherwise, use isDown()
    inline bool Pressed()
    {
        if (wasDown && !down)
        {
            wasDown = false;
            return true;
        }
        return false;
    }

    virtual void Draw();
    virtual void Invalidate();
    virtual void AddToWindow(UIWindow *win);
};
#endif
