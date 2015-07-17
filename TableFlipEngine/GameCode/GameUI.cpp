/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "WebGLGame.h"
#include "UserInterface.h"
#include "UITextOut.h"
#include "UIButton.h"
#include "UITextField.h"
#include "UITextArea.h"
#include "SoundSystem.h"
#include "MiscLogic.hpp"
#include "HomeBase.h"
float xPos = 15;
float xWidth = 70;
float shrinkTo = xWidth / 2;
int towers = 0;
HomeBase *playerBase;

UIWindow *wndChat;
//UITextArea *;
UIWindow *wndChatInput;
//UITextIn    *txtChatInput;
UITextOut *fpsCounter;

//build window
UIWindow  *wndMain;
UITextOut *txtMoney;
UIButton  *btnBuildMiner;
UIButton  *btnBuildAgency;

UIButton  *btnBuildLocalAgent;

UIWindow    *wndInspector;
UITextField *txtEdit;
UITextField *txtInput;
UITextArea  *txtArea;

void ReturnAction(UIElement *elem, int id)
{
    LogInfo("RETURN!!!!!!!!");
    txtArea->getString().append(txtInput->getString());
    txtInput->getString().clear();
}

void InitUI(Entity *base)
{
    fpsCounter = new UITextOut(L"Testing.", true);
    fpsCounter->SetFontSize(54);
    fpsCounter->SetOffset(Vector2(0.0f, 960.0f));
    gUIManager->AddToRootWindow(fpsCounter);

    wndInspector = gUIManager->CreateWindow(L"-=inspector=-",
                                            Vector2(200.0, 200.0),
                                            Vector2(200.0f, 400.0f),
                                            Vector4(0.3330, 0.3330, 0.3330, 1.0) );

    //below title bar
    float top = wndInspector->dimensions.y - 25.0;
    //test internel textedit functionality
    UITextField *txtTest = new UITextField;
    txtTest->getString().fill(L"test");
    txtTest->SetOffset(Vector2(50.0, top - 30.0 ));
    txtTest->SetDimensions(Vector2(70.0, 24.0));
    txtTest->frameThickness = 0.0081;
    txtTest->SetColor(Vector4(0.19, 0.19, 0.19, 1.0));
    txtTest->SetTextOffset(Vector2(5.0, 7));
    wndInspector->AddElement(txtTest);

    txtInput = new UITextField;
    txtInput->getString().fill(L"123...");
    txtInput->SetOffset(Vector2(10.0, top - 60.0 ));
    txtInput->SetDimensions(Vector2(180.0, 24.0));
    txtInput->frameThickness = 0.0081;
    txtInput->SetColor(Vector4(0.19, 0.19, 0.19, 1.0));
    txtInput->SetTextOffset(Vector2(5.0, 7));
    txtInput->SetActionCallback(&ReturnAction);
    wndInspector->AddElement(txtInput);
    
    //counter
    txtEdit = new UITextField;
    txtEdit->allowInput = false;
    txtEdit->SetOffset(Vector2(50.0, top - 90.0));
    txtEdit->SetDimensions(Vector2(70.0, 24.0));
    txtEdit->frameThickness = 0.0081;
    txtEdit->SetFontSize(14.0);
    txtEdit->SetFontWeight(1.2);
    txtEdit->SetColor(Vector4(0.19, 0.19, 0.19, 1.0));
    txtEdit->SetTextOffset(Vector2(5.0, 7));
    wndInspector->AddElement(txtEdit);

    txtArea = new UITextArea;
    txtArea->getString().fill(L"test - ");
    txtArea->SetOffset(Vector2(20.0, 10));
    txtArea->SetDimensions(Vector2(160.0, 180.0));
    txtArea->frameThickness = 0.0081;
    txtArea->SetFontSize(14.0);
    txtArea->SetFontWeight(1.5);
    txtArea->SetColor(Vector4(0.19, 0.19, 0.19, 1.0));
    txtArea->SetTextOffset(Vector2(2.0, 9.0));
    wndInspector->AddElement(txtArea);
    }

void UpdateState()
{
    if (playerBase->getMode() == INPUT_MODE_PLACEMENT)
    {
        //btnBuildMiner->Hide();
    }
    else if (playerBase->getMode() == INPUT_MODE_EXPAND)
    {
        switch (playerBase->selection->getType())
        {
            case ETYPE_HOMEBASE:
                btnBuildMiner->Show();
                btnBuildAgency->Show();
                btnBuildLocalAgent->Hide();
            break;
            case ETYPE_TOWER:
                btnBuildMiner->Show();
                btnBuildAgency->Show();
                btnBuildLocalAgent->Hide();
            break;
            case ETYPE_AGENCY:
                btnBuildMiner->Hide();
                btnBuildAgency->Hide();
                btnBuildLocalAgent->Show();
            break;
            
            default:
            break;
        }
    }
    
}
#include "TString.h"

void UpdateUI()
{

    fpsCounter->output.clear();
    fpsCounter->output.append(L"FPS: ");
    fpsCounter->output.append(GetFPS());
    const wchar_t * wst = fpsCounter->output.c_str();
    static unsigned int num = 0;
    txtEdit->getString().clear();
    txtEdit->getString().append(++num/6);


   // LOGOUTPUT << "wchar_t size: " << sizeof(wchar_t);
    //LogError();
    /*if (Input::GetInstance()->GetKeyDownThisFrame(KB_SPACE))
        SoundSystem::GetInstance()->PlayTest();*/

//     //text input behavior
//     if (txtChatInput->inFocus())
//     {
//         if (Input::GetInstance()->GetKeyDownThisFrame(KB_RETURN))
//         {
//             txtChat->SetText(txtChat->output.str() + txtChatInput->output.str() + "\n");
//             UIManager::GetInstance()->DropFocus(txtChatInput);
//             wndChatInput->Hide();
//             Input::GetInstance()->textGrabMode = false;
//         }
//     }
//     else if (Input::GetInstance()->GetKeyDownThisFrame(KB_RETURN))
//     {
//         wndChatInput->Show();
//         UIManager::GetInstance()->GrabFocus(txtChatInput);
//         Input::GetInstance()->textGrabMode = true;
//     }
//     else
//         wndChatInput->Hide(); //so it hides when focus is dropped by another mechanism
// 
// 
//     //check button presses
//     if (btnBuildMiner->Pressed() || Input::GetInstance()->GetKeyDownThisFrame(KB_M))
//     {
//         playerBase->setPlacementType(ETYPE_TOWER);
//         playerBase->ChangeState(INPUT_MODE_PLACEMENT);
//         towers++;
//     }
//     if (btnBuildAgency->Pressed())
//     {
//         playerBase->setPlacementType(ETYPE_AGENCY);
//         playerBase->ChangeState(INPUT_MODE_PLACEMENT);
//     }
//     //if (btnBuildLocalAgent->Pressed())
//     //    playerBase->SpawnLocalAgent();
//     //
}

void ShutdownUI()
{
    
}

void SetMoney(double _money)
{
   /* txtMoney->output.str(L"");
    txtMoney->output.clear();
    txtMoney->output.precision(2);
    txtMoney->output << std::fixed << _money << L"$";
    txtMoney->Invalidate();*/
}
