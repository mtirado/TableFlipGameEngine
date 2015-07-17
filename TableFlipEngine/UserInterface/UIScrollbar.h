/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UI_SCROLLBAR_H__
#define __UI_SCROLLBAR_H__

#include "UISlider.h"
//basically just a slider, but uses different skin
class UIScrollbar : public UISlider
{
private:
    UIScrollbar();
    unsigned int numScrollVerts;
    bool  btnDown;
    int   btnTick; //magnitude of btn press
    unsigned int lastTick;
    unsigned int tickDelay;
protected:
public:

   UIScrollbar(float _min, float _max);
   ~UIScrollbar();

    virtual void Invalidate();
    virtual void MouseDown(Vector2 pt);
    virtual void MouseUp();
    virtual void FocusFunc();
    virtual void AddToWindow(UIWindow *win);

    virtual void SetPercent(float p);

};



#endif
