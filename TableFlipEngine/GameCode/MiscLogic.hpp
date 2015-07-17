/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _MISC_LOGIC_H
#define _MISC_LOGIC_H
#include <Components.h>
#include "WebGLGame.h"
#include "ObjectManager.h"
#include "Worker.hpp"
#include "Builder.h"
//141 * 1024  = 145,000//
//325 = 333,000
//256
#define NUM_WORKGROUPS 640
#define WORKGROUP_SIZE 1024 //may be too hgih for some cards?
#define MAX_INDICES_IN_CELL 64
#define WACKY_NUMBER NUM_WORKGROUPS * WORKGROUP_SIZE
//BVH to hack around with
class IndexGrid
{
public:
    //data will be [num_elements] [e1] [e2] ... [en] [num_elements] [etc..]
    unsigned int data[MAX_INDICES_IN_CELL * NUM_WORKGROUPS * WORKGROUP_SIZE];

    void Prepare()
    {
        //set all initial counts to 0
        unsigned int c = 0;
        for (unsigned int i = 0; i < WACKY_NUMBER; i++)
        {
            data[i * c] = 0;
        }
    }
};

class GPUTest : public Logic
{
private:
    GLuint dataBuff;
    GLuint gridBuff;
    GLuint positionsBuff; //sends positions back for cpu to put in grid
    Vector3 *positions;   //mapped data pointer
    GLuint renderVAO;
    GLint  locVertPos;
    GLint  locVertNormal;
    GLint  locPosition;
    GLint  locColor;
    ModelResource *cube;
    IndexGrid grid;
protected:

    ShaderResource *computeShader;
    ShaderResource *buildGridShader;
    ShaderResource *drawShader;

    struct particle
    {
        //.w is mass
        Vector4 position;
        Vector4 color;
        Vector4 velocity;
    };
    unsigned int numParticles;
    unsigned int indicesPerGridCell;

public:
    //BlueprintResource *newBox;
    void Init() 
    {
        numParticles=NUM_WORKGROUPS * WORKGROUP_SIZE;//1048576;
        indicesPerGridCell = MAX_INDICES_IN_CELL;
        //load compute shader
        computeShader   = LoadShaderResource("Data/Shaders/Compute/Particulate.shader");
        drawShader      = LoadShaderResource("Data/Shaders/DrawParticulate.shader");
        buildGridShader = LoadShaderResource("Data/Shaders/Compute/BuildGrid.shader");
        cube = new ModelResource(0);
        cube->LoadCubeTriStrip();

        //tell renderer to use callback
        ArmRenderCallback();
        activeUpdate = true;

        //allocate any buffers and such
        
        //locVertPos    = drawShader->GetAttributeLocation(ATR_POSITION0);
       // locVertNormal = drawShader->GetAttributeLocation(ATR_NORMAL0);
        locPosition   = drawShader->GetAttributeLocation(ATR_POSITION0);
        locColor      = drawShader->GetAttributeLocation(ATR_NORMAL0);
        
        //glDisableVertexAttribArray(locPosition);
        //glDisableVertexAttribArray(locColor);
        
        glGenBuffers(1, &dataBuff);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuff);
        glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(particle), (void *)0,
                     GL_DYNAMIC_COPY);

       /* glGenBuffers(1, &gridBuff);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuff);
        glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * 4 * indicesPerGridCell, (void *)0,
                     GL_DYNAMIC_COPY);

        glGenBuffers(1, &positionsBuff);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsBuff);
        glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * 12, (void *)0,
                     GL_DYNAMIC_COPY);*/

        //TODO check out ARB_vertex_attrib_binding
        //may solve the VAO problem where the attrib pointer never actually gets set?
        //VAO's should be more efficient.
        
        //setup dat array buffer
        //glGenVertexArrays(1, &renderVAO);
       // glBindVertexArray(renderVAO);
       // glBindBuffer(GL_ARRAY_BUFFER, dataBuff);

       // glVertexAttribPointer(locPosition, 4, GL_FLOAT, GL_FALSE, sizeof(particle), (void *)0);
       // glVertexAttribPointer(locColor, 4, GL_FLOAT, GL_FALSE, sizeof(particle), (void *)sizeof(Vector4));
       // glVertexAttribDivisor(0, 1);
        //glVertexAttribDivisor(1, 1);
       // glEnableVertexAttribArray(locPosition);
       // glEnableVertexAttribArray(locColor);

