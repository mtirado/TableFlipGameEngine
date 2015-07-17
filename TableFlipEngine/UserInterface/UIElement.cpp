/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UserInterface.h"
#include "UIWindow.h"

UIElement::~UIElement()
{
    if (vboTc)
        glDeleteBuffers(1, &vboTc);
    if (vboPos)
        glDeleteBuffers(1, &vboPos);
    //drop focus if we killed currently focused element.
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
    {
        if (UIManager::GetInstance()->hasFocus[i] == this)
            UIManager::GetInstance()->hasFocus[i] = 0;
    }

#if defined(UI_DEBUG_MODE)
    glDeleteBuffers(1, &vboDbgTc);
    glDeleteBuffers(1, &vboDbgRect);
#endif

}
UIElement::UIElement()
{
    //elements are linked list nodes too
    next = 0;
    data = 0;
    //everything scrolls by default
    scrolls    = true;
    elementId  = ELEM_DEFAULT;
    offset     = Vector2(0.0);
    dimensions = Vector2(0.0);
    position   = Vector2(0.0);
    color      = Vector4(0.0);
    cursorId   = -1;
    actionCallback = 0;
    //every element needs a parent window.
    parentWindow = gUIManager->GetRootWindow();

    vboPos = 0;
    vboTc  = 0;

    retainFocus = false;
    hidden = false;

#if defined(UI_DEBUG_MODE)
    glGenBuffers(1, &vboDbgTc);
    glGenBuffers(1, &vboDbgRect);
#endif

}

//buffers are manually created by subclasses
void UIElement::CreateGLBuffers()
{
    if (!vboPos)
        glGenBuffers(1, &vboPos);
    if (!vboTc)
        glGenBuffers(1, &vboTc);
}

void UIElement::AddToWindow(UIWindow* wnd)
{
    parentWindow = wnd;
    Invalidate();
}


void UIElement::MouseUp()
{

    //if (UIManager::GetInstance()->hasFocus[cursorId] == this)
    //  UIManager::GetInstance()->hasFocus[cursorId] = 0;
}

//updates position, no drawing dome here
void UIElement::Invalidate()
{
    position = parentWindow->GetPosition() + offset;
    if (scrolls)
        position += parentWindow->getScrollOffset();

#if defined(UI_DEBUG_MODE)

    //texcoords -- one pixel on the UI atlas
    Vector2 tc = Vector2(0.0, 1.0);
    float dataTC[4][2] =  {
        { tc.x, tc.y },
        { tc.x, tc.y },
        { tc.x, tc.y },
        { tc.x, tc.y },
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboDbgTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(),
            2, GL_FLOAT, GL_FALSE, 0,0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, 
            dataTC, GL_DYNAMIC_DRAW);

    //positions
    float data[4][2] =  {
        {  position.x, position.y + dimensions.y },
        {  position.x, position.y },
        {  position.x + dimensions.x, position.y + dimensions.y },
        {  position.x + dimensions.x, position.y }
    };
    //memset(data, 0, sizeof(float)*12);
    glBindBuffer(GL_ARRAY_BUFFER, vboDbgRect);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8,
            data, GL_DYNAMIC_DRAW);

#endif
}

#if defined(UI_DEBUG_MODE)
void UIElement::DebugDraw()
{
    glBindBuffer(GL_ARRAY_BUFFER, vboDbgRect);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 
            2, GL_FLOAT, GL_FALSE,0,0);

    glBindBuffer(GL_ARRAY_BUFFER, vboDbgTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(),
            2, GL_FLOAT, GL_FALSE,0,0);
    
    //draw it.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}
#endif

