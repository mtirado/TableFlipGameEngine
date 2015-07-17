/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Utilities.h"
#include "ShaderResource.h"
#include "Resources.h"
#include "androidEX.h"
#include <stdio.h>
#include <stdlib.h>
using std::pair;
//custom shader file format - keep things nice and automated for the game code...

/*
 * PASSES: 1
 * 
 * p1
 * {
 *      ATTRIBUTES: 3
 *      float4 position ATR_POSITION0
 *      float4 normal ATR_NORMAL0
 *      float4 texCoord ATR_TEXCOORD0
 *      UNIFORMS: 3
 *      mat4 view
 *      mat4 world
 *      float4 color
 *      
 *      vp (OR COMPUTE_SHADER**)
 *      { (for now.. ) first line of code must be 'precision highp float;'
 *      [vertex program src]
 *      }
 *      fp
 *      { (for now.. ) first line of code must be 'precision highp float;'
 *      [fragment program src]
 *      }
 * }
 * 
 */

//TODO MAKE A REAL ID GENERATOR, not this noobish junk
static unsigned int idCount = 1;

ShaderResource::ShaderResource(const char *filename)
{
    id = 0;
    mats = 0;
    vertexShader = 0;
    fragmentShader =0;
    program = 0;
    for (int i = 0; i < NUM_ATRS; i++)
        attributes[i] = 0;
    FileName = filename;
    computeShader = 0;
}

ShaderResource::~ShaderResource()
{
    for (int i = 0; i < NUM_ATRS; i++)
        if (attributes[i])
            delete attributes[i];
        
   // for (int i = 0; i < uniforms.size(); i++)
    //    delete uniforms[i];

    if (computeShader)
    {
        glDetachShader(program, computeShader);
        glDeleteProgram(program);
        glDeleteShader(computeShader);
        LogInfo("Compute Shader deleted...");
    }
    else
    {
        
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        LogInfo("Shader deleted...");
    }

    if (references > 0)
        LogError("REMOVING SHADER WITH ACTIVE REFERENCES");
}


static char typeBuff[64];
static char nameBuff[64];
static char attribBuff[64];
static char buff[512]; //reads a line,
static char buff2[512];//heh!

