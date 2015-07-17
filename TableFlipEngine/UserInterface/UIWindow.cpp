/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIWindow.h"
#include "UITextOut.h"
#include "UISlider.h"
#include "UIView.h"

UIWindow::UIWindow()
{
    elements = 0;
    sortFunc = 0;
    scrolls = false;
    vScroll = 0;
    hScroll = 0;
    updateStaticText = false;
    glGenBuffers(1, &vboPos);
    glGenBuffers(1, &vboTc);
    vboFrame = 0;
    Title = "GL Window";
    hidden = false;
    canDock = false;
    showTitlebar = true;

    winScrolled = false;
    scrollOffset = Vector2(0.0, 0.0);
    scrollDimensions = Vector2(0.0,0.0);
    elemMax = Vector2(0.0, 0.0);
    //top left region has window pallete
    SetBGTexcoords(Vector2(4.0 / skin.getAtlasRes(), 1.0));

    //2px default window padding
    windowPadding = Vector2(2.0);

    //no frame by default
    framePixels = 0.0;
    frameVerts  = 0;

    //3 is some fancy LCD smoothing?
    staticTextBuffer  = text_buffer_new( LCD_FILTERING_ON );

}

UIWindow::~UIWindow()
{
    if (frameVerts)
        delete frameVerts;

    glDeleteBuffers(1, &vboTc);
    glDeleteBuffers(1, &vboPos);
    UIElement *elem = elements;
    UIElement *junk;
    while (elem != 0)
    {
        junk = elem;
        elem = elem->next;
        delete junk;
    }
    text_buffer_delete(staticTextBuffer);
}

//recursive, so # of elements in window is now limited to stack size...
bool findElement(UIElement *elem, UIElement *start)
{
    if (start == NULL)
        return false;
    if (start == elem)
        return true;
    //recurse
    return findElement(elem, start->next);  
}

void UIWindow::AddElement(UIElement *elem, bool ignoreMargin)
{
    //dont add duplicates, these are linked list nodes now
    if (findElement(elem, elements))
    {
#if defined(UI_DEBUG_MODE)
        LogError("duplicate UI node found.  ABORT MISSION!");
        exit(-601);
#endif
        return;
    }

    CalcWindowMargin();

    if (!ignoreMargin)
    {
        //apply margin  (top left only right now)
        if (elem->offset.x < windowArea.x)
            elem->offset.x += windowArea.x - elem->offset.x;
        if (elem->offset.y < windowArea.z)
            elem->offset.y += windowArea.z - elem->offset.y;
    }

    if (sortFunc != 0)
    {

    }
    else
    {
        //insert at front
        elem->next = elements;
        elements = elem;
    }

    elem->AddToWindow(this);

    //update scroll area, if we have scrolbars
    if (vScroll || hScroll)
        CalcScrollDimensions();

    //check if its a new maximum
    Vector2 pt = elem->offset + elem->getDimensions();
    if (pt.y > elemMax.y)
        elemMax.y = pt.y;
    if (pt.x > elemMax.x)
        elemMax.x = pt.x;

    //LOGOUTPUT << "elem max: " << elemMax; LogInfo();

}

void UIWindow::CalcMaximum()
{
    UIElement *e = elements;
    Vector2 max;
    while(e != 0)
    {
        max = e->offset + e->getDimensions();
        if (max.y > elemMax.y)
            elemMax.y = max.y;
        if (max.x > elemMax.x)
            elemMax.x = max.x;
        e = e->next;
    }
}

bool UIWindow::RemoveElement(UIElement *elem)
{
    UIElement *e = elements;
    //check first elem
    if (e == elem)
    {
        elements = e->next;
        e->Invalidate();
        CalcScrollDimensions();
        return true;
    }

    UIElement *prev = e;
    e = e->next;

    while (e != 0)
    {
        if (e == elem)
        {
            prev->next = e->next;
            e->Invalidate();
            //update scroll area
            CalcScrollDimensions();
            //recalculate maximum
            CalcMaximum();
            return true;
        }
        prev = e;
        e = e->next;
    }
    return false;
}


