/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UITitleBar.h"

UITitleBar::UITitleBar()
{
    color = Vector4(0.4, 0.6, 0.1,  1.0f);
}


void UITitleBar::AddToWindow(UIWindow* wnd)
{
    UIElement::AddToWindow(wnd);
    text.AddToWindow(wnd);
}

void UITitleBar::SetOffset(Vector2 pos)
{
    UIElement::SetOffset(pos);
    //3.0 should be something involving width of texxt
    text.SetOffset(  Vector2(offset.x + dimensions.x / 3.0, offset.y)  );
    Invalidate();
}

void UITitleBar::Invalidate()
{
    UIElement::Invalidate();
    text.Invalidate();
}

void UITitleBar::MouseDown(Vector2 pt)
{
    clickedOffset.x = parentWindow->GetPosition().x - pt.x;
    clickedOffset.y = parentWindow->GetPosition().y - pt.y;
}


void UITitleBar::FocusFunc()
{
    Vector2 newPos;
    newPos.x = Input::GetInstance()->GetXCoords(cursorId);
    newPos.y = Input::GetInstance()->GetYCoords(cursorId);

    newPos.x = clickedOffset.x + (newPos.x * 1000) / Renderer::GetInstance()->width;
    newPos.y = clickedOffset.y + ( (Renderer::GetInstance()->height - newPos.y ) * 1000) / Renderer::GetInstance()->height;

    parentWindow->MoveWindow(newPos);
}



void UITitleBar::Draw()
{
       UIElement::Draw();
}