//reads a shader variable, attribute or uniform
bool readVar(FILE *file, ShaderVar *sv, bool isAttrib)
{
    if (!fgets(buff, sizeof(buff), file))
    return false;
    
    if (isAttrib)
    {
        if (sscanf(buff, "%s %s %s;", typeBuff, nameBuff, attribBuff) != 3)
        {
            LogError("Error reading shader variable, check syntax.");
            return false;
        }
    }
    else
    {
        if (sscanf(buff, "%s %s;", typeBuff, nameBuff) != 2)
        {
            LogError("Error reading shader variable, check syntax.");
            return false;
        }
    }
    ShaderVariables type = NUM_SVARS;
    //check type
    if (!strcmp(typeBuff, "float")) { type = SV_FLOAT; }
    else if (!strcmp(typeBuff, "vec2")) { type = SV_FLOAT2; }
    else if (!strcmp(typeBuff, "vec3")) { type = SV_FLOAT3; }
    else if (!strcmp(typeBuff, "vec4")) { type = SV_FLOAT4; }
    else if (!strcmp(typeBuff, "sampler2D")) { type = SV_SAMPLER2D; }
    else if (!strcmp(typeBuff, "sampler2DShadow")) { type = SV_SAMPLER_2DSHADOW; }
    else if (!strcmp(typeBuff, "samplerCube")) { type = SV_SAMPLER_CUBE; }
    else if (!strcmp(typeBuff, "mat2")) { type = SV_MAT2; }
    else if (!strcmp(typeBuff, "mat3")) { type = SV_MAT3; }
    else if (!strcmp(typeBuff, "mat4")) { type = SV_MAT4; }
   /* else if (!strcmp(typeBuff, "bool")) { type = SV_BOOL; }
    else if (!strcmp(typeBuff, "bool2")) { type = SV_BOOL2; }
    else if (!strcmp(typeBuff, "bool3")) { type = SV_BOOL3; }
    else if (!strcmp(typeBuff, "bool4")) { type = SV_BOOL4; }*/
    else if (!strcmp(typeBuff, "int")) { type = SV_INT; }
    else if (!strcmp(typeBuff, "int2")) { type = SV_INT2; }
    else if (!strcmp(typeBuff, "int3")) { type = SV_INT3; }
    else if (!strcmp(typeBuff, "int4")) { type = SV_INT4; }
    else if (!strcmp(typeBuff, "float_time")) { type = SV_TIME_SECONDS; }
    else
    {
        LogError("Unkown shader variable type");
        return false;
    }
    
    AttributeNames attrType = NUM_ATRS;
    if (isAttrib)
    {
        //check Attribute type
        if      (!strcmp(attribBuff, "POSITION0"))      { attrType = ATR_POSITION0;     }
        else if (!strcmp(attribBuff, "NORMAL0"))        { attrType = ATR_NORMAL0;       }
        else if (!strcmp(attribBuff, "TEXCOORD0"))      { attrType = ATR_TEXCOORD0;     }
        else if (!strcmp(attribBuff, "TEXCOORD1"))      { attrType = ATR_TEXCOORD1;     }
        else if (!strcmp(attribBuff, "BARYCENTRIC"))    { attrType = ATR_BARYCENTRIC;   }
        else if (!strcmp(attribBuff, "POSITION1"))      { attrType = ATR_POSITION1;     }
        else if (!strcmp(attribBuff, "NORMAL1"))        { attrType = ATR_NORMAL1;       }
        else
        {
            LogError("Unkown Attribute Type");
            return false;
        }
    }
    
    //remove semicolon if it exists
    size_t len = strlen(nameBuff);
    if (nameBuff[len -1] == ';')
        nameBuff[len -1] = 0;
    
    
    sv->type = type;
    sv->attribType = attrType;
    sv->name = nameBuff;
    sv->location = -1; //invalid untill we link program
    return true;
}

//uhhh
#define SHADER_MAX_SIZE 100000
//does this mean i'm assuming non-unicode files?? not 100% sure.
static char vpBuffer[SHADER_MAX_SIZE];
static char fpBuffer[SHADER_MAX_SIZE];


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

//type is GL_FRAGMENT_SHADER GL_VERTEX_SHADER GL_COMPUTE_SHADER
//file is a GLSL shader loaded into memory
GLuint CompileShader(GLenum type, char *file)
{
    GLchar *source = 0;// = readFile(file, &length);
    GLuint shader;
    GLint shader_ok;

        
    if (!file)
    {
        LogError("NO SHADER SOURCE!!!");
        exit(-87);
        return -1;
    }
    shader = glCreateShader(type);
    GLint fsize = (unsigned int)strlen(file);
        
    source = file;

    //using null terminated option does NOT work in llvm / osx?
    glShaderSource(shader, 1, (const GLchar**)&source, &fsize);
   // glShaderSource(shader, 1, (const GLchar**)&source, NULL); //NULL assumes each string is terminated
   
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
        
    if (!shader_ok)
    {
        LogError("Failed to Compile Shader!");
        //LogError(file);
        //fprintf(stderr, "Failed to compile %s:\n", source);
        show_info_log(shader);
        glDeleteShader(shader);
        exit(-88);
        return -1;
    }
    return shader;

}

