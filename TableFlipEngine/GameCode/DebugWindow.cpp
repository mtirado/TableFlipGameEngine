/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UserInterface.h"
#include <string>
#include <sstream>
using std::string;
using std::endl;
using std::ifstream;
int btnClix = 0;

UITextOut *targetLabel;

void DebugWindowInit()
{
    UIWindow *dbgWindow = new UIWindow();
    dbgWindow->color[0] = 0.33f;
    dbgWindow->color[1] = 0.33f;
    dbgWindow->color[2] = 0.33f;
    UIManager::GetInstance()->AddWindow(dbgWindow);

    dbgWindow->MoveWindow(Vector2(50,50));
    dbgWindow->dimensions = Vector2(20,20);
    
      //create a title bar
    UITitleBar *titleBar = new UITitleBar;
    titleBar->SetColor(1,0,1);
   // titleBar->color[0] = 1; titleBar->color[1] = 0; titleBar->color[3] = 1;
    titleBar->offset = Vector2(0,dbgWindow->dimensions.y - 2.5f);
    titleBar->dimensions.x = dbgWindow->dimensions.x;
    titleBar->dimensions.y = 2.5f;
    titleBar->Invalidate();
    dbgWindow->AddElement(titleBar);
    
    
    UITextOut *objLabel = new UITextOut();
    objLabel->output.str() = "selection";
    objLabel->offset = Vector2(27, 90);
    objLabel->Invalidate();
    dbgWindow->AddElement(objLabel);
    
    targetLabel = new UITextOut();
   
    targetLabel->output.str() = "ButtonClicks: ";
    targetLabel->offset = Vector2(2, 75);
    targetLabel->Invalidate();
    dbgWindow->AddElement(targetLabel);
    dbgWindow->MoveWindow(Vector2(50,50)); //yeah move window invalidates,  DOY
    
}

void DebugWindowUpdate()
{

    //http://www.antongerdelan.net/opengl/raycasting.html
   /* Vector4 rayClip, rayEye, rayWorld;
    rayClip[0] = (2.0 * Input::GetInstance()->GetXCoords(0)) / Renderer::GetInstance()->width - 1.0f;
    rayClip[1] = 1.0f - (2.0f * Input::GetInstance()->GetYCoords(0)) / Renderer::GetInstance()->height;
    rayClip[2] = -1.0; //forward in opengl
    rayClip[3] = 1.0;
    Matrix4 inv;
    MatrixInvert(inv,Renderer::GetInstance()->GetCamera()->GetProjection());
    Matrix4Multiply(rayEye, inv, rayClip);
    rayEye[2] = -1.0f;
    rayEye[3] = 0.0f;
    Matrix4MultiplyVector4(rayWorld, Renderer::GetInstance()->GetCamera()->GetInverseView(), rayEye);
    Vector3 mouseRay = Vector3(rayWorld[0], rayWorld[1], rayWorld[2]);
    mouseRay.Normalize();
    
    Vector3 start = Renderer::GetInstance()->GetCamera()->GetTransform()->position;
    Vector3 end = start + mouseRay * 1000;
    
 
    Entity *hit = Physics::GetInstance()->Raycast(start, end);
    if (hit && Input::GetInstance()->GetMouseBtn2Down())
    {
        std::ostringstream windowText;
        btnClix++;
        windowText.str("");
        windowText.clear();
        windowText << "target: " << hit->name.c_str() << "\n";
       // windowText << "< " << hit->name.c_str();
        
        targetLabel->output.str() = windowText.str();
        Renderer::GetInstance()->GetCamera()->GetTransform()->position.y += 0.001;
        
    }
    else
        targetLabel->output.str() = "no target";*/
}

void DebugWindowShutdown()
{
}
