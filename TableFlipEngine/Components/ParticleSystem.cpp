/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "ParticleSystem.h"

GLuint ParticleSystem::vboParticles = 0;

Vector2 seed[MAX_PARTICLES];
//VBO never gets deleted, add static shutdown if ya care
void ParticleSystem::staticInit()
{
    if (vboParticles == 0)
    {
        glGenBuffers(1, &vboParticles);
        glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
        glEnableVertexAttribArray(0);
        
        //its a dummy buffer, theres no actual attribute data, i byte per element
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        //srand(GetTimecode());
        for (int i = 0; i < MAX_PARTICLES; i++)
            seed[i] = Vector2(0.0001f * (10000.0f - (float)(rand()%20000) ), 0.0001f * (10000.0f - (float)(rand()%20000) )) ; //-1.0 <--> + 1.0

        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Vector2), seed, GL_STATIC_DRAW);
    }
}

ParticleSystem::ParticleSystem(ShaderResource *shader, Entity *e)
{
    if (vboParticles == 0)
        staticInit();
    emitting = false;
    particles = 0;
    numParticles = 0;
    material = 0;
    material = new Material;
    //load the shader!
    if (!material->LoadShader(shader))
    {
        delete material;
        material = 0;
        LogWarning("ParticleSystem() could not load shader.");
        return;
    }
    entity = e;
    material->SetEntity(e);
    material->RegisterWorldViewProjMatrix("WorldViewProjection");
    Vector3 dir(0.0, 0.3, 0.7);
    //TODO add a arbitrary streaming uniform send system, instead of copies
    //material->RegisterUniform("direction", SV_FLOAT3, &dir);
    //material->RegisterUniform("World", SV_ENTITY_TRANSFORM, 0);
   
}

ParticleSystem::~ParticleSystem()
{
    if (material)
        delete material;
}

void ParticleSystem::Init(unsigned int _numParticles)
{
    if (particles != 0)
        return; //already initialized...

    numParticles = _numParticles;
    particles = new Particle[numParticles];
    memset(particles, 0, sizeof(particles));
    numActive = 0;

    glGenBuffers(1, &vboParticleData);
    glBindBuffer(GL_ARRAY_BUFFER, vboParticleData);
    glEnableVertexAttribArray(1);

    //NOTE 2 because 2 floats...
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //NOTE vector 2 because its only 2 floats at the moment
    glBufferData(GL_ARRAY_BUFFER, numParticles * sizeof(Vector2), particles, GL_DYNAMIC_DRAW);
}

void ParticleSystem::Emit(unsigned int numPerSecond)
{
    particlesPerSecond = numPerSecond;
    //assuming 60 FPS
    framesPerParticle = 60.0 / particlesPerSecond;
    emitting = true;
    framespassed = 0;
}

void ParticleSystem::Stop()
{
    emitting = false;
}

void ParticleSystem::Update()
{
    //current time in seconds
    float time = (GetTimecode() * 0.001f);
    unsigned int emit = 0;
    if (framesPerParticle > 1.0) //not really the most accurate, drops the remainder
    {
        if (framespassed >= framesPerParticle)
        {
            emit = 1;
            framespassed = 0;
        }
        else
            emit = 0;
    }
    else
        emit = 1 / framesPerParticle;
    
    unsigned int deactivated = 0;
    for (unsigned int i = 0; i < numActive; i++)
    {
        //may be iffy when count is 2 ?
        if (particles[i].timeDeactivate < time)
        {
            //deactivate
            numActive--;
            Particle tmp = particles[numActive];
            particles[numActive] = particles[i];
            particles[i] = tmp;
        }
    }

    //emit, dont go over numParticles
    if (emitting)
    {
        if (numActive + emit > numParticles)
            emit = numParticles - numActive;

        for (int i = 0; i < emit; i++)
        {
            particles[numActive].timeActivated = time;
            particles[numActive].timeDeactivate = time + 0.01 * ((float)(rand()%9 + 1));
            numActive++;
        }
    }
    
    //send normals
    glBindBuffer(GL_ARRAY_BUFFER, vboParticleData);
    glEnableVertexAttribArray(1); //normal location
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numActive * sizeof(Vector2), particles, GL_DYNAMIC_DRAW);

    framespassed++;
}


void ParticleSystem::PrepareMaterial()
{
    if (material->GetAttributeLocation(ATR_POSITION0) < 0)
        LogWarning("ParticleSystem() cannot prep material");
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vboParticles);
        glEnableVertexAttribArray(0); //vertpos location
        //its just a random float between -1.0 <--> +1.0
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        material->SendUniformData();
    }

   material->getProgram();
    
}

void ParticleSystem::FinishMaterial()
{
    glDisableVertexAttribArray(0);
}

