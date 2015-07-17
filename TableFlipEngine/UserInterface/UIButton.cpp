/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIButton.h"

UIButton::UIButton()
{   
    hovering = false; 
    down = false; 
    wasDown = false;
    CreateGLBuffers();
    frameVerts  = 0;
    textOffset = Vector2(0.0);
}
UIButton::~UIButton()
{
}

void UIButton::MouseDown(Vector2 pt)
{
    down = true; 
    wasDown = true;
}

void UIButton::MouseUp()
{ 
    down = false;
}

void UIButton::HoverFunc()
{ 
    hovering = true;
}


void UIButton::AddToWindow(UIWindow *win)
{
    frameVerts = new Vector2[win->skin.getNumVerts(UI_SKIN_BTN)];
    //invalidates and adds
    UIElement::AddToWindow(win);
    text.offset = offset;
    text.SetDimensions(dimensions);
    text.AddToWindow(win);
    //note, this is a hack to get the exact rendered text dimensions
    //double window text update 
    
    //this should also  be done if the button's dimensions change too.
    win->UpdateTextDimensions(); //ensures latest dimensions
    Vector2 center = dimensions / 2.0;
    center.y -= text.getTextDimensions().y / 4.0; //use text center

    Vector2 textHalf = text.getTextDimensions() / 2.0;
    LOGOUTPUT << "center: " << center << "\ntextHalf: " << textHalf;
    LogInfo();
    textOffset = center - textHalf;
    text.Invalidate();
    //text.getTextDimensions() / 2.0; 
    


}

void UIButton::Draw()
{

    int numVerts = parentWindow->skin.getNumVerts(UI_SKIN_BTN);
    // LOGOUTPUT << "numverts: " << numVerts; LogInfo();
    //positions
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(gUIManager->GetPositionLocation(),
            2, GL_FLOAT, GL_FALSE, 0, 0);

    //texcoords
    if (down)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 
                parentWindow->skin.getTexcoords(UI_SKIN_BTN_DOWN));
    }
    else if (hovering)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 
                parentWindow->skin.getTexcoords(UI_SKIN_BTN_HOVER));
    }
    else //normal
    {
        glBindBuffer(GL_ARRAY_BUFFER, 
                parentWindow->skin.getTexcoords(UI_SKIN_BTN));
    }
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 
            2, GL_FLOAT, GL_FALSE,0,0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);


    //probably should use a better way to reset hovering status?
    hovering = false;


    /*Vector4 storeColor = color;
      if (hovering)
    {
    //gUIManager->matUIElement.SetUniformData("color", &hoverColor);
    color = hoverColor;
    if (hoverText.length())
    text.output = hoverText;
    }
    else
    {
    gUIManager->matUIElement.SetUniformData("color", &color);
    if (hoverText.length())
    text.output = normalText;
    }
    //reset it back to normal!
    hovering = false;

    gUIManager->matUIElement.SendUniformData();
    UIElement::Draw();
    //yeah ;its a stringstream :[
    if (text.output.length() > 0)
    text.Draw();

    color = storeColor;*/
}

void UIButton::Invalidate()
{
    UIElement::Invalidate();

    //text.parentWindow = parentWindow;
    text.offset = offset + textOffset;
    //text.offset.y += dimensions.y / 3.1; 
    ////TODO 3.1 should be changed to font height or somethin
    //text.offset.x += 1;
    text.output = normalText;
    text.Invalidate();

    //LOGOUTPUT << "ui button invalidate txt position: " << text.getPosition();LogInfo();


    unsigned int size;
    parentWindow->skin.BuildButton(position, dimensions, 
            UI_SKIN_BTN, frameVerts, &size);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //Upload vertex data to the video device
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * size, 
            frameVerts, GL_DYNAMIC_DRAW); 
}

