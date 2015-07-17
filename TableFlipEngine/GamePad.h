/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#ifndef GAMEPAD_H__
#define GAMEPAD_H__
#include "UserInterface.h"

class DPad : public UIElement
{
};

//typical touch screen joystick...
class Joystick : public UIElement
{
private:
    //center is obviously, <0,0>
    float   stickHalfWidth;
    Vector2 stickPos;
    Vector2 stickLimit;
    Vector2 clickedOffset;
    
public:
    void Draw();
    void Invalidate();
    void FocusFunc();
    void MouseDown(Vector2 pt);
    
};

#endif