bool ShaderResource::CompileComputeProgram(char* computeFile)
{
    computeShader = CompileShader(GL_COMPUTE_SHADER, computeFile);
    if (computeShader == 0)
    {
        LogError("could not compile compute program...");
        exit(-88);
        return false;
    }

    GLint program_ok;
    program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
    {
        LogError("Failed to link shader program:");
        char buffer[4096]; //sorry
        glGetProgramInfoLog(program, 4096, NULL, buffer);
        LOGOUTPUT << buffer << "\n";
        LogError();
        glDeleteProgram(program);
        exit(-88);
        return false;
    }


    //this whole function is just copy/pasta from below :[
    int numUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    LOGOUTPUT << "Linked compute program with " << numUniforms << " active uniforms";
    LogInfo();


    //try to link all the attributes
    //TODO if theres a way to verify variable types, do that too.
    for (int i = 0; i < NUM_ATRS; i++)
    {
        if (attributes[i] == 0)
            continue;

        int loc = glGetAttribLocation(program, attributes[i]->name.c_str());
        if (loc < 0)
        {
            LOGOUTPUT << "could not find attribute in compute shader: " << attributes[i]->name;
            LogError();
        }
        attributes[i]->location = loc;
    }

    for (int i = 0; i < uniforms.size(); i++)
    {
        int loc = glGetUniformLocation(program, uniforms[i].name.c_str());
        if (loc < 0)
        {
            LOGOUTPUT << "could not find uniform in compute shader: " << uniforms[i].name;
            LogError();
        }
        uniforms[i].location = loc;
        uniformTable.insert(pair<int, ShaderVar *>(loc, &uniforms[i]));
    }
    return true;
}



//probably going to need to be modified slightly for geometry shaders
bool ShaderResource::CompileProgram(char* vertFile, char* fragFile)
{
    GLint program_ok;
    
    //compile both parts
    vertexShader = CompileShader(GL_VERTEX_SHADER, vertFile);
    if (vertexShader == 0)
    {
        LogError("No Vertex Shader!?");
        return false;
    }
    fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragFile);
    if (fragmentShader == 0)
    {
        LogError("No Fragment Shader!?");
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);     
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
    {
        LogError("Failed to link shader program:");
        
        //show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        exit(-89);
        return false;
    }

    int numUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    LOGOUTPUT << "Linked program with " << numUniforms << " active uniforms";
    LogInfo();
    
        
    //try to link all the attributes
    //TODO if theres a way to verify variable types, do that too.
    for (int i = 0; i < NUM_ATRS; i++)
    {
        if (attributes[i] == 0)
            continue;
       
        int loc = glGetAttribLocation(program, attributes[i]->name.c_str());
        if (loc < 0)
        {
            LOGOUTPUT << "could not find attribute in shader: " << attributes[i]->name;
            LogError();
        }
        attributes[i]->location = loc;
    }    
   
    for (int i = 0; i < uniforms.size(); i++)
    {
        int loc = glGetUniformLocation(program, uniforms[i].name.c_str());
        if (loc < 0)
        {
            LOGOUTPUT << "could not find uniform in shader: " << uniforms[i].name;
            LogError();
        }
        uniforms[i].location = loc;
        uniformTable.insert(pair<int, ShaderVar *>(loc, &uniforms[i]));
    }    
    return true;
}



