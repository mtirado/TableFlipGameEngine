/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _ENEMY_1A_H
#define _ENEMY_1A_H
#include "Agent.hpp"
#include <Physics.h>

class Enemy1A : public Agent
{
private:
    
public:
    void Init()
    {
        maxHealth = 10.0f;
        speed = 1.0;
        attack = 1.0;
        defense = 1.0;
        health = maxHealth;
    }

    void Update(float dt)
    {
        Physics::GetInstance()->dynamicsWorld->;
    }
    
};

#endif
