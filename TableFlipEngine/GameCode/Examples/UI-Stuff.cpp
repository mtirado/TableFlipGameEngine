/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Game.h"
#include "Utilities.h"
#include "Renderer.h"
#include "UserInterface.h"
#include "UIButton.h"
#include "ObjectManager.h"
#include "Builder.h"
#include "SoundSystem.h"
#include "UIButton.h"
#include "UIWindow.h"
#include "UISlider.h"
#include "UIView.h"
#include "UIListBox.h"
#include "UITextField.h"
#include "UITextView.h"

Camera *mainCamera;
UITextField *gDbgTxtField = 0;
//level load callback
void OnLoad()
{

}

void GameInit()
{
    mainCamera = new Camera;
    mainCamera->Init();
    mainCamera->GetTransform()->position = Vector3(0.0, 0.0, 0.0);
    Renderer::GetInstance()->camera = mainCamera;
    InitUI();

}

void GameUpdate()
{
    UpdateUI();
    mainCamera->moveForce = 0.016120f;
    mainCamera->linearDrag = 0.85f;
    mainCamera->FlyLookControl();
}


void GameShutdown()
{
    ShutdownUI();
    //mainCamera->DestroyAndRelease();
}


void InitUI()
{
    UIWindow * win = gUIManager->CreateWindow(0, Vector2(50.0,50.0), 
                     Vector2(500.0,500.0) ); 

    win->CreateFrame();  
    win->CreateScrollbars(false, true);

    UIButton *btn = new UIButton();
    btn->SetText(L"Do Not Press!");
    btn->SetOffset(Vector2(150.0, 300.0));
    btn->SetDimensions(Vector2(150,50));
    btn->SetFontSize(20.0);
        
   /* UITextArea *txtArea = new UITextArea();
    txtArea->SetOffset(Vector2(200.0,200.0));
    txtArea->SetDimensions(Vector2(80.80)); 
    txtArea->getString().append(L"TESTINGG\nand whatnot\nyahurrd??");
*/



    UIListBox *view = new UIListBox();
    view->SetOffset(Vector2(20.0, 20.0));
    view->SetDimensions(Vector2(100, 200));
    win->AddElement(view);
    //FIXME create frame and scrollbars must happen after added to window
    view->CreateFrame();
    view->CreateScrollbars(false, true);
        
    UIListBox *list = new UIListBox();
    list->SetOffset(Vector2(20.0, 230.0));
    list->SetDimensions(Vector2(100, 200));
    win->AddElement(list);
    list->CreateFrame();
    list->CreateScrollbars(false, true);
    
    for (int i = 0; i < 18; i++)
    {
        view->AddToList(L"that Text", 0);
        list->AddToList(L"though...", 0);
    }




    UITextView *txtView = new UITextView();
    txtView->SetOffset(Vector2(130, 370));
    txtView->SetDimensions(Vector2(90,120));
    win->AddElement(txtView);
    txtView->CreateFrame();
    txtView->CreateScrollbars(false, true);
    //txtView->getString().clear();
    //txtView->getString().append(L"tesssssssst???");
    txtView->Invalidate();
    
    //win->AddElement(txtArea);
    win->AddElement(btn);    

    //add a buncha sliders
    for (int i = 0; i < 24; i++)
    {
        UISlider *slide = new UISlider(0, 100);
        slide->SetDimensions(Vector2(20, 80));  
        slide->SetOffset(Vector2(160 + i * 12, 40));
        win->AddElement(slide);
    }

    //test text field
    UITextField *txtIn = new UITextField;
    txtIn->SetOffset(Vector2(140, 170));
    txtIn->SetDimensions(Vector2(140, 1)); 
    txtIn->SetFontSize(16);
    //txtIn->allowInput = false;
    txtIn->getString().clear();
    txtIn->getString().append(L"t");

    win->AddElement(txtIn);

    gDbgTxtField = txtIn;

}

void UpdateUI()
{

}

void ShutdownUI()
{
}