//TODO: implement multiple passes, only writing single pass support for now.
ShaderResource* LoadShaderFile( const char* filename, int _priority, ResourceLoadMemInfo *mem )
{
       
    //parse our custom joint,
#if defined(__ANDROID__)
    if (!mem) return 0;
    FILE *file = fmemopen(mem->mem, mem->size, "r");
#else
    FILE *file = fopen(filename, "r");
#endif
    if (!file)
    {
        LOGOUTPUT << "Error Opening Shader File: " << filename;
        LogError();
    }
    
    
    int curPass = 0;
    int numPasses = 0;
    int numAttrs = 0;
    int numUniforms = 0;
    char c;
    
    //read number of passes (first line of file)
    if (!fgets(buff, sizeof(buff), file))
        return 0;
    
    if (sscanf(buff, "PASSES: %d", &numPasses) != 1)
    {
        LOGOUTPUT << "Error reading number of passes: " << filename;
        LogError();
        return 0;
    }
    
    //seek to the first pass {
    do 
    {
        c = fgetc(file);
    }
    while(c != '{' && c != EOF);
    //clear the line, next line we are in bracket
    if (!fgets(buff, sizeof(buff), file))
        return 0;
    curPass = 1;
    
    //read how many attributes we have.
    if (!fgets(buff, sizeof(buff), file))
        return 0;
    //the white space trickery may not work on other platforms....
    if (sscanf(buff, " ATTRIBUTES: %d", &numAttrs) != 1)
    {
        LOGOUTPUT << "Error Reading Number of Attributes: " << filename;
        LogError();
        return 0;
    }
    
    if (numAttrs >= NUM_ATRS)
    {
        LogError("Attribute count is too large.");
        return 0;
    }
    
    
    ShaderResource *newShader = new ShaderResource(filename);
    //read attribs
    for (int i = 0; i < numAttrs; i++)
    {
        ShaderVar *svar = new ShaderVar;
        if (!readVar(file, svar, true))
        {
            LOGOUTPUT << "Error Reading Attribute, loop index: " << i << "  - in file: " << filename;
            LogError();
            delete svar;
            newShader->Delete();
            return 0;
        }    
        else
            newShader->attributes[svar->attribType] = svar;     
    }
    
    //how many uniforms...
    if (!fgets(buff, sizeof(buff), file))
    { newShader->Delete(); return 0; }
    
    if (sscanf(buff, " UNIFORMS: %d", &numUniforms) != 1)
    {
        LOGOUTPUT << "Error Reading Number of Uniforms: " << filename;
        LogError();
        newShader->Delete();
        return 0;
    }
        
        
    //read uniforms
    for (int i = 0; i < numUniforms; i++)
    {
        ShaderVar svar;
        if (!readVar(file, &svar, false))
        {
            LOGOUTPUT << "Error Reading Uniform, loop index: " << i << "  - in file: " << filename;
            LogError();
            newShader->Delete();
            return 0;
        }
        else
        newShader->uniforms.push_back(svar);     
    }

    //store position, bout to rewind
    long int pos = ftell(file);

    //scan the file for a COMPUTE_SHADER block
    fseek(file, 0, 0);
    bool loop = true;
    bool computeShader = false;
    do
    {
        if (fgets(buff, sizeof(buff), file))
        {
            memset(buff2, 0, sizeof(buff2));
            sscanf(buff, " %s", buff2);
            if (strcmp(buff2, "COMPUTE_SHADER") == 0)
            {
                //LOGOUTPUT << "\n\n-----\nCOMPUTE_CHECK PASSED\n----\n\n";
                //LogInfo();
                loop = false;
                computeShader = true;
            }
        }
        else
        {
            //LOGOUTPUT << "\n\n-----\nCOMPUTE_CHECK FAILED\n----\n\n";
           // LogInfo();
            loop = false;
        }
            
    } while (loop);

    //restore position
    fseek(file, pos, 0);
    
    //find vertex(or compute) program
    do { c = fgetc(file); }
    while(c != '{' && c != EOF);

    //clear the line, next line we are in bracket
    if (!fgets(buff, sizeof(buff), file))
    { newShader->Delete(); return 0; }
    
    //locations in file
    int bracketDepth = 0;
    int buffIdx = 0;
    int vpCount = 0;
    bool reading = true;
    //we're in, so keep reading untill we hit a '}' that is unacounted for.
    do 
    { 
        c = fgetc(file);
        
        if (c == EOF)
        {
            LOGOUTPUT << "Error Reading Vertex Program: " << filename; 
            LogError();
            newShader->Delete();
            return 0;
        }
        
        if (c == '{')
            bracketDepth++;
        
        
        if (c == '}' && bracketDepth == 0)
        {
            c = 0; //change it to a null terminator
            vpCount = buffIdx;
            reading = false;
        }
        else if (c == '}')
        {
            bracketDepth--;
        }
        vpBuffer[buffIdx] = c;
        buffIdx++;
        
        if (buffIdx >= SHADER_MAX_SIZE)
        {
            LOGOUTPUT << "Vertex Program larger than SHADER_MAX_SIZE: " << SHADER_MAX_SIZE << "  - in file: " << filename;
            LogError();
            newShader->Delete();
            return 0;
        }
    }
    while(reading);

    if (!computeShader)
    {
        //read fragment program
        //seek to the next{
        do { c = fgetc(file); }
        while(c != '{' && c != EOF);
        //clear the line, next line we are in bracket
        if (!fgets(buff, sizeof(buff), file))
        { newShader->Delete(); return 0; }


        bracketDepth = 0;
        reading = true;
        buffIdx = 0;
        int fpCount = 0;
        //we're in, so keep reading untill we hit a '}' that is unacounted for.
        do
        {
            if (buffIdx >= SHADER_MAX_SIZE)
            {
                LOGOUTPUT << "Vertex Program larger than SHADER_MAX_SIZE: " << SHADER_MAX_SIZE << "  - in file: " << filename;
                LogError();
                newShader->Delete();
                return 0;
            }

            c = fgetc(file);

            if (c == EOF)
            {
                LOGOUTPUT << "Error Reading Vertex Program: " << filename;
                LogError();
                newShader->Delete();
                return 0;
            }

            if (c == '{')
                bracketDepth++;


            if (c == '}' && bracketDepth == 0)
            {
                c = 0; //change it to a null terminator
                fpCount = buffIdx;
                reading = false;
            }
            else if (c == '}')
            {
                bracketDepth--;
            }

            fpBuffer[buffIdx] = c;
            buffIdx++;

        }
        while(reading);
   
        //all the info we need is in memory, so now compile shaders,
        //link the program, and retrieve variable locations. if any fail, the whole thing is a bust.
        if (!newShader->CompileProgram(vpBuffer, fpBuffer))
        {
            LOGOUTPUT << "Could Not Compile Shader Program." << filename;
            LogError();
            newShader->Delete();
            return 0;
        }
    }
    else //compute shader
    {
        if (!newShader->CompileComputeProgram(vpBuffer))
        {
            LOGOUTPUT << "Could Not Compile Compute Shader." << filename;
            LogError();
            newShader->Delete();
            return 0;
        }
    }

    fclose(file);
    newShader->SetId(idCount++);
    
    return newShader;
}

