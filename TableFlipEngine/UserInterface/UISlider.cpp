/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UISlider.h"
#include "UIWindow.h"


UISlider::UISlider(float _min, float _max)
{
    lastloc = 9834509823; //some crazy number
    min = _min;
    max = _max;
    value = min;
    vertical = false;
    //set percent, and invalidate draw buffers
    FocusFunc();  
    //overwrite percent..
    percent = 0.50; 
    numVerts = 0;
    scrollVerts = 0;
    scrollPos = 0.0;
    //generates vboPos and vboTc
    CreateGLBuffers(); 
}

UISlider::~UISlider()
{
    if (scrollVerts)
        delete[] scrollVerts;
}


void UISlider::FocusFunc()
{
    if (cursorId < 0)
    {
        //LogError("CURSOR ID IS INVALID!!!!!!");
        return;
    }
    
    float mouseloc;
    float size;
    float btnHalfLen = parentWindow->skin.getInfo(UI_SKIN_SLIDER)->height/2.0;
    if (vertical)
    {
        //mouseloc in local space.
        mouseloc = gInput->GetCoords(cursorId).y - position.y; 

        if (mouseloc == lastloc) //don't waste time on non-scrolls
            return;

        size = dimensions.y;
        //check if on end caps or not, remember y is top oriented in UI      
        if (mouseloc < btnHalfLen)
        {
            mouseloc = btnHalfLen;
            percent  = 1.0;
        }
        else if (mouseloc > dimensions.y - btnHalfLen)
        {
            mouseloc = dimensions.y - btnHalfLen;
            percent  = 0.0;
        }
        else //adjust the value with the end cap half length, invert percentage
            percent = 1.0 - (mouseloc - btnHalfLen) / (size - (btnHalfLen+btnHalfLen));
    }
    else
    {
        //get mouseloc in local space
        mouseloc = gInput->GetCoords(cursorId).x - position.x;

        if (mouseloc == lastloc)
            return;
        
        size = dimensions.x;
       
        //check if on end caps
        if (mouseloc < btnHalfLen)
        {
            mouseloc = 0;
            percent  = 0.0;
        }
        else if (mouseloc > dimensions.x - btnHalfLen)
        {
            mouseloc = dimensions.x;
            percent  = 1.0;
        }
        else //adjust the value with the end cap half length as inner margins
            percent = (mouseloc - btnHalfLen) / (size - (btnHalfLen+btnHalfLen));
    }

    hasScrolled = true;
    
    //if (vertical) //vertical sliders start at bottom
    //    percent = 1.0 - percent;

    scrollPos = mouseloc; 
    value = percent * max;
    lastloc = mouseloc;
    Invalidate();
}


void UISlider::SetPercent(float p)
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
    value     = p * max;
    percent   = p;
    Invalidate();

}

void UISlider::SetValue(float val)
{
    if (val > max)
        val = max;
    if (val < 0.0 /*min*/) //TODO negative values
        val = 0.0;

    bool vertical = dimensions.y > dimensions.x;
    float axisDim = vertical ? dimensions.y : dimensions.x;
    SetPercent(axisDim * (max / val));
}

void UISlider::AddToWindow(UIWindow *win)
{
    UIElement::AddToWindow(win);

    //detect if its vertical or horizontal
    vertical = dimensions.y > dimensions.x;

    //adjust dimensions to skin size
    float skWidth = win->skin.getInfo(UI_SKIN_SLIDER)->width;
    if (vertical)
        dimensions.x = skWidth;
    else
        dimensions.y = skWidth;


    numVerts = win->skin.getNumVerts(UI_SKIN_SLIDER);
    vboTc    = win->skin.getTexcoords(UI_SKIN_SLIDER); 
    //just delete the old ones    
    if (scrollVerts)
    {
        delete[] scrollVerts;
    }
    scrollVerts = new Vector2[numVerts];

    //set initial position.
    SetPercent(percent);

}

void UISlider::Invalidate()
{
    UIElement::Invalidate();
    
    unsigned int num; 
    parentWindow->skin.BuildSlider(position, dimensions, scrollPos,
                                      scrollVerts, &num); 

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * num, 
                 scrollVerts, GL_DYNAMIC_DRAW); 
      
        
}

void UISlider::Draw()
{
    if (hidden)
        return;
   
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 
            2, GL_FLOAT, GL_FALSE, 0,0);

    glBindBuffer(GL_ARRAY_BUFFER, vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
            GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);

   
   // glEnable(GL_STENCIL_TEST);
}


