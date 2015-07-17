/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#include "Entity.h"
#include "Materials.h"
#include "Renderer.h"
#include "Utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include "ShaderResource.h"

Material::Material(const Material &mat)
{
    shader = mat.shader;
    shader->AddRef();
    program = shader->GetProgram();
    id = shader->GetNewMatId();
    entity = 0;
    transformPointer = 0;
    vMatLocation = mat.vMatLocation;
    pMatLocation = mat.pMatLocation;
    srcBlend = mat.srcBlend;
    dstBlend = mat.dstBlend;
    SetAutoVars();
    //copy links  NOTE not possible at this point in time, design needs expansion
   // for (int i = 0; i < mat.uniformLinks.size(); i++)
    //{
     //   RegisterUniform(mat.uniformLinks[i].name, );
    //}
    
    
}

Material::~Material()
{
    //remove shader reference
    if (shader)
        shader->RemoveRef();
    
    //remove any texture references we may be holding
    for (int i = 0; i < uniformLinks.size(); i++)
    {
        
        if (uniformLinks[i].type == SV_SAMPLER2D && uniformLinks[i].data != 0)
        {
            ((TextureResource *)uniformLinks[i].data)->RemoveRef();
        }
        else if (uniformLinks[i].data != 0 && uniformLinks[i].type != SV_ENTITY_TRANSFORM)
        {
            if (uniformLinks[i].type == SV_FLOAT || uniformLinks[i].type == SV_INT || uniformLinks[i].type == SV_SAMPLER2D)
                delete ((int *)uniformLinks[i].data); //single int
            else //is an array
                delete[] ((int *)(uniformLinks[i].data));
        }
        else
        {
          //  LogError("No data in uniform link, transform perhaps??");
        }
    }
}

