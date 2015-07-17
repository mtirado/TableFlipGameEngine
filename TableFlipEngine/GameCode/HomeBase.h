/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _HOME_BASE_H_
#define _HOME_BASE_H_

#include "WebGLGame.h"
#include "BoundingVolumes.h"
class Tower : public Logic
{
private:
public:
    void Init() { entity->setType(ETYPE_TOWER); }
};

class Agency : public Logic
{
public:
    void Init();
    void Update();
    void SpawnLocal();
private:
};
//normal is for usul selections / movement, placement is - obvious, command is for unit commands
enum InputMode { INPUT_MODE_EXPAND = 0, INPUT_MODE_PLACEMENT };


class HomeBase : public LogicFSM
{
private:
    class HomeNormal : public LogicState
    {   public:
        HomeBase *base;
        HomeNormal(HomeBase *b);
        void Enter();
        void Update(float dt);
        void Exit();
    };
    class HomePlacement : public LogicState
    {   public:
        HomeBase *base;
        HomePlacement(HomeBase *b);
        void Enter();
        void Update(float dt);
        void Exit();
    };
    class HomeExpand : public LogicState
    {   public:
        HomeBase *base;
        HomeExpand(HomeBase *b);
        void Enter();
        void Update(float dt);
        void Exit();
    };

    double money;
    double gains;
    unsigned int moneyGainDelay;
    unsigned int lastGain;

    InputMode inputMode;
    double towerCost;
    double agencyCost;
    double localAgentCost;
    vector<Tower *> towers;
    Entity *towerPlaceholder;
    Entity *agencyPlaceholder;

    Entity *placeholder;

    EntityType placementType;

public:
    ~HomeBase();
    void Init();
    void Update(float dt);
    void CheckInput();

    Entity *selection;

    bool SpawnLocalAgent();
    
    bool WithdrawMoney(double amt)
    {
        if (money < amt)
            return false;
        money -= amt;
        return true;
        
    }
    
    void setPlacementType(EntityType type) { placementType = type; }
    double getTowerCost() { return towerCost; }
    InputMode getMode() { return inputMode; }
    
};

#endif
