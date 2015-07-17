/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Logic.h"
#include "ObjectManager.h"
#include "Renderer.h"
Logic::Logic ( )
{
    this->activeUpdate = false;
    //if scripted logic, load the angel script file
   // script == 0 ? scripted = false : scripted = true; activeUpdate = needUpdate;
   // scriptName = script;
    //Init gets called after attached to entity
}


Logic::~Logic()
{
    if (needsUpdate())
        ObjectManager::GetInstance()->Remove(this);


}

bool Logic::ArmRenderCallback()
{
    Renderer::GetInstance()->AddRenderCallback(this);
}

void Logic::DisarmRenderCallback()
{
    Renderer::GetInstance()->RemoveRenderCallback(this);
}




void LogicFSM::ChangeState(unsigned int stateIdx)
{
    if (stateIdx >= states.size())
    {
        LogWarning("LogicFSM::ChangeState() - index out of range");
        return;
    }
    if (currentState == states[stateIdx])
        return;
    currentState->Exit();
    currentState = states[stateIdx];
    currentState->Enter();
}

void LogicFSM::ChangeState(LogicState* nextState)
{
    if (currentState == nextState)
        return;
    
    for (int i = 0; i < states.size(); i++)
    {
        if (states[i] == nextState)
        {
            ChangeState(i);
            return;
        }
    }
    LogWarning("LogicFSM::ChangeState() - state not found!");
}
