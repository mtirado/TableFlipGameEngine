/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIListBox.h"
#include "UISlider.h"
#include "stdlib.h"

UIListBox::UIListBox()
{
    //itemHeight = 18;
    selection = 0;
    hoverElem = 0;
    //top left, 2nd pixel over
    //theWin.SetBGTexcoords(Vector2(5.0 / theWin.skin.getAtlasRes(), 1.0));

    SetHoverCoords(Vector2(6.0 / theWin.skin.getAtlasRes(), 1.0));
    SetHighlightCoords(Vector2(7.0 / theWin.skin.getAtlasRes(), 1.0));
    
    theWin.setWindowPadding(0.0, 0.0);
}

UIListBox::~UIListBox()
{
    //if (items)
    //    ClearItems();
}


void UIListBox::MouseOut()
{
    UIView::MouseOut();
    hoverElem = 0;
    InvalidateSelection();
}


void UIListBox::MouseDown(Vector2 pt)
{

    UIView::MouseDown(pt);
    // UISlider *scrl = theWin.getVScroll();
    //convert to list box space
    //UITextOut *elem = (UITextOut *)theWin.elements;
    //for ( ; elem != 0; elem = (UITextOut *)elem->next)
    //
    UIElement *elem = focusElement;
    if (elem)
    {

        if (elem->getElementId() == ELEM_UITEXTOUT)
        {
            vec4 w = {1,1,1,1};
            vec4 r = {1,0,0,1};
            vec4 b = {0,0,0,1};
            if (selection)
                selection->fontStyle.foreground_color = b;

            ((UITextOut *)elem)->fontStyle.foreground_color = w;

            selection = (UITextOut *)elem;
            InvalidateSelection();
            theWin.InvalidateStaticText();
        }
    }

}

void UIListBox::SetHoverCoords(Vector2 tc)
{
    hoverCoords = tc;
    InvalidateTexcoords();
}

void UIListBox::SetHighlightCoords(Vector2 tc)
{
    highlightCoords = tc;
    InvalidateTexcoords(); 
}

void UIListBox::InvalidateTexcoords()
{
    //holds hover, and highlight verts
    float dataTC[10][2] =  {

        { hoverCoords.x, hoverCoords.y },
        { hoverCoords.x, hoverCoords.y },
        { hoverCoords.x, hoverCoords.y },
        { hoverCoords.x, hoverCoords.y },

        //degen
        { 0.0, 0.0 },
        { 0.0, 0.0 },

        { highlightCoords.x, highlightCoords.y },
        { highlightCoords.x, highlightCoords.y },
        { highlightCoords.x, highlightCoords.y },
        { highlightCoords.x, highlightCoords.y },
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(),2,GL_FLOAT,GL_FALSE,0,0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, dataTC, GL_DYNAMIC_DRAW);

}

void UIListBox::InvalidateSelection()
{
    Vector2 selPos(-25000, -25000);
    Vector2 selDim(0,0);
    Vector2 hovPos = selPos;
    Vector2 hovDim = selDim;
    if (selection)
    {
        selPos = selection->getPosition(); 
        selDim = selection->getDimensions();
        Vector4 windowArea = theWin.getWindowArea();
        //check for rectangle adjustments that need to be made
        Vector2 selClip = selPos + selDim;
        if (selClip.x > selPos.x + windowArea.y) //area is a rect, not normal x,y
            selDim.x -= selClip.x - (selPos.x + windowArea.y);
        //if (selClip.y > windowArea.w)
        //    selDim.y -= windowArea.w - selClip.y;

    }

    if (hoverElem)
    {
        hovPos = hoverElem->getPosition(); 
        hovDim = hoverElem->getDimensions();
        Vector4 windowArea = theWin.getWindowArea();
        //check for rectangle adjustments that need to be made
        Vector2 hovClip = hovPos + hovDim;
        if (hovClip.x > hovPos.x + windowArea.y) //area is a rect, not normal x,y
            hovDim.x -= hovClip.x - (hovPos.x + windowArea.y);

       // hovPos.x -= windowArea.x;
    }

    float data[10][2] =  {

        {  hovPos.x, hovPos.y + hovDim.y },
        {  hovPos.x, hovPos.y },
        {  hovPos.x + hovDim.x, hovPos.y + hovDim.y },
        {  hovPos.x + hovDim.x, hovPos.y },

        //degen
        {  hovPos.x + hovDim.x, hovPos.y },
        {  selPos.x, selPos.y + selDim.y },

        {  selPos.x, selPos.y + selDim.y },
        {  selPos.x, selPos.y },
        {  selPos.x + selDim.x, selPos.y + selDim.y },
        {  selPos.x + selDim.x, selPos.y }
    };

    //memset(data, 0, sizeof(float)*12);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20, data, GL_DYNAMIC_DRAW);

}

void UIListBox::Invalidate()
{
    UIView::Invalidate();
    InvalidateSelection();
}

void UIListBox::HoverFunc()
{
    //hovering on a list item?
    Vector2 pt;
    pt.x = Input::GetInstance()->GetXCoords(0);
    pt.y = Input::GetInstance()->GetYCoords(0);
    UIElement *elem = theWin.MouseHover(pt);

    hoverElem = 0;

    //TODO DropHover
    if (elem)
    {
        //only track textouts
        if (elem->getElementId() == ELEM_UITEXTOUT)
        {
            hoverElem = (UITextOut *)elem; 
        }
    }
    InvalidateSelection();

}

void UIListBox::DrawPreText()
{
    //update selector text before it draws
    Update();

    //draw highlight
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation()); //vertpos location
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 
            2, GL_FLOAT, GL_FALSE,0,0);

    //vert texcoords
    glBindBuffer(GL_ARRAY_BUFFER, vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 
            2, GL_FLOAT, GL_FALSE,0,0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 10); //2 quads + degenerate tri

}

void UIListBox::Update()
{
    //update selector
    if (theWin.hasScrolled())
    {
        InvalidateSelection();
    }

}

UIElement *UIListBox::AddToList(wchar_t *text, void *data)
{
    UITextOut *txt = new UITextOut();
    float height = ptToPx(18);
    txt->SetFontSize(18);
    txt->output = text;
   
    //set width (minus win dressings)
    Vector4 area    = theWin.getWindowArea();
    float areaWidth = area.y - area.x + theWin.getWindowPadding().x*2;
    txt->SetOffset(Vector2(0.0, theWin.getYMax()));
    txt->SetDimensions(Vector2(areaWidth, height));

    txt->setData(data);     //attach any user data
    theWin.AddElement(txt);
    //adjust x offset after adding (window can organize elements on add)
    //TODO i odn't like how this must be done after elements are added
    txt->offset.x;
    txt->offset.y;
    txt->textOffset = Vector2(2.0, 0.0);
    txt->SetDimensions(Vector2(areaWidth, height));

   /* char d[512];
    wcstombs(d, text, 512);
    LOGOUTPUT << "add to list: " << d <<
        "\npos: " << txt->offset << "\ndim: " << txt->getDimensions()
        << "\n areaWidth: " << areaWidth;
    LogInfo();*/


    return txt;

}

bool UIListBox::RemoveFromList(UIElement *elem)
{
    bool ret = theWin.RemoveElement(elem);
    if (ret)
        delete elem;
    return ret;
}