GLchar *readFile(const char *file, GLint *len)
{
    FILE *fp;
    char *content = NULL;
 
    int count=0;
 
    if (file != NULL) 
    {
        fp = fopen(file,"rt");
 
        if (fp != NULL) 
        {
 
            fseek(fp, 0, SEEK_END);
            count = (int)ftell(fp);
            rewind(fp);
 
            if (count > 0) 
            {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = (int)fread(content,sizeof(char),count,fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    *len = count+1;
    return content;
}

void Material::SetAutoVars()
{
    //always attempts to register time
    RegisterUniform("SV_TIME_SECONDS", SV_TIME_SECONDS, 0);
}

void Material::BindAttributes()
{
    //shader->BindAttributes(); //didnt need this..
    //resets uniform sent state as well
    for (int i = 0; i < uniformLinks.size(); i++)
        uniformLinks[i].sent = false;
    pSent = false;
    vSent = false;
    wvpSent = false;
}

bool Material::LoadShader(const char *filename) 
{
    shader = LoadShaderResource(filename);
    
    if (shader == 0) 
        return false; 

    shader->AddRef();
    id = shader->GetNewMatId();
    program = shader->GetProgram();

    SetAutoVars();
    
    return true; 
     
}

bool Material::LoadShader(ShaderResource* shaderResource)
{
    shader = shaderResource;
    shader->AddRef(); //skipping resource load, so manually add reference
    
    if (shader == 0) 
        return false; 

    id = shader->GetNewMatId();
    program = shader->GetProgram();

    SetAutoVars();
    
    return true;
}
   
    
static void show_info_log(GLuint object)
{
    GLint log_length;
    char *log;

    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = (char *)malloc(log_length);
    glGetShaderInfoLog(object, log_length, NULL, log);
    //sprintf(stderr, "%s", log);
    LogInfo(log);
    free(log);
}


//goes through the established data links, and sends data to the gpu
void Material::SendUniformData()
{    
    for (int i = 0; i < uniformLinks.size(); i++)
    {       
        //go through our registered uniforms, and update the shader uniform data pointers
        //to use this new uniform, links are created at material initialization, and MUST be valid.
        ShaderVar *svar = shader->uniformTable[uniformLinks[i].location];
        
        //already sent, do not update
        if (uniformLinks[i].sendOnce && uniformLinks[i].sent)
            continue;
         
        //TODO verify variable types?? error checking stuff
        //update pointer, and mark as invalidated
        if (uniformLinks[i].type == SV_ENTITY_TRANSFORM && uniformLinks[i].data == 0)
            svar->data = transformPointer;
        else
            svar->data = uniformLinks[i].data;
        
        //TODO probably remove validation i think i sidestepped the need for this.
        svar->invalidated = true;   //NOTE this is a bug,  projection matrix has 0 data, causes a crash if it sends
        uniformLinks[i].sent = true;
    }
    
    //updated our variables, now tell the shader to send opengl the data
    shader->SendUniforms();   
   
}

void *AllocateUniformData(ShaderVariables type, int *allocSize)
{
        
    if (type == SV_FLOAT || type == SV_INT /*|| type == SV_SAMPLER2D*/)
    {
        *allocSize = sizeof(int);
        return new int;
    }
    else if (type == SV_FLOAT2 || type == SV_INT2)
    {
        *allocSize = 2 * sizeof(int);
        return new int[2];
    }
    else if (type == SV_FLOAT3 || type == SV_INT3)
    {
        *allocSize = 3 * sizeof(int);
        return new int[3];
    }
    else if (type == SV_FLOAT4 || type == SV_INT4)
    {
        *allocSize = 4 * sizeof(int);
        return new int[4];
    }
    else if (type == SV_MAT2)
    {
        *allocSize = 2 * 2 * sizeof(int);
        return new int[2 * 2];
    }
    else if (type == SV_MAT3)
    {
        *allocSize = 3 * 3 * sizeof(int);
        return new int[3 * 3];
    }
    else if (type == SV_MAT4)
    {
        *allocSize = 4 * 4 * sizeof(int);
        return new int[4 * 4];
    }
    
    //null type returns nada
    return (void *)0;
    *allocSize = 0;
        
}

//TODO add one that just takes location, and avoid string lookup
void Material::SetUniformData(string name, void* data)
{
    int loc = shader->GetUniformLocation(name);
    for(int i = 0; i < uniformLinks.size(); i++)
    {
        if (uniformLinks[i].location == loc)
        {
            if (uniformLinks[i].dataSize)
            {
                //yes, assuming 4 byte minimum type size
                memcpy(uniformLinks[i].data, data, uniformLinks[i].dataSize);
                uniformLinks[i].sent = true; //if sharing material we need to reset the sent state
                return;
            }
        }
    }
}


bool Material::RegisterUniform ( string name, ShaderVariables type, void* data )
{
   
   
    UniformLink newLink;
    int allocSize;
 
    //verify this uniform exists in shader land
    newLink.location = shader->GetUniformLocation(name);
    
    if (newLink.location < 0 || (data == 0 && (type != SV_ENTITY_TRANSFORM && type != SV_TIME_SECONDS) )) //entity transform type has nul ldata
    {
        LOGOUTPUT << "Could Not Register Uniform Data Link: " << name;
        LogError();
        return false;
    }
    
    //TODO validate type and print error if not valid
    newLink.type = type;
    newLink.data = AllocateUniformData(type, &allocSize);
    newLink.dataSize = allocSize;
    
   
    //TODO   obvious crash potential here,  safety checks or something? i derno
    if (type != SV_SAMPLER2D && type != SV_ENTITY_TRANSFORM && type != SV_TIME_SECONDS) //samplers just point to a texture resource
        memcpy(newLink.data, data, allocSize);
    else if (type == SV_SAMPLER2D)
    {
        //broken optimization... sort by textures and whatnot please..
        //newLink.sendOnce = true;

        newLink.data = data;
        ((TextureResource *) data)->AddRef();
    }
    else if (type == SV_ENTITY_TRANSFORM) //most objects will have this, no use copying data all over the place.
    {
        if ((size_t)data == 1)
        { //HACK around my lack of shared materials, this tells us to use transform ptr, if its valid of course
            newLink.data = 0;
        }
        else if (entity)
        {
            newLink.data = entity->GetTransform()->transform;
        }
        else
        {
            LogError("null entity in material while trying to set shader transform matrix, did you set the materials entity?");
            return false;
        }
    }
    else if (type == SV_TIME_SECONDS)
    {
        //will return the first of this type!
        //if (shader->GetUniformLocation("SV_TIME_SECONDS") < 0)
          //  return false;
    }
    else
    {
        LogError("something bad has happened, unknown shader var????");
        return false;
    }
    uniformLinks.push_back(newLink);
    
    return true;
 
}


    
bool Material::RegisterViewMatrix(string name)
{
    vMatLocation = shader->GetUniformLocation(name);
    if (vMatLocation < 0)
    {
       // LogError("Could not register view matrix, check name");
        return false;
    }
    else
        return true;

}
bool Material::RegisterProjectionMatrix(string name) 
{ 
    pMatLocation = shader->GetUniformLocation(name); 
    
    if (pMatLocation < 0)
    {
        LogError("Could not register projection matrix, check name");
        return false;         
    }
    else 
        return true; 
}

bool Material::RegisterWorldViewMatrix(string name)
{
    wvMatLocation = shader->GetUniformLocation(name);
    if (wvMatLocation < 0)
    {
       // LogError("Could not register view matrix, check name");
        return false;
    }
    else
        return true;

}

bool Material::RegisterWorldViewProjMatrix(string name)
{
    wvpMatLocation = shader->GetUniformLocation(name);

    if (wvpMatLocation < 0)
    {
        LogError("Could not register world view projection matrix, check name");
        return false;
    }
    else
        return true; 
}


void Material::SetProjectionMatrix(Matrix4 pMat) 
{  
    //if (pSent)
    //    return;
    if (pMatLocation < 0)
    {
        LogError("Material Projection Matrix is NOT registered and you are trying to use it." );
    }
    else
        glUniformMatrix4fv(pMatLocation, 1, false, pMat); 
    
  //  pSent = true;
    
}
void Material::SetViewMatrix(Matrix4 vMat) 
{ 
    //if (vSent)
    //    return;
    if (vMatLocation < 0)
    {
        LogError("Material View Matrix is NOT registered and you are trying to use it." );
    }
    else
        glUniformMatrix4fv(vMatLocation, 1, false, vMat); 
    
    //vSent = true;
    
}    

void Material::SetWorldViewMatrix(Matrix4 wvMat)
{
   // if (wvSent)
   //     return;
    if (vMatLocation < 0)
    {
        LogError("Material View Matrix is NOT registered and you are trying to use it." );
    }
    else
        glUniformMatrix4fv(wvMatLocation, 1, false, wvMat);

    //wvSent = true;

}    

void Material::SetWorldViewProjMatrix(Matrix4 wvpMat)
{
    //if (wvpSent)
    //    return;
    if (wvpMatLocation < 0)
    {
        LogError("Material WorldViewProj Matrix is NOT registered and you are trying to use it." );
    }
    else
        glUniformMatrix4fv(wvpMatLocation, 1, false, wvpMat);

   // vSent = true;
}




