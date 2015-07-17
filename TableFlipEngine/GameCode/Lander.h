/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __LANDER_HPP__
#define __LANDER_HPP__
#include "Input.h"
#include <Logic.h>
#include <Entity.h>

class Lander;
class Instrumentation
{
private:
    BlueprintResource *bpAttMask;
    BlueprintResource *bpAttGyro;
    BlueprintResource *bpAttCrosshair;
    Entity *attitude;
    Entity *crosshair;
    Entity *mask;
    float attitudeYOffset;

    Lander *lander;

public:
    Instrumentation() { lander = 0; attitude = crosshair = mask = 0; }
    void Init(Lander *_lander);
    void Update();
    void Destroy();
};

class Lander : public Logic
{
public:
    Instrumentation instrumentation;
   
    void Init();
    void Update(float dt);
    void Destroy();
};



#endif