UIElement* UIWindow::MouseDown(Vector2 pt)
{


    //check scrollbars first (incase of overlap)
    if (vScroll)
    {
        if (scrollDimensions.y > 1.0 && 
                ptInRect(pt, position + vScroll->offset, vScroll->getDimensions()))
        {
            //vScroll->MouseDown(pt);//moved
            return vScroll;
        }
    }
    if (hScroll)
    {
        if (scrollDimensions.x > 1.0 && 
                ptInRect(pt,  position + hScroll->offset, hScroll->getDimensions()))
        {
            //hScroll->MouseDown(pt);//moved
            return hScroll;
        }

    }

    UIElement *elem = elements;
    //for (int i = 0; i < elements.size(); i++)
    for ( ; elem != 0; elem = elem->next)
    {
        //elem = elements[i];
        if (elem->isHidden())
            continue;
        //Vector2 windowSpaceOffset = (dimensions * elem->offset) / 1000.0;
        //position = position + windowSpaceOffset;

        Vector2 pos = position + elem->offset;
        if (elem->scrolls)
            pos += scrollOffset;

        if (ptInRect(pt, pos, elem->getDimensions()))
        {
            //elem->MouseDown(pt);
            //moved to after window returns element
            //in either UIView or UserInterface
            return elem;
        }
    }
    return 0;
}

UIElement *UIWindow::MouseHover(Vector2 pt)
{
    //if its on the scrollbars, return 0
    if (vScroll)
    {
        if (scrollDimensions.y > 1.0 && 
                ptInRect(pt, position + vScroll->offset, vScroll->getDimensions()))
        {
            vScroll->HoverFunc();
            return 0;
        }
    }
    if (hScroll)
    {
        if (scrollDimensions.x > 1.0 && 
                ptInRect(pt,  position + hScroll->offset, hScroll->getDimensions()))
        {
            hScroll->HoverFunc();
            return 0;
        }

    }


    //get point in window space
    Vector2 winpt = pt - position; 
    //if on frame, return 0
    if (winpt.x < windowArea.x || winpt.x > windowArea.y)
        return 0;
    if (winpt.y < windowArea.z || winpt.y > windowArea.w)
        return 0;

    UIElement *elem = elements;
    //for (int i = 0; i < elements.size(); i++)
    for ( ; elem != 0 ; elem = elem->next)
    {
        //elem = elements[i];
        if (elem->isHidden())
            continue;

        Vector2 pos = position + elem->offset;
        if (elem->scrolls)
            pos += scrollOffset;

        if (ptInRect(pt, pos, elem->getDimensions()))
        {
            //elem->HoverFunc();
            //moved outside to after MouseHover call
            return elem;
        }
    }
    return 0;
}

void UIWindow::MouseUp()
{
    //if (UIManager::GetInstance()->hasFocus == this)
    //   UIManager::GetInstance()->hasFocus = 0;
}


void UIWindow::CalcWindowMargin()
{
    //not in screen space, use window space
    float vs = 0.0;
    float hs = 0.0;

    if (vScroll)
        vs = vScroll->dimensions.x;
    if (hScroll)
        hs = hScroll->dimensions.y;

    //TODO test y,w margins. only need the top left margin right now...

    //left, right, top, bottom
    windowArea.x = windowPadding.x + framePixels;
    windowArea.y = dimensions.x    - windowPadding.x - framePixels - vs;
    windowArea.z = windowPadding.y + framePixels;
    windowArea.w = dimensions.y    - windowPadding.y - framePixels - hs;

}

void UIWindow::SetBGTexcoords(Vector2 tc)
{
    //texcoords
    float dataTC[4][2] =  {
        { tc.x, tc.y },
        { tc.x, tc.y },
        { tc.x, tc.y },
        { tc.x, tc.y },
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(),2,GL_FLOAT,GL_FALSE,0,0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, dataTC, GL_DYNAMIC_DRAW);

}

void UIWindow::MoveWindow ( Vector2 pt )
{
    position = pt;
    float data[4][2] =  {

        {  position.x, position.y + dimensions.y },
        {  position.x, position.y },
        {  position.x + dimensions.x, position.y + dimensions.y },
        {  position.x + dimensions.x, position.y }
    };
    //memset(data, 0, sizeof(float)*12);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, data, GL_DYNAMIC_DRAW);


    if (framePixels > 0.0)
    {
        InvalidateFrame();
    }

    //update elements in window.
    UIElement *elem = elements;
    for ( ; elem != 0 ; elem = elem->next)
        elem->Invalidate();

    if (vScroll)
        vScroll->Invalidate();
    if (hScroll)
        hScroll->Invalidate();

    //update windows text
    UpdateStaticText();
}

