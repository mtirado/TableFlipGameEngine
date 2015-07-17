/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UISLIDER_H__
#define __UISLIDER_H__

#include "UserInterface.h"
/// TODO this slider does not handle negative values right now
class UISlider : public UIElement
{
    friend class UIWindow;
private:
    UISlider();
protected:
    unsigned int numVerts;
    float lastloc;     //avoid extra scrolls
    float scrollPos;   //position of button 
    
    bool vertical;
    //skin vert data
    Vector2 *scrollVerts; 

    float min;
    float max;
    float value;
    float percent;     //0.0 - 1.0
    float btnLen;
    bool  hasScrolled;  //has scrolled since last check

public:
    //void MouseUp();
    virtual void AddToWindow(UIWindow *win);
    virtual void Draw();
    virtual void Invalidate();
    virtual void FocusFunc();

    //accounts for clamping/endcaps
    virtual void SetPercent(float p);
    void SetValue(float val);

    inline float getPercent() { return percent; }
    inline float getValue()   { return value;   }
    inline float getMin()     { return min;     }
    inline float getMax()     { return max;     }

    ///range min/max and handle dimensions
    UISlider(float _min, float _max);
    ~UISlider();

    //you can only rely on this working once,
    //and for scrollbars, that happens in UIWindow
    inline bool checkScrolled()
    {
        bool rtn = hasScrolled;
        hasScrolled = false;
        return rtn;
    }
};



#endif