//         CheckGLErrors();
//         LogInfo("binding");
        Reset();

//         CheckGLErrors();
//         LogInfo("\n------ BUFFERS INITTED -----------");
        

    }

    void Reset()
    {
         glBindBuffer(GL_ARRAY_BUFFER, dataBuff);
        CheckGLErrors();
//         LogInfo("map range");
        particle *initBuff = reinterpret_cast<particle *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, numParticles * sizeof(particle),
                                                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));


        int length  = WORKGROUP_SIZE;
        int count   = 0;

        for (int i = 0; i < numParticles; i++)
        {
            //i = x + length * (y + length * z);
            initBuff[i].position = Vector4( (15000 - rand()%15000) * 0.001,
                                            20.0 + ( rand()% 4000) * 0.001,
                                            (15000 - rand()%15000) * 0.001,
                                            1.0); //mass

            initBuff[i].color = Vector4(0.96, 0.07, 0.12, 1.0);/*Vector4((float)(rand()%2) * 1.001,
                                        (float)(rand()%2) * 1.001,
                                        (float)(rand()%2) * 1.001,
                                        0.4);*/


            Vector3 vel = Vector3(  0.85 * (1.0 - (rand()%2000 * 0.001)),
                                    0.85 * (1.0 - (rand()%2000 * 0.001)),
                                    0.85 * (1.0 - (rand()%2000 * 0.001))  );
            vel.Normalize();
            vel *= 50.0;

            initBuff[i].velocity = Vector4(vel.x,
             -fabs(vel.y),
             vel.z,
             0.0);

            count++;
        }

        LOGOUTPUT << " init count: " << count << "\n length: " << length;
        LogInfo();

//         CheckGLErrors();
//         LogInfo("unmapping");
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    
    void BuildGrid()
    {
        //extract positions from vram
 
        CheckGLErrors();
        glUseProgram(buildGridShader->GetProgram());
        CheckGLErrors();

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuff);
        //using smaller positions buffer to limit pcie bandwidth consumption
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, positionsBuff);

        buildGridShader->SendUniforms();
        glDispatchCompute(NUM_WORKGROUPS, 1, 1);
        CheckGLErrors();
       // LogInfo("positions updated");
        glBindBuffer(GL_ARRAY_BUFFER, positionsBuff);
        positions = reinterpret_cast<Vector3 *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, numParticles * 12,
                                                                GL_MAP_READ_BIT ));
        CheckGLErrors();
        //the grid will be a 3d array of integers, an index list.
        //find xyz min/max
        Vector3 min = positions[0];
        Vector3 max = positions[0];
//         for (unsigned int i = 1; i < numParticles; i++)
//         {
//             Vector3 p = positions[i];
//             if      (p.x < min.x) min.x = p.x;
//             else if (p.x > max.x) max.x = p.x;
//             
//             if      (p.y < min.y) min.y = p.y;
//             else if (p.y > max.y) max.y = p.y;
//             
//             if      (p.z < min.z) min.z = p.z;
//             else if (p.z > max.z) max.z = p.z;
//             
//         }
        //little margin never hurt
        min -= Vector3(1,1,1);
        max += Vector3(1,1,1);
        
        grid.Prepare();
        //very simple grid right now, just uses position & adjust for min/max
        int x,y,z,idx;
        int dx = (fabs(min.x) + fabs(max.x)) / indicesPerGridCell;
        int dy = (fabs(min.y) + fabs(max.y)) / indicesPerGridCell;
        int dz = (fabs(min.z) + fabs(max.z)) / indicesPerGridCell;
