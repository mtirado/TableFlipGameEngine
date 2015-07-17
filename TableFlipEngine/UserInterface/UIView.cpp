/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIView.h"
#include "UITextOut.h"

UIView::UIView()
{
    elementId = ELEM_UIVIEW;
    focusElement = 0;
    hoverElement = 0;
    //use buffers for highlighting or whatever in subclasses 
    CreateGLBuffers();

    theWin.SetBGTexcoords(Vector2(5.0 / theWin.skin.getAtlasRes(), 1.0));
}

void UIView::SetDimensions(Vector2 dim)
{
    UIElement::SetDimensions(dim);
    theWin.dimensions = dimensions;
}

void UIView::HoverFunc()
{
    Vector2 pt;
    pt.x = Input::GetInstance()->GetXCoords(0);
    pt.y = Input::GetInstance()->GetYCoords(0);
    UIElement *hover = theWin.MouseHover(pt);
    //manage hover element, and call hover notifications
    if (hover != hoverElement)
    {
        if (hoverElement)
            hoverElement->MouseOut();
        hoverElement = hover;
        hover->MouseOver();   
    }
}

void UIView::MouseOut()
{
    if (hoverElement)
        hoverElement->MouseOut();
}

void UIView::MouseOver() 
{
    HoverFunc();
}

void UIView::FocusFunc()
{
    //TODO probably going to need some kinda view list for updates
    //yup, AND dynamic text, TODO thisnext time 
    if (focusElement)
        focusElement->FocusFunc();
}

void UIView::DropFocus()
{ 
    if (focusElement)
        focusElement->DropFocus();
}

void UIView::MouseUp()
{
    if (focusElement)
        focusElement->MouseUp();
}

void UIView::MouseDown(Vector2 pt)
{
    focusElement = theWin.MouseDown(pt);
    if (focusElement)
    {
        focusElement->cursorId = cursorId;
        focusElement->MouseDown(pt);
    }
}

void UIView::CreateScrollbars(bool horizon, bool vert)
{
    theWin.CreateScrollbars(horizon, vert);
}

void UIView::CreateFrame()
{
    theWin.CreateFrame();
}

void UIView::Invalidate()
{
    theWin.dimensions = dimensions;
    UIElement::Invalidate();
    //call move window
    theWin.MoveWindow(position);
}


void UIView::Draw()
{
    if (theWin.hidden)
        return;
   
    theWin.Update();
    DrawWindow();
    DrawElems();
    DrawPreText();
    DrawText();
    //post text is called in draw text
}

void UIView::SetBGTexcoords(Vector2 tc)
{
    theWin,SetBGTexcoords(tc);
}

void UIView::DrawWindow()
{
    //it increments
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glStencilFunc(GL_EQUAL, winStencilId, 0xff);
    
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    //ui style
    gUIManager->matUIElement.SendUniformData();
    glBindBuffer(GL_ARRAY_BUFFER, theWin.vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation()); //vertpos location
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 2, GL_FLOAT, GL_FALSE,0,0);

    //vert texcoords
    glBindBuffer(GL_ARRAY_BUFFER, theWin.vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, GL_FLOAT, GL_FALSE,0,0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //frame?
    if (theWin.vboFrame)
    {
        //go up back a level so frame is not overwritten
        glStencilOp(GL_DECR, GL_DECR, GL_DECR);
        glStencilFunc(GL_EQUAL, gUIManager->winStencilId+1, 0xff);

        int numVerts = theWin.skin.getNumVerts(UI_SKIN_FRAME);
        //positions
        glBindBuffer(GL_ARRAY_BUFFER, theWin.vboFrame);
        glEnableVertexAttribArray(gUIManager->GetPositionLocation());
        glVertexAttribPointer(gUIManager->GetPositionLocation(),
                2, GL_FLOAT, GL_FALSE, 0, 0);

        //texcoords
        glBindBuffer(GL_ARRAY_BUFFER, theWin.skin.getTexcoords(UI_SKIN_FRAME));
        glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
        glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 
                2, GL_FLOAT, GL_FALSE,0,0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);
    }

}

void UIView::DrawElems()
{
    //enumerate views and render them after this window
    //vector<UIElement *> views;

    //draw elements
    glStencilFunc(GL_EQUAL, winStencilId+1, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    UIElement *elem = theWin.elements;
    for ( ; elem != 0; elem = elem->next)
    {
        //elem = theWin.elements[i];
        if (!elem->isHidden())
        {
            //views inside views scared me.
            /*if (elem->getElementId() == ELEM_UIVIEW)
            {
                views.push_back(elem);
                continue;
            }*/

            //TODO if they're off screen don't draw
            if(elem->scrolls)
            {                                                  //a lil buffer zone
                if (elem->getPosition().y > position.y + dimensions.y + 50.0)
                    continue;
                if (elem->getPosition().x > position.x + dimensions.x + 50.0)
                    continue;
            }

            elem->Draw();
        }
    }

    //draw scrollbars
    UIElement *scrl = theWin.getVScroll();
    if(scrl)
       scrl->Draw();
    scrl = theWin.getHScroll();
    if(scrl)
       scrl->Draw();
}

void UIView::DrawPreText()
{
    //this is where any extra draw code (before text) would go
}

void UIView::DrawText()
{
    //glStencilFunc(GL_ALWAYS, winStencilId+1, 0xff);

    Matrix4 ident, scale, model;
    Matrix4Identity(ident);
    //TODO remove extra matrices, in UIWindow too.
    //this assumes static and dynamic text use the same shader
    glUseProgram( theWin.staticTextBuffer->shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( theWin.staticTextBuffer->shader,
                    "model" ),1, 0, ident);
        glUniformMatrix4fv( glGetUniformLocation( theWin.staticTextBuffer->shader,
                    "view" ),1, 0, ident);
        glUniformMatrix4fv( glGetUniformLocation( theWin.staticTextBuffer->shader,
                    "projection" ),1, 0,
                gRenderer->GetFontFrustum()->projMatrix);
        text_buffer_render( theWin.staticTextBuffer );
    }
    glUseProgram(gUIManager->matUIElement.program);
    glDisable(GL_DEPTH_TEST);

    //TODO i didnt actually test this
    DrawPostText();


    //views within views need to be drawn too...?
    

    //draw views at end to increment window stencil id
    //UIView *v;

    //after each view is drawn, decrement that stencil buffer
    glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
    glStencilFunc(GL_EQUAL, winStencilId+1, 0xff);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


    //bring stencil id back down to windows id  
    glBindBuffer(GL_ARRAY_BUFFER, theWin.vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation()); //vertpos location
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 2, GL_FLOAT, GL_FALSE,0,0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

}

void UIView::DrawPostText()
{
    //this is for drawing, YUP after text
}
