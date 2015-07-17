/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "HomeBase.h"
#include "Builder.h"
#include <unistd.h>

BlueprintResource *bpTowerA;
BlueprintResource *bpAgency;
BlueprintResource *bpLocalAgent;

void Agency::Init() 
{
    entity->setType(ETYPE_AGENCY);
}

void Agency::Update()
{
}




void HomeBase::Init()
{
    entity->setType(ETYPE_HOMEBASE);
    activeUpdate = true;
    money = 0.0;
    gains = 10.093; //per second
    lastGain = GetTimecode();
    moneyGainDelay = 3500;

    towerCost = 7.0;
    agencyCost = 14.0;
    localAgentCost = 1.0;
    
    bpTowerA = LoadBlueprint("Data/Blueprints/towerA.bp");
    towerPlaceholder = Builder::GetInstance()->ConstructEntity(bpTowerA, Vector3(0,-100000,0));
    towerPlaceholder->setType(ETYPE_GENERIC);
    towerPlaceholder->GetMesh()->hidden = true;

    bpAgency = LoadBlueprint("Data/Blueprints/agency.bp");
    agencyPlaceholder = Builder::GetInstance()->ConstructEntity(bpAgency, Vector3(0,-100000,0));
    agencyPlaceholder->setType(ETYPE_GENERIC);
    agencyPlaceholder->GetMesh()->hidden = true;

    bpLocalAgent = LoadBlueprint("Data/Blueprints/testThing.bp");

    selection = this->entity;
    //fill out state machine  NOTE these are in the order of the enum
    LogicState *state = new HomeExpand(this);
    states.push_back(state);
    state->Enter();
    currentState = state;
    state = new HomePlacement(this);
    states.push_back(state);
    
    
}

void HomeBase::Update(float dt)
{
    if (Input::GetInstance()->GetKeyDownThisFrame(KB_ESC))
    {
        ChangeState(INPUT_MODE_EXPAND);
        selection = this->entity;
        Renderer::GetInstance()->GetCamera()->SetOrbit(this->entity);
    }
    
    money += gains * dt;
    SetMoney(money);
    lastGain = GetTimecode();
    currentState->Update(dt);
    
}

bool HomeBase::SpawnLocalAgent()
{
    if (selection->getType() != ETYPE_AGENCY)
        return false;
    
    if (WithdrawMoney(localAgentCost))
    {
        Vector3 p = selection->GetTransform()->position;
        p.x += 50;
        p.y += 25;
        Builder::GetInstance()->ConstructEntity(bpLocalAgent, p);
    }
    else
        return false;
}


//////////////////////////////
// STATES
/////////////////////////

//normal
HomeBase::HomeNormal::HomeNormal(HomeBase* b)
{
    base = b;
}

void HomeBase::HomeNormal::Enter()
{
    base->inputMode = INPUT_MODE_EXPAND;
}
void HomeBase::HomeNormal::Update(float dt)
{

}
void HomeBase::HomeNormal::Exit()
{

}

// command
HomeBase::HomeExpand::HomeExpand(HomeBase* b)
{
    base = b;
}
void HomeBase::HomeExpand::Enter()
{
    base->inputMode = INPUT_MODE_EXPAND;
}
void HomeBase::HomeExpand::Update(float dt)
{
    //check for selections
    if(Input::GetInstance()->GetMouseBtn1JustPressed())
    {
        usleep(1); //ugh... weird bug thing
        Vector2 coords(Input::GetInstance()->GetXCoords(0), Input::GetInstance()->GetYCoords(0));
        RayHitInfo hit = Physics::GetInstance()->ScreenRaycast(coords);
        if (hit.entity)
        {
            if (hit.entity->getType() != ETYPE_DEFAULT ) //default being some world geom
            {
                Renderer::GetInstance()->GetCamera()->SetOrbit(hit.entity);
                base->selection = hit.entity;
                base->ChangeState(INPUT_MODE_EXPAND);
            }
           // else if (hit.entity->getType() == ETYPE_HOMEBASE)
            //{
            //    base->ChangeState(INPUT_MODE_EXPAND);
            //    Renderer::GetInstance()->GetCamera()->SetOrbit(base->entity);
           // }
        }
    }
}
void HomeBase::HomeExpand::Exit()
{

}

//placement
HomeBase::HomePlacement::HomePlacement(HomeBase* b)
{
    base = b;
}

void HomeBase::HomePlacement::Enter()
{
    base->inputMode = INPUT_MODE_PLACEMENT;
    //unhide the placeholder mesh
    if (base->placementType == ETYPE_TOWER)
        base->placeholder = base->towerPlaceholder;
    else if (base->placementType == ETYPE_AGENCY)
        base->placeholder = base->agencyPlaceholder;
    base->placeholder->GetMesh()->hidden = false;

}
void HomeBase::HomePlacement::Update(float dt)
{
    //FIXME i have no idea what the fuck is happening here.
    //without this sleep the raycast in bullet will magically break somehow, and never work again
    usleep(1);
    Vector2 coords(Input::GetInstance()->GetXCoords(0), Input::GetInstance()->GetYCoords(0));
    RayHitInfo hit = Physics::GetInstance()->ScreenRaycast(coords);
    base->placeholder->GetTransform()->SetPosition(hit.worldPoint);
    if(Input::GetInstance()->GetMouseBtn1JustPressed())
    {
        //valid placement!
        if (hit.entity)
        {
            if (base->money >= base->towerCost)
            {
                Vector3 pos;
                if (hit.entity->getType() == ETYPE_TOWER)
                {
                    pos = hit.entity->GetTransform()->position;
                    //NOTE assumes NO NEGATIVES IN Y COMPONENT
                    //will work though if min y is negative HACK not good
                    pos.y += abs(hit.entity->GetMesh()->GetMaxY()) + abs(hit.entity->GetMesh()->GetMinY());
                }
                else
                    pos = hit.worldPoint;

                if (base->placementType == ETYPE_TOWER)
                {
                    base->money -= base->towerCost;
                    Entity *e = Builder::GetInstance()->ConstructEntity(bpTowerA, pos);
                    e->setType(ETYPE_TOWER);
                }
                else if (base->placementType == ETYPE_AGENCY)
                {
                    base->money -= base->towerCost;
                    Entity *e = Builder::GetInstance()->ConstructEntity(bpAgency, pos);
                    e->setType(ETYPE_AGENCY);
                }
                //Renderer::GetInstance()->GetCamera()->SetOrbit(e);
                base->ChangeState(INPUT_MODE_EXPAND);
            }            
        }
    }
}
void HomeBase::HomePlacement::Exit()
{
    base->placeholder->GetMesh()->hidden = true;
}

