/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _LOGIC_H_
#define _LOGIC_H_

#include "Utilities.h"
#include <string>
#include <vector>


class Logic  : public Engine::Component
{
private:
    
    void Update(){}
   //bool scripted;
   
protected:
    
    bool activeUpdate;  //set this to true if you need updates every frame!

    //activates the render callback, make sure you disarm in logic destroy!
    bool ArmRenderCallback(); //true if added ok
    void DisarmRenderCallback();
    
public:
    
     Logic();
    std::string scriptName;
    inline bool isScripted() { return scriptName.length(); }
    ~Logic();

    inline bool needsUpdate() { return activeUpdate; }
    
    virtual void Init(){}
    //must set active update to true for this to be called!!!!
    virtual void Update(float dt){}
    virtual void Destroy(){}
    virtual void OnCollisionEnter(Entity *other) { /*Scripting::GetInstance()->CallOnCollisionEnter(scriptObject, other);*/ }
    virtual void OnCollisionStay(Entity *other)  { /*Scripting::GetInstance()->CallOnCollisionStay(scriptObject, other);*/ }
    virtual void OnCollisionLeave(Entity *other) { /*Scripting::GetInstance()->CallOnCollisionLeave(scriptObject, other); */}
    virtual void RenderCallback() {} //right now this gets called at the end of the render
    
};

//states for the FSM
class LogicState
{
public:
    std::string name;
    virtual void Enter() = 0;
    virtual void Update(float dt){}
    virtual void Exit() = 0;
};

//logic class with a finite state machine
//init should populate the states
class LogicFSM : public Logic
{
protected:
    std::vector<LogicState *> states;
    LogicState *currentState;
public:
    ~LogicFSM() { for (int i=0; i < states.size(); i++) delete states[i]; }
    virtual void Init()=0; //populates the state vector
    void Update(float dt) { currentState->Update(dt); }
    void ChangeState(unsigned int stateIdx);
    void ChangeState(LogicState *nextState);
    
    LogicState &getCurrentState() { return *currentState; }

};

#endif