void UIWindow::FocusFunc()
{

}

void UIWindow::ScrollWindow()
{
    bool scrl = false;
    if (vScroll)
        if (vScroll->checkScrolled())
        {
            scrollOffset.y = scrollDimensions.y *  -vScroll->percent ;
            scrl = true;
        }
    if (hScroll)
        if (hScroll->checkScrolled())
        {
            scrollOffset.x = scrollDimensions.x * -hScroll->percent;
            scrl = true;
        }

    //window scrolled, now we must update every child elements position
    if (scrl)
    {
        winScrolled = true;
        MoveWindow(position);
        /*UIElement *elem = elements;
          for( ; elem != 0; elem = elem->next)
          elem->Invalidate();

          UpdateStaticText();*/
    }
    else
        winScrolled = false;
}

void UIWindow::Update()
{
    //check for scrolls
    ScrollWindow();

    //static text buffer has been invalidated. 
    if (updateStaticText)
        UpdateStaticText();
}

//view has a different draw because of stencil madness
void UIWindow::Draw()
{
    if (hidden)
        return;

    //set the windows base stencil id.
    winStencilId = gUIManager->winStencilId;

    glUseProgram(gUIManager->matUIElement.program);      
    gUIManager->matUIElement.SetProjectionMatrix(
            gRenderer->GetUIFrustum()->projMatrix);

    //do not draw root window area...
    if (this != gUIManager->GetRootWindow())
    {
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, gUIManager->winStencilId, 0xff);

        gUIManager->matUIElement.SendUniformData();

        glBindBuffer(GL_ARRAY_BUFFER, vboPos);
        glEnableVertexAttribArray(gUIManager->GetPositionLocation()); //vertpos location
        glVertexAttribPointer(gUIManager->GetPositionLocation(), 
                2, GL_FLOAT, GL_FALSE,0,0);

        //vert texcoords
        glBindBuffer(GL_ARRAY_BUFFER, vboTc);
        glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
        glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 
                2, GL_FLOAT, GL_FALSE,0,0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        if (vboFrame)
        {
            //go up back a level so frame is not overwritten
            glStencilOp(GL_DECR, GL_DECR, GL_DECR);
            glStencilFunc(GL_ALWAYS, gUIManager->winStencilId, 0xff);

            int numVerts = skin.getNumVerts(UI_SKIN_FRAME);
            // LOGOUTPUT << "numverts: " << numVerts; LogInfo();
            //positions
            glBindBuffer(GL_ARRAY_BUFFER, vboFrame);
            glEnableVertexAttribArray(gUIManager->GetPositionLocation());
            glVertexAttribPointer(gUIManager->GetPositionLocation(),
                    2, GL_FLOAT, GL_FALSE, 0, 0);

            //texcoords
            glBindBuffer(GL_ARRAY_BUFFER, skin.getTexcoords(UI_SKIN_FRAME));
            glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
            glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 
                    2, GL_FLOAT, GL_FALSE,0,0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);
        }


    }
    else
        gUIManager->matUIElement.SendUniformData();
    //enumerate views and render them after this window
    vector<UIElement *> views;

    //draw where stencil value is the right value (inside window)
    glStencilFunc(GL_EQUAL, gUIManager->winStencilId, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    UIElement *elem = elements;
    //for (int i = 0; i < elements.size(); i++)
    for ( ; elem != 0; elem = elem->next)
    {
        //elem = elements[i];
        if (!elem->isHidden())
        {
            if (elem->getElementId() == ELEM_UIVIEW)
            {
                views.push_back(elem);
                continue;
            }

            //if they're off screen don't draw
            if(elem->scrolls)
            {                                                       //a lil buffer zone
                if (elem->getPosition().y > position.y + dimensions.y + 50.0)
                    continue;
                if (elem->getPosition().x > position.x + dimensions.x + 50.0)
                    continue;
            }

            //just to be safe i'm going to reset frame color and thickness here
            //elem->frameColor.w = elem->frameThickness;
            //gUIManager->matUIElement.SetUniformData("color", &elem->color);
            //gUIManager->matUIElement.SetUniformData("frameColor", &elements[i]->frameColor);
            //gUIManager->matUIElement.SetUniformData("aspectScale",
            //                         const_cast<Vector2 *>(&elements[i]->getAspectScale()));

            gUIManager->matUIElement.SendUniformData();

#if defined(UI_DEBUG_MODE)
            elem->DebugDraw();
#endif

            elem->Draw();

        }
    }

    if (vScroll)
        vScroll->Draw();
    if (hScroll)
        hScroll->Draw();
    //draw text
    //glColor4f(1.00,1.00,1.00,1.00);
    Matrix4 ident, scale, model;
    Matrix4Identity(ident);
    glUseProgram( staticTextBuffer->shader );
    {
        //TODO model/view matrices are wasting bandwidth / gpu time
        //(in view class too)
        glUniformMatrix4fv( glGetUniformLocation( staticTextBuffer->shader,
                    "model" ),1, 0, ident);
        glUniformMatrix4fv( glGetUniformLocation( staticTextBuffer->shader,
                    "view"  ),1, 0, ident);
        glUniformMatrix4fv( glGetUniformLocation( staticTextBuffer->shader,
                    "projection"  ),1, 0,
                gRenderer->GetFontFrustum()->projMatrix);
        text_buffer_render( staticTextBuffer );
    }

    glDisable(GL_DEPTH_TEST);
    glUseProgram(gUIManager->matUIElement.program);

    UIView *v;
    //draw views at end to increment window stencil id
    for (unsigned int i = 0; i < views.size(); ++i)
    {
        v = (UIView *)views[i];
        v->winStencilId = gUIManager->winStencilId;
        v->Draw();
    }

    //go back to window level
    //glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    //glStencilFunc(GL_ALWAYS, gUIManager->winStencilId, 0xff);
    //draw frame, if applicable

}



