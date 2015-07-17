/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIWindow.h"
#include "UIScrollbar.h"

    UIScrollbar::UIScrollbar(float _min, float _max) 
: UISlider(_min, _max)
{
    scrollPos = 0;
    btnTick = 0;
    lastTick = 0;
    tickDelay = 160;

    elementId = ELEM_UISCROLLBAR;
}

UIScrollbar::~UIScrollbar()
{
}


void UIScrollbar::MouseDown(Vector2 pt)
{
    btnTick = 0;
    UIElement::MouseDown(pt);
    LOGOUTPUT << "btnsize: " << btnLen;
    LogInfo();
    if (vertical)
    {
        if (pt.y < position.y + btnLen && pt.y > position.y)
        {
            LogInfo("btndown top");
            btnTick = -1; 
        }
        else if (pt.y > position.y + dimensions.y - btnLen &&
                pt.y < position.y + dimensions.y)
        {
            LogInfo("btndown btm");
            btnTick = 1; 
        }

    }
    else
    {
        if (pt.x < position.x + btnLen && pt.x > position.x)
        {
            LogInfo("btndown left");
            btnTick = -1;
        }
        else if (pt.x > position.x + dimensions.x - btnLen &&
                pt.x < position.x + dimensions.x)
        {
            LogInfo("btndown right");
            btnTick = 1;
        }

    }
}

void UIScrollbar::MouseUp()
{
    UIElement::MouseUp();
    btnTick = 0;
}



void UIScrollbar::AddToWindow(UIWindow *win)
{
    UISlider::AddToWindow(win);
    //set texcoord bufferid
    vboTc = win->skin.getTexcoords(UI_SKIN_SCROLLBAR); 
    btnLen  = win->skin.getInfo(UI_SKIN_SCROLLBAR)->width;
    //clamp dimensions to button size
    if (vertical)
        dimensions.x = btnLen;
    else
        dimensions.y = btnLen;
    scrollPos = btnLen;
    SetPercent(0.0);

}

void UIScrollbar::Invalidate()
{
    if (hidden)
        return;

    UIElement::Invalidate();

    unsigned int num; 
    parentWindow->skin.BuildScrollbar(position, dimensions, scrollPos,
            scrollVerts, &num); 

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * num, 
            scrollVerts, GL_DYNAMIC_DRAW); 

    //set button size
    btnLen = parentWindow->skin.getInfo(UI_SKIN_SCROLLBAR)->width;
}


void UIScrollbar::SetPercent(float p)
{
    //clamp percent
    if (p < 0.0)
        p = 0.0;
    if (p > 1.0)
        p = 1.0;

    bool vertical = dimensions.y > dimensions.x;
    float axisDim = vertical ? dimensions.y : dimensions.x;
    //figure out scrollPos (local space)
    scrollPos = axisDim * p;
    //clamp to buttons
    if (scrollPos < btnLen + btnLen/2)
        scrollPos = btnLen + btnLen/2;
    if (scrollPos + btnLen + btnLen/2 > axisDim)
        scrollPos = axisDim - btnLen - btnLen/2;
    value     = p * max;
    percent   = p;
    Invalidate();

}

void UIScrollbar::FocusFunc()
{
    if (cursorId < 0)
    {
        //LogError("CURSOR ID IS INVALID!!!!!!");
        return;
    }


    float mouseloc;
    float size;
    float framePixels = parentWindow->getFramePixels();
    btnLen = parentWindow->skin.getInfo(UI_SKIN_SCROLLBAR)->height;
    float btnHalfLen = btnLen/2.0;
    if (vertical)
    {
        //mouseloc in local space.
        mouseloc = gInput->GetCoords(cursorId).y - position.y; 
        if (mouseloc == lastloc) //don't waste time on non-scrolls
            return;
        size = dimensions.y;
    }
    else
    {
        mouseloc = gInput->GetCoords(cursorId).x - position.x;
        if (mouseloc == lastloc)
            return;
        size = dimensions.x;
    }

    if (btnTick)
    {
        if (lastTick + tickDelay > GetTimecode())
            return;
        lastTick = GetTimecode();
        mouseloc = scrollPos + btnTick * 2.2;
    }

    //check if on end caps or not, remember y is top oriented in UI      
    if (mouseloc < btnHalfLen + btnLen)
    {
        mouseloc = btnLen + btnHalfLen;
        percent  = 0.0;
    }
    else if (mouseloc > size - btnLen - btnHalfLen)
    {
        mouseloc = size - btnLen - btnHalfLen;
        percent  = 1.0;
    }
    else //adjust the value with the end cap half length, invert percentage
        percent = (mouseloc - btnHalfLen - btnLen) 
            / (size - (btnHalfLen+btnHalfLen+(btnLen * 2)));


    hasScrolled = true;
    scrollPos = mouseloc; 
    value = percent * max;
    lastloc = mouseloc;
    Invalidate();

    //LOGOUTPUT << "percent: " << percent; LogInfo();
}