//         for (unsigned int i = 0; i < numParticles; i++)
//         {
//             x = positions[i].x / dx;
//             y = positions[i].y / dy;
//             z = positions[i].z / dz;
//             idx = x + indicesPerGridCell * (y + indicesPerGridCell * z);
//             //check to make sure theres room
//             //-1 because count takes up one space
//             unsigned int count = grid.data[idx];
//             if (count < indicesPerGridCell-1)
//             {
//                 grid.data[idx + 1 + count] = i;
//                 grid.data[idx] += 1; //increase count
//             }
//             //i = x + length * (y + length * z);
//         }
        
        glUnmapBuffer(GL_ARRAY_BUFFER);

    }
    
    void Update(float dt)
    {

       // BuildGrid();
        
        //run compute shader
        CheckGLErrors();
       // LogInfo("use prog.");
        glUseProgram(computeShader->GetProgram());
        CheckGLErrors();
       // LogInfo("bind base");
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuff);
        CheckGLErrors();

        if(gInput->GetKeyDownThisFrame(KB_R))
            Reset();

        computeShader->SendUniforms();
        dt = GetDTSeconds();
        if(dt > 0.02)
            dt = 0.02; //no crazy big dt's.

        glUniform1f(computeShader->GetUniformLocation("dt"), dt);
        
       // LogInfo("dispatching.");
        //updates positions
        glDispatchCompute(NUM_WORKGROUPS, 1, 1);

        //build new grid.

        CheckGLErrors();
       // LogInfo("done.");
        
    }

    void RenderCallback()
    {

        
        Matrix4 vp;
        Matrix4Multiply(vp, gRenderer->GetCamera()->GetProjection(), gRenderer->GetCamera()->GetInverseView());

       // glEnable(GL_BLEND);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glUseProgram(drawShader->GetProgram());
        //glBindVertexArray(renderVAO);

        //glBindBuffer(GL_ARRAY_BUFFER, cube->vertPositions);
        //glVertexAttribPointer(locVertPos, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        //glBindBuffer(GL_ARRAY_BUFFER, cube->vertNormals);
       // glVertexAttribPointer(locVertNormal, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, dataBuff);
        glVertexAttribPointer(locPosition, 4, GL_FLOAT, GL_FALSE, sizeof(particle), (void *)0);
        glVertexAttribPointer(locColor, 4, GL_FLOAT, GL_FALSE, sizeof(particle), (void *)sizeof(Vector4));

       // glVertexAttribDivisor(locPosition, 1);
       // glVertexAttribDivisor(locColor, 1);

        //glEnableVertexAttribArray(locVertPos);
        //glEnableVertexAttribArray(locVertNormal);
        glEnableVertexAttribArray(locPosition);
        glEnableVertexAttribArray(locColor);
        
        drawShader->SendUniforms();
        glUniformMatrix4fv(drawShader->GetUniformLocation("ViewProjection"), 1, false, (GLfloat *)&vp[0]);
        
        glDrawArrays(GL_POINTS, 0, numParticles);
        //my gtx 750 can't handle transforming that many verts
      // glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, cube->numVerts, numParticles);
        
        glDisableVertexAttribArray(locPosition);
        glDisableVertexAttribArray(locColor);
        //glDisableVertexAttribArray(locVertNormal);
       // glDisableVertexAttribArray(locVertPos);
       // glVertexAttribDivisor(locPosition, 0);
      //  glVertexAttribDivisor(locColor, 0);
        
        CheckGLErrors();
        glDisable(GL_BLEND);
    }

    void Destroy()
    {
       DisarmRenderCallback();
       glDeleteBuffers(1, &dataBuff);
    }

};


#endif