bool findTextElement(UITextOut *elem, std::vector<UITextOut *> &vec)
{
    for (int i = 0; i < vec.size(); i++)
        if (vec[i] == elem)
            return true;

    return false;
}

//add else's for error checks if this gets screwey
void UIWindow::AddStaticText(UITextOut *txt)
{

    if (findTextElement(txt, staticText))
        return;
    staticText.push_back(txt);
    //rewrite whole butter
    InvalidateStaticText();
}

void UIWindow::RemoveStaticText(UITextOut *txt)
{
    for (int i = 0; i < staticText.size(); i++)
        if (staticText[i] == txt)
        {
            staticText.erase(staticText.begin()+i);
            InvalidateStaticText();
            return;
        }
}

void UIWindow::UpdateText(text_buffer_t *buffer, std::vector<UITextOut *> &texts)
{
    text_buffer_clear(buffer);
    for (int i = 0; i < texts.size(); i++)
    {
        UITextOut *txt = texts[i];
        
        //set dimensions -- should be other way around...
        buffer->dimensions.x = txt->dimensions.x;
        buffer->dimensions.y = txt->dimensions.y;
        vec2 pen;

        //mouse down text cursor stuff will need to also correct for offset
        pen.x = txt->getPosition().x + txt->textOffset.x;
        pen.y = txt->getPosition().y + txt->textOffset.y;;
        //TODO word wrap, alignment, scroll view check, etc.
        text_buffer_pinfo info = 
        text_buffer_printf_index(buffer, &pen,
                &txt->fontStyle, const_cast<wchar_t *>(txt->output.c_str()));
        int idx = info.startIdx;
#if defined(UI_DEBUG_MODE)
        if (idx < 0)
        {
            LogError("UIWindow::UpdateText() -- text_buffer_printf_index() error");
        }
#endif
        txt->bufferIndex = idx; //set the start index
        txt->glyphsDrawn = info.numGlyphs;
        txt->lastGlyph   = info.lastGlyph;


               
        //calculate text dimensions, pen has been updated
        //seems odd, but i can't quickly get absolute text dimensions 
        //untill it's rendered,  
        //just wait untill after initial text update i guess...?
        txt->textDimensions = Vector2(info.width, info.height);
    }
}

