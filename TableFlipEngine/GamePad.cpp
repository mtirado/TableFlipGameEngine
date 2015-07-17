/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#include "GamePad.h"
#include "UIWindow.h"

void Joystick::MouseDown(Vector2 pt)
{
   // cout << "mouse Down?\n";
    //clickedOffset.x = parentWindow->GetPosition().x - pt.x;
   // clickedOffset.y = parentWindow->GetPosition().y - pt.y;
}


void Joystick::FocusFunc()
{
        if (cursorId < 0)
        {
                LogError("CURSOR ID IS INVALID!!!!!!");
                return;
        }

      
    Vector2 newPos;
    newPos.x = Input::GetInstance()->GetXCoords(cursorId);
    newPos.y = Input::GetInstance()->GetYCoords(cursorId);
   
    newPos.x = clickedOffset.x + (newPos.x * 100) / Renderer::GetInstance()->width;
    newPos.y = clickedOffset.y + ( (Renderer::GetInstance()->height - newPos.y ) * 100) / Renderer::GetInstance()->height;
    //parentWindow->MoveWindow(newPos);
    
    //
}


void Joystick::Invalidate()
{       
    position = parentWindow->GetPosition() + offset;
    float data[6][2] =  {
                           {  position.x, position.y },
                           {  position.x + dimensions.x, position.y },
                           {  position.x + dimensions.x, position.y + dimensions.y },
                           {  position.x + dimensions.x, position.y + dimensions.y },
                           {  position.x, position.y + dimensions.y },
                           {  position.x, position.y }
                           };

    UIElement::Draw();
//        //Make the new VBO active
//        glBindBuffer(GL_ARRAY_BUFFER, vbo);
//        glEnableVertexAttribArray(0); //vertpos location
//        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0,         0);
//        //Upload vertex data to the video device
//        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, data, GL_DYNAMIC_DRAW);
}

void Joystick::Draw()
{
    UIElement::Draw();
//     UIManager::GetInstance()->SetDrawColor(0.0f, 0.0f, 0.93f, 1.0f);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     glEnableVertexAttribArray(0); //vertpos location
//                                                 //stride   //offset start
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0,         0);
//     glDrawArrays(GL_TRIANGLES, 0, 6);
}

