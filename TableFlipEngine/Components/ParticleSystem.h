/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__

#include "Utilities.h"
#include <Materials.h>
#include <BulletMultiThreaded/SpuNarrowPhaseCollisionTask/Box.h>

//NOTES
//set size on buffer once?
#define MAX_PARTICLES 102400

struct Particle
{
    float timeActivated;
    float timeDeactivate;
};

class ParticleSystem : public Engine::Component
{
protected:

    static GLuint vboParticles;

    GLuint vboParticleData;
    unsigned int numParticles;
    unsigned int numActive;
    unsigned int particlesPerSecond;
    float framesPerParticle; // will be positive if pps is > 60
    unsigned int framespassed; //how many frames passed since last emission
    bool emitting;
    Particle *particles;
    ParticleSystem(){}
public:

    //its public, but dont screw around with it..
    Material *material; //this is the actual shader thats doing all the work!
    
    inline unsigned int getShaderId()   { return material->GetShaderId(); }
    inline unsigned int getNumActive()  { return numActive; }
    inline bool hasMaterial()           { return material ? true : false; }
    virtual void PrepareMaterial();
    virtual void FinishMaterial();
    static void staticInit();

    virtual void Init(unsigned int _numParticles);
    virtual void Update();

    //assumes 60fps.
    virtual void Emit(unsigned int numPerSecond);
    virtual void Stop();
    
    
    ParticleSystem(ShaderResource *shader, Entity *e);
    ~ParticleSystem();

    

};

#endif