void UIWindow::UpdateStaticText()
{
    UpdateText(staticTextBuffer, staticText);
    updateStaticText = false;
}

void UIWindow::UpdateTextDimensions()
{
        UpdateStaticText();
}

//TODO allow for scrollbars to be removed,
void UIWindow::CreateScrollbars(bool horizon, bool vert)
{

    float scrlSize = skin.getInfo(UI_SKIN_SCROLLBAR)->width;
    //adjust for frame thickness, and scrollbar dimensions
    if (vert)
    {
        vScroll = new UIScrollbar(0.0, 100.0);
        vScroll->scrolls = false;
        vScroll->vertical = true;
        vScroll->SetOffset(Vector2(dimensions.x - scrlSize - framePixels, framePixels));
        vScroll->SetDimensions(Vector2(scrlSize, dimensions.y - framePixels*2));
        vScroll->AddToWindow(this);
        //AddElement(vScroll, true);
    }
    if (horizon)
    {
        hScroll = new UIScrollbar(0.0, 100.0);
        hScroll->scrolls = false;
        hScroll->vertical = false;
        hScroll->SetOffset(Vector2(framePixels, dimensions.y - scrlSize - framePixels));
        hScroll->SetDimensions(Vector2(dimensions.x - framePixels*2, scrlSize));
        hScroll->AddToWindow(this);
        //AddElement(hScroll, true);
    }
    CalcScrollDimensions();
    CalcWindowMargin();
}

//TODO i guess just proper window padding in general
//so it doesnt snap to element edge on scrollbars
//Vector2 scrollPadding(6.0, 6.0); 

void UIWindow::CalcScrollDimensions()
{
    //size of scroll buttons
    float scrlSize = skin.getInfo(UI_SKIN_SCROLLBAR)->width;

    float xMax = 0.0f;
    float yMax = 0.0f;
    Vector2 dim;
    Vector2 pos;
    //account for frame and scrollbar widths
    Vector2 scrollPadding = Vector2(framePixels);
    if (vScroll)
        scrollPadding.x += scrlSize;
    if (hScroll)
        scrollPadding.y += scrlSize;

    Vector2 scrl = scrollPadding;

    UIElement *el = elements;
    //for (int i = 0; i < elements.size(); ++i)
    for ( ; el != 0; el = el->next)
    {
        //UIElement *el = elements[i];
        if (!el->scrolls)
            continue;

        dim = el->getDimensions();
        //only scroll if overflowing outside window
        pos = el->offset - (dimensions - scrl);
        if (pos.x + dim.x > xMax)
            xMax = pos.x + dim.x;
        if (pos.y + dim.y > yMax)
            yMax = pos.y + dim.y; 
    }
    scrollDimensions.x = xMax;
    scrollDimensions.y = yMax;

    if (hScroll)
    {
        if (xMax < 0.001)
        {
            hScroll->Hide();
        }
        else if (hScroll->isHidden())
            hScroll->Show();
    }
    if (vScroll)
    {
        if (yMax < 0.001)
        {
            vScroll->Hide();
        }
        else if(vScroll->isHidden())
            vScroll->Show();
    }


}

void UIWindow::CreateFrame()
{
    if (frameVerts)
        return;
    //only do this once
    glGenBuffers(1, &vboFrame);
    framePixels = skin.getInfo(UI_SKIN_FRAME)->width;
    frameVerts  = new Vector2[skin.getNumVerts(UI_SKIN_FRAME)];
    CalcWindowMargin();
    MoveWindow(position);
}

void UIWindow::InvalidateFrame()
{
    //get frames coords
    unsigned int size;
    skin.BuildFrame(position, dimensions, frameVerts, &size);
    glBindBuffer(GL_ARRAY_BUFFER, vboFrame);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * size, frameVerts, GL_DYNAMIC_DRAW);        

}

void UIWindow::InvalidateElements()
{
    MoveWindow(position);
    UIElement *elem = elements;
    //for (int i = 0; i < elements.size(); i++)
    for ( ; elem != 0; elem = elem->next)
    {
        //calling show will invalidate
        if (!elem->isHidden())
        {
            elem->Invalidate();
        }
    }

    if (vScroll)
        vScroll->Invalidate();
    if (hScroll)
        hScroll->Invalidate();

}


