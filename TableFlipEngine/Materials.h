/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef _MATERIALS_H__
#define _MATERIALS_H__

#include "Utilities.h"
#include "ShaderResource.h"
//#include <GL/glew.h>
#include <vector>
#include <map>
#include "EngineMath.h"
#include "Resources.h"
class Entity;
using std::map;
using std::vector;


//type is GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
GLuint CompileShader(GLenum type, const char *file);

struct UniformLink
{
    int location;
    void *data;
    int dataSize; //size of data block
    ShaderVariables type;
    bool sendOnce;
    bool sent;
    UniformLink() { sent = false; sendOnce = false; dataSize =0; location = -1; data = 0; type = NUM_SVARS; }
};

//this helps the communication of data between a mesh, and a shader.
//you can register uniform variable links to an address in your program
//and have them sync'd every time SendUniformAttributes() is called
//TODO for multiple light sources there will need to be a way to remove and add lights
//as they enter and leave the cameras presence, and possibly other things, maybe a new function
//ResetUniformLink() is needed. and a way to send individual registered uniforms instead of all of them at once.
//maybe some hax are in order for lights specifically, and we bypass uniform links? 
//havent spent too much time designing this really. its due for some thinkering,
class Material
{
private :
    ShaderResource *shader;
    Entity *entity; //if we're attached to an entity, this is he/she/whatevs
public:
    unsigned int id;
   // GLuint vertexShader;
   // GLuint fragmentShader;
    GLuint program;
    
    //View and Projection are special cases, maybe more are needed?  lights??
    int  vMatLocation;
    int  pMatLocation;
    int  wvMatLocation;
    int  wvpMatLocation; //most optimal, compute wvp before it hits the shader
    bool pSent;
    bool vSent;
    bool wvSent;
    bool wvpSent;
    
    vector<UniformLink> uniformLinks;
    vector<void *> dataSlot; //this is needed to save / load data links from blueprint, and to index the pointers
    
    float *transformPointer;

    //right now only automatic type is time
    void SetAutoVars();
    
public:
    //USE COPY CONSTRUCTOR!!
    Material(const Material &mat);
    Material() {pSent = false; vSent = false; entity = 0; shader = 0; id = 0;
                vMatLocation = -1; pMatLocation = -1; wvpMatLocation = -1; wvMatLocation = -1;
                srcBlend = GL_ONE; dstBlend = GL_ZERO;
    }
    ~Material();

    unsigned int srcBlend;
    unsigned int dstBlend;
    
    inline unsigned int GetId() { return id; }
    inline unsigned int GetShaderId() { return shader->GetId(); }
    
    //TODO these load shader functions all sound the same...
    //also. if this fails we should load some default material
    bool LoadShader(const char *filename); 
    bool LoadShader(ShaderResource *shaderResource); 

    //inverse model matrix
    void SetProjectionMatrix(Matrix4 pMat);
    void SetViewMatrix(Matrix4 vMat);
    void SetWorldViewMatrix(Matrix4 vMat);
    void SetWorldViewProjMatrix(Matrix4 wvpMat);
    void SetEntity(Entity *ent) { entity = ent; }
    inline int GetAttributeLocation(AttributeNames attr) { return shader->GetAttributeLocation(attr); }
    inline int GetUniformLocation(string name) { return shader->GetUniformLocation(name); }
    GLuint getProgram() { return program; }
    //updates materials local copy of data
    void SetUniformData(string name, void *data); //TODO this could be slow,  string cmpares and such. just getting a feel for material data
   
    bool RegisterUniform(string name, ShaderVariables type, void *data);
    bool RegisterViewMatrix(string name);
    bool RegisterProjectionMatrix(string name);
    bool RegisterWorldViewMatrix(string name);
    bool RegisterWorldViewProjMatrix(string name);
    void SendUniformData();
    
    //this is a HACK around improper shared materials, so sprites dont need a million different materials
    //just for individual transforms
    inline float *getTransformPtr() { return transformPointer; }
    inline void setTransformPtr(float *t) { transformPointer = t; }
    //binds attributes -- resets uniform link sent states 
    void BindAttributes(); //TODO rename this function please, attributes dont need to be bound...

    const char *GetShaderName() { return shader->GetFileName(); }
    
};

#endif