int ShaderResource::GetAttributeLocation(AttributeNames attr)
{
    if (attributes[attr] == 0)
    {
        //TODO better error reporting, print attr name
        //TODO hrm, this gets fired a heck of a lot if no texcoords or normals used...
        //TODO add verbose debug logging and add this to it
        //LogError("ERROR! Shader Attribute not found!");
        //LOGOUTPUT << "Shader Attribute not found ATR IDX: " << attr;
        //LogWarning();
        return -1;
    }
    return attributes[attr]->location;
}

int ShaderResource::GetUniformLocation(string name)
{
    for (int i = 0; i < uniforms.size(); i++)
    {
        if (name.compare(uniforms[i].name) == 0)
            return uniforms[i].location;
    }

    //just incase someone decides they dont want to used
    // the variable name 'time', check type for SV_TIME_SECONDS
    if (name == "SV_TIME_SECONDS")
    {
        for (int i = 0; i < uniforms.size(); i++)
        {
            if (uniforms[i].type == SV_TIME_SECONDS)
                return uniforms[i].location;
        } 
    }
    
   // LOGOUTPUT << "Could not locate Uniform: " << name;
   // LogError();
    return -1;
}

void ShaderResource::BindAttributes()
{
    for(int i = 0; i < NUM_ATRS; i++)
    {
        if (attributes[i])
        {
            int location = attributes[i]->location;
            if (location != -1)
            {
                glBindAttribLocation(program, location, attributes[i]->name.c_str());
            }
        }
    }
}

    
void ShaderResource::SendUniforms()
{
    for (int i = 0; i < uniforms.size(); i++)
    {
        //FIXME this was a mess...
        //if (uniforms[i].invalidated) //only send if invalidated...
        {
            if (!uniforms[i].data && uniforms[i].type != SV_TIME_SECONDS)
            {// this check is screwed up now that i commented out the above validation
               // LOGOUTPUT << "Uniform not registered : " << uniforms[i].name;
               // LogError();
                
            }
            else
            {
           
                switch (uniforms[i].type)
                {
                    
                    case SV_FLOAT:
                    { 
                        //maybe not using the 1fv varsion is quicker?
                        glUniform1fv(uniforms[i].location, 1, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_FLOAT2:
                    {    
                        glUniform2fv(uniforms[i].location, 1, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_FLOAT3:
                    {    
                        glUniform3fv(uniforms[i].location, 1, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_FLOAT4:
                    {    
                        glUniform4fv(uniforms[i].location, 1, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_MAT2:
                    {                                           //no transpose
                        glUniformMatrix2fv(uniforms[i].location, 1, false, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_MAT3:
                    {    
                        glUniformMatrix3fv(uniforms[i].location, 1, false, (GLfloat *)uniforms[i].data);
                    }
                    break;
                    case SV_MAT4:
                    {    
                        glUniformMatrix4fv(uniforms[i].location, 1, false, (GLfloat *)uniforms[i].data);
                    }
                    case SV_ENTITY_TRANSFORM:
                    {    
                        glUniformMatrix4fv(uniforms[i].location, 1, false, (GLfloat *)uniforms[i].data);
                    }    
                    break;
                    case SV_INT:
                    {    
                        glUniform1iv(uniforms[i].location, 1, (GLint *)uniforms[i].data);
                    }
                    break;
                    case SV_INT2:
                    {    
                        glUniform2iv(uniforms[i].location, 1, (GLint *)uniforms[i].data);
                    }
                    break;
                    case SV_INT3:
                    {    
                        glUniform3iv(uniforms[i].location, 1, (GLint *)uniforms[i].data);
                    }
                    break;
                    case SV_INT4:
                    {    
                        glUniform4iv(uniforms[i].location, 1, (GLint *)uniforms[i].data);
                    }
                    break;
                    case SV_TIME_SECONDS:
                    {
                        float time = (GetTimecode() * 0.001f);
                        glUniform1fv(uniforms[i].location, 1, (GLfloat *)&time);
                    }
                    break;
                    case SV_SAMPLER2D: //the data is a pointer to TextureResource
                    {
                        if ((*(GLuint *)uniforms[i].data) == 0) 
                        {
                            LogError("Trying to bind null texture resource");
                            break;
                        }
                      
                        if (uniforms[i].textureStage == 0)        
                            glActiveTexture(GL_TEXTURE0);    //how many of these should be supported???
                        else if (uniforms[i].textureStage == 1)        
                            glActiveTexture(GL_TEXTURE1);
                        else if (uniforms[i].textureStage == 2)        
                            glActiveTexture(GL_TEXTURE2);
                        else if (uniforms[i].textureStage == 3)        
                            glActiveTexture(GL_TEXTURE3);
                        else if (uniforms[i].textureStage == 4)        
                            glActiveTexture(GL_TEXTURE4);
                        else if (uniforms[i].textureStage == 5)        
                            glActiveTexture(GL_TEXTURE5);
                        else if (uniforms[i].textureStage == 6)        
                            glActiveTexture(GL_TEXTURE6);
                        else if (uniforms[i].textureStage == 7)        
                            glActiveTexture(GL_TEXTURE7);
                        else
                        {
                            LogWarning("Invalid Texture stage, setting it to 0... ");
                            uniforms[i].textureStage = 0;
                            glActiveTexture(GL_TEXTURE0);
                        }

                        GLuint tID = ((TextureResource *)uniforms[i].data)->GetTexId();
                        glBindTexture(GL_TEXTURE_2D, ((TextureResource *)uniforms[i].data)->GetTexId());
                        glUniform1i(uniforms[i].location, uniforms[i].textureStage); //default stage is 0
                        break;
                        
                    }
                    
                    default:
                        LogError("Unkown Shader Variable Type");
                        break;
                }
            }
            uniforms[i].invalidated = false;
        }
    }
}


    
    
    
