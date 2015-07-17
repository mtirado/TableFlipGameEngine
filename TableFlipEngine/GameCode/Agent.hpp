/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _AGENT_H
#define _AGENT_H
#include <Logic.h>

//really, just an interface
class Agent : public Logic
{
protected:
    float maxHealth;
    float health;
    float attack;
    float defense;
    float speed;
public:
    
};

#endif
