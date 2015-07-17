/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef SHADER_RESOURCE_H
#define SHADER_RESOURCE_H

#include "Utilities.h"

#include <vector>
#include <map>
#include <string>
using std::map;
using std::string;
using std::vector;
enum ShaderVariables {  SV_FLOAT = 0, SV_FLOAT2, SV_FLOAT3, SV_FLOAT4, 
                        SV_INT, SV_INT2, SV_INT3, SV_INT4, 
                        SV_MAT2, SV_MAT3, SV_MAT4, SV_ENTITY_TRANSFORM,  //entity transform will be a pointer to the transform
                        SV_SAMPLER2D, SV_SAMPLER_2DSHADOW, SV_SAMPLER_CUBE,
                        SV_TIME_SECONDS, NUM_SVARS };

//barycentric will only work with GL_TRIANGLES,  send <1,0,0> <0,1,0> <0,0,1> for vert 1, 2, 3
// when interpolated in fragment shader that becomes the exact barycentric position of the fragment
enum AttributeNames  { ATR_POSITION0= 0, ATR_NORMAL0, ATR_BARYCENTRIC, ATR_TEXCOORD0, ATR_TEXCOORD1,
                       ATR_POSITION1,    ATR_NORMAL1, NUM_ATRS };

struct ShaderVar
{
    ShaderVariables type;    //one of the above types
    AttributeNames attribType; //what attribute is this, also index into attribute array..
    string name;        //name of variable in shader
    int location;       //location in shader program
    void *data;         //pointer to data, some variables will not change like VP matrices, light positions, etc.
    bool invalidated;   //data has changed, resend to shader
    int  textureStage;
    ShaderVar() {       data = 0; invalidated = false; 
                    location = -1; name = ""; type = NUM_SVARS; textureStage = 0; }
};

class ShaderResource : public Resource
{
private:
	GLuint computeShader;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    unsigned int id;
    
    //TODO a REAL f'kin id, not this hacked mess
    unsigned int mats;
    
    ~ShaderResource();
    ShaderResource();
    
    //TODO switch to protected & garbage collected
public:
    //DO NOT EVER CALL THIS - will implement a lazy garbage collector
    inline void Delete() { delete this; }
    
public:
    
    ShaderResource(const char *filename);
   
    ShaderVar *attributes[NUM_ATRS];
    vector<ShaderVar> uniforms;
    //map uniforms by location in GLSL
    map <int, ShaderVar *> uniformTable;
    
    inline GLuint GetProgram()          { return program;       }
    inline unsigned int GetId()         { return id;            }
    inline void SetId(unsigned int _id) { id = _id;             }
    inline unsigned int GetNewMatId()   { return (id * 10000) + mats++; } //TODO current limits to 10,000 mats possible till whole system breaks
    
    bool LoadShaderFile(const char *shaderFile, ResourceLoadMemInfo *mem);
    bool CompileProgram(char *vertFile, char *fragFile);
    bool CompileComputeProgram(char *computeFile);
    
    int GetAttributeLocation(AttributeNames attr);
    int GetUniformLocation(string name);
    
    //TODO may crash if location is invalid?
    string GetUniformName(int location) { return uniformTable[location]->name; }
    
    //binds all attribute locations
    void BindAttributes();
    //sends any uniform data that is marked as invalidated
    void SendUniforms();
    
    
    
};

//resource loader
ShaderResource* LoadShaderFile( const char* filename, int _priority = 0, ResourceLoadMemInfo *mem = 0);

#endif
