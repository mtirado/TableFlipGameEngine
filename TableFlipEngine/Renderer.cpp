/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#define DEBUG_SHADOWS 0
#define MOBILE_SHADOWS
#include "Renderer.h"
#include "Input.h"
#include "Utilities.h"
#include "SOIL.h"
#include "Builder.h"
#include "text-buffer.h"
//#include "Scripting.h"
#include "ObjectManager.h"
#include "UserInterface.h"
#if defined(__ANDROID__) || defined(__WEB_GL__)
    #include<stdio.h>
#else
    #include <cstdio>
#endif

using std::multimap;
using std::vector;
using std::pair;

Renderer *Renderer::instance = 0;
float tmpRot = 0.0f;
GLFrustum frustum;

Renderer::Renderer()
{
    initDone = false;
    debugOctree = 0;
    skyMap = 0;
    debugDraw = false;
    currentPass = 0;
    camera = 0;
    fontTex = 0;
    testTex = 0;
    shadowDepthWrite = 0;
    skyCube = new ModelResource(0);
}
Renderer::~Renderer()
{
    skyCube->Delete();
    if (skyMap)
        glDeleteTextures(1, &skyMap);
}

void Renderer::InitInstance()
{
    if (!instance)
        instance = new Renderer;
}

void Renderer::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

void RegisterGLExtensions()
{
#if defined(PC_PLATFORM)
    glewInit();
#endif
}



bool Renderer::Init()
{

    LOGOUTPUT << "OpenGL Version: "<< glGetString(GL_VERSION);
    LogInfo();
    
    RegisterGLExtensions();

    if (!defaultMaterial.LoadShader("Data/Shaders/Colored-lit.shader"))
    {
        LogError("error loading default diffuse");
        return false;
    }
    else
    {
        defaultMaterial.RegisterProjectionMatrix("ProjectionMatrix");
        defaultMaterial.RegisterViewMatrix("ViewMatrix");
    }
        
    dbgFloat = 0.0f;    
    //set clear color
    glClearStencil(0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //backface culling 
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
   
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    //testTex = SOIL_load_OGL_texture("Data/Textures/fishTex.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
    //SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_INVERT_Y | SOIL_FLAG_MIPMAPS | SOIL_FLAG_COMPRESS_TO_DXT);
    CheckGLErrors();
    LogInfo("terrain init");
    
   // LogInfo("terrain init");
    projection2D.SetOrthographic(0.0f, 100.0f, 0.0f, 100.0f, 0.00f, 100.0f);
   // terrain.Init(1500.0f); //real radius will be this * persistance (i think)
    CheckGLErrors();
   // LogInfo("Load Timecube");
   // terrain.LoadTimeCube(6,3, 142578, 0.2050253, 3.10, true);
   // terrain.LoadHeightmap("Data/Textures/heightmap.bmp");

    
    CheckGLErrors();
    LogInfo("skycube load");
    //load box
    skyCube->LoadCube(true);
    CheckGLErrors();
    //LogInfo("smymap shader load");
    //skyMapShader = LoadShaderResource("Data/Shaders/skymap.shader");
    CheckGLErrors();
 //   LogInfo("skymap load");
//     skyMap = SOIL_load_OGL_cubemap("Data/Textures/complex-nebula_right.png", "Data/Textures/complex-nebula_left.png",
//         "Data/Textures/complex-nebula_bottom.png", "Data/Textures/complex-nebula_top.png",
//         "Data/Textures/complex-nebula_front.png", "Data/Textures/complex-nebula_back.png",
//         SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
// 
//     LOGOUTPUT << "\n\nCubemap Loaded, return value: " << skyMap << "\nSOIL RESULT: " << SOIL_last_result() << "\n\n";
//     LogInfo();


    CheckGLErrors();
    LogInfo("texcube params");
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    
   // mc->uniformLinks[i].data = LoadTexture(nameBuff, /*SOIL_LOAD_AUTO*/SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
   // SOIL_FLAG_INVERT_Y | /*SOIL_FLAG_MIPMAPS |*/ flagRepeat, priority);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
#if !defined(__WEB_GL__) && !defined(MOBILE_SHADOWS)
    shadowDepthWrite = LoadShaderFromFile("Data/Shaders/ShadowDepthWrite.shader");
    if (!shadowDepthWrite)
    {
        LogError("could not load shadow depth write shader, disabling shadow mapping.");
        //TODO disable shadow mapping...
    }
    else
    {
       shadowMVPLoc = shadowDepthWrite->GetUniformLocation("MVP");
    }
    
    dbgQuad = LoadShaderFromFile("Data/Shaders/DebugQuad.shader");
    //init framebuffer - depth texture for shadowmapping
  
    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //calmp_to_edge?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    if (!DEBUG_SHADOWS)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    
    glDrawBuffer(GL_NONE); // No color buffer is drawn to.
    glReadBuffer(GL_NONE);
    

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        LogError("error generating frame buffer texture");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    static const GLfloat g_quad_vertex_buffer_data[] = { 
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
        };
        
                
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
#endif

    //load skydome
    //bpSkydome = LoadBlueprint("Data/Blueprints/Sky.bp");
    //skydome = Builder::GetInstance()->ConstructEntity(bpSkydome, Vector3(0,0,0));
    //RemoveMesh(skydome->GetMesh());


    CheckGLErrors();
    LogInfo("Renderer init done");
    
    return true;
}

void Renderer::PreRender()
{


  if (Input::GetInstance()->GetKeyDownThisFrame(KB_F12))
    debugDraw = !debugDraw;
    
    if (camera)
    {
        camera->Update(1.0f); //static time scale for this bitch
        camera->frustum = &frustum;
        Matrix4 invTransform, tmp;
        camera->GetTransform()->GetTransform(tmp);
        MatrixInvert(invTransform, tmp);
    }

    glClearStencil(0);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClearColor(0.360f,0.650f,0.890f,1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   
    ptLight.position = Vector3(0.0f, 89.0f, -0.0f);
    ptLight.diffuseColor = Vector4(1.0f,1.0f,1.0f,1.0f);
    ptLight.specularColor = Vector4(1.0f,1.0f,1.0f,1.0f);

    
   
    
    
}

void testGrid()
{
}

//should take different cameras, one day
void Renderer::DrawScene()
{
}

float addit = 0.0f;
Matrix4 shadowVP;
Matrix4 shadowV, shadowM;
Matrix4 depthMVP; Matrix4 biasMVP;
Matrix4 tmp;
Matrix4 inv;
bool rdbgOnce = true;    
void Renderer::Render3D()
{
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    
    if (rdbgOnce) LogInfo("top");
    CheckGLErrors();
    Matrix4Identity(shadowM);

     if (rdbgOnce) LogInfo("setting sunlight params");
    
    sunLight.SetDirection(Vector3(-0.8,-2.4,0.6));
    sunLight.SetShadowFrustumDistance(30);
    sunLight.GetViewProjection(shadowVP);
    CheckGLErrors();
    if (rdbgOnce) LogInfo("render unsorted");
    RenderUnsortedMeshes();
    CheckGLErrors();
    //if (rdbgOnce) LogInfo("draw terrain");
    //terrain.Draw();
   // CheckGLErrors();
    if (rdbgOnce) LogInfo("Render Transparent");
    RenderTransparentMeshes();

     CheckGLErrors();
    if (rdbgOnce) LogInfo("Particles Draw");
    //raahh particles need to be sorted for transparency too!?
    RenderParticles();

    //any render callbacks?
    for (int i = 0; i < renderCallbacks.size(); i++)
    {
        renderCallbacks[i]->RenderCallback();
    }    
    
    CheckGLErrors();
    if (rdbgOnce) LogInfo("Debug Draw");
    DebugDraw();
    CheckGLErrors();

    if (rdbgOnce) LogInfo("Done");

    rdbgOnce = false;
    //glDisable(GL_BLEND);

    if (debugOctree)
        OctreeDraw(debugOctree);

}


void Renderer::PostRender()
{
    glCapture.Update(GetDTSeconds());
    if (gInput->GetKeyDownThisFrame(KB_F9))
    {
        if (glCapture.isCapturing())
            glCapture.EndCapture();
        else
            glCapture.StartCapture("testing", width, height);
    }
}

void Renderer::Shutdown()
{
}

void Renderer::ResizeWindow(unsigned int w, unsigned int h)
{
    if (w == 0)  w = 1; width = w;
    if (h == 0)  h = 1; height = h;
    resolution.x = width;
    resolution.y = height;
   
    glViewport(0, 0, width, height);
    text_buffer_set_screensize(width, height);
                                                            //1.0, 1000.0,Z
    frustum.SetPerspective(63.0f, (GLfloat)w / (GLfloat)h, 0.01f, 999.0f);
    if (camera)
      camera->frustum = &frustum;
    
    
    float fovy = 63; // degrees
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    nearPlaneHeight = (float)abs(viewport[3]-viewport[1]) /
        (2*tan(0.5*fovy*PI/180.0));

    float aspectRatio;
    aspectRatio = (float)width / (float)height;

    projection2D.SetOrthographic(0, w, h, 0.0, 0.0, 1000.0);
    projectionUI.SetOrthographic(0.0, width, height, 0.0, 0.0, 1000.0); 
    //oriented at bottom left because thats how freetype do :[
    projectionFont.SetOrthographic(0.0, w, 0.0, h, 0.0, 1000.0);

    if (isReady())
    {
        //errr
        gUIManager->SetRootWindowDimensions(width, height);
        gUIManager->InvalidateElements();
    }


    
}

void Renderer::DrawSky()
{
      // skydome
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
     //draw sky
    skydome->GetTransform()->position = camera->GetTransform()->position;
    skydome->GetTransform()->UpdatePosition();
    Mesh* mesh = skydome->GetMesh();
       
    glUseProgram(mesh->material->program);
    mesh->material->BindAttributes();

    //send view and projection matrices,  
    mesh->material->SetProjectionMatrix(camera->GetProjection());
    //  MatrixInvert(inv, camera->GetTransform()->transform);
    mesh->material->SetViewMatrix(camera->GetInverseView());

    //TODO: need to batch these for static meshes
    mesh->PrepareMaterial(); 
    
    //send any registered uniforms
    mesh->material->SendUniformData();
    
    glDrawArrays(GL_TRIANGLES, 0, mesh->numVerts);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);
    
/*
    glDisable(GL_CULL_FACE);
    //skycube
    Matrix4 m;
    Matrix4 vp;
    Matrix4 mvp;
    Matrix4Identity(m);
    Vector3 pos = camera->GetTransform()->position;
    m[12] = pos.x;
    m[13] = pos.y;
    m[14] = pos.z;
    Matrix4Multiply(vp, camera->GetProjection(), camera->GetInverseView());
    Matrix4Multiply(mvp, vp, m);
    
    glUseProgram(skyMapShader->GetProgram());
    
    //attribute
    skyMapShader->BindAttributes();
    glBindBuffer(GL_ARRAY_BUFFER, skyCube->vertPositions);
    glEnableVertexAttribArray(skyMapShader->GetAttributeLocation(ATR_POSITION0));
    glVertexAttribPointer(skyMapShader->GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, 0);

    //uniforms
    glUniformMatrix4fv(skyMapShader->GetUniformLocation("mvp"), 1, false, (GLfloat *)mvp);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyMap);
    glUniform1i(skyMapShader->GetUniformLocation("cubemap"), 0); //default stage is 0

    glDrawArrays(GL_TRIANGLES, 0, skyCube->numVerts);
     
    glEnable(GL_CULL_FACE);*/
    
}


void Renderer::RenderUnsortedMeshes()
{
    if (rdbgOnce) LogInfo("top unsorted");
    CheckGLErrors();
    multimap<unsigned int, Mesh *>::iterator iter;
    RenderShadowDepthBuffer();
    //this is fucking horribly ugly, but lets just get shadow mapping working please... kthx
    currentPass = 2;
    
    glViewport(0, 0, width, height);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
   // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //bind shadowmap

     if (rdbgOnce) LogInfo("draw sky");
    



    //draw sky!
    //DrawSky();



    CheckGLErrors();
    //glUniform1i(shadowDepthWrite->GetUniformLocation("shadowMap"), 1);
    if (rdbgOnce) LogInfo("draw meshes");
    iter = unsortedMeshes.begin();
    if (iter != unsortedMeshes.end())
    {        
        Mesh* lastMesh = iter->second;
        unsigned int curShaderId = iter->first;
        glUseProgram(iter->second->material->program);
        iter->second->material->BindAttributes();

        //send view and projection matrices,  
        
        iter->second->material->SetProjectionMatrix(camera->GetProjection());
      //  MatrixInvert(inv, camera->GetTransform()->transform);
        iter->second->material->SetViewMatrix(camera->GetInverseView());
        
       
        while (iter != unsortedMeshes.end())
        {
            if (iter->second->hidden)
            {
                iter++;
                continue;
            }
            //TODO: need to batch these for static meshes
            iter->second->PrepareMaterial(); 
            
            if (iter->first != curShaderId) //re initialize for every new shader.
            {
            
                lastMesh->FinishMaterial();
                lastMesh = iter->second;
                //shader changing
                glUseProgram(iter->second->material->program);
                iter->second->material->BindAttributes();
                
                iter->second->PrepareMaterial();
                
                //send view and projection matrices, materials are responsible for registering other matrices 
                
                iter->second->material->SetProjectionMatrix(camera->GetProjection());
                //MatrixInvert(inv, camera->GetTransform()->transform);
                iter->second->material->SetViewMatrix(camera->GetInverseView());
            }
            
            //TODO hacky inverse model matrix, you need to use global space for lighting
            //and eliminate this extra transform
            iter->second->transform->GetTransform(tmp);
            MatrixInvert(hackyMatrix, tmp);
            glUniform3fv(iter->second->material->GetUniformLocation("lightDir") , 1, (GLfloat *)sunLight.GetInvDir());
            
            //
            SendShadowData(iter);

            //send any registered uniforms
            iter->second->material->SendUniformData();
            Matrix3 rotation;
            Matrix4ExtractRotation(rotation, hackyMatrix); //needs inverse rotation
            glUniformMatrix3fv(iter->second->material->GetUniformLocation("invRotation"), 1, GL_FALSE, rotation);
            
            //assuming triangles for now...  
            if (iter->second->isIndexed)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iter->second->triIndices);
            
                glDrawElements(
                    GL_TRIANGLES,      // mode
                    iter->second->numTris * 3,    // count
                    GL_UNSIGNED_INT,   // type
                    (void*)0           // element array buffer offset
                );
            }
            else
                glDrawArrays(GL_TRIANGLES, 0, iter->second->numVerts);
            
            
            iter++;
        }
        lastMesh->FinishMaterial();
    }
    CheckGLErrors();
}

void Renderer::RenderTransparentMeshes()
{
    multimap<unsigned int, Mesh *>::iterator iter;
    //RenderShadowDepthBuffer(); //TODO i think shadow depth render will need to also render transparent objects
    
    //this is fucking horribly ugly, but lets just get shadow mapping working please... kthx
    currentPass = 2;
    
    glViewport(0, 0, width, height);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
   // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //bind shadowmap
   
    //glUniform1i(shadowDepthWrite->GetUniformLocation("shadowMap"), 1);
    
    iter = transparentMeshes.begin();
    if (iter != transparentMeshes.end())
    {        
        Mesh* lastMesh = iter->second;
        unsigned int curShaderId = iter->first;
        glUseProgram(iter->second->material->program);
        iter->second->material->BindAttributes();

        //send view and projection matrices,  
        
        iter->second->material->SetProjectionMatrix(camera->GetProjection());
      //  MatrixInvert(inv, camera->GetTransform()->transform);
        iter->second->material->SetViewMatrix(camera->GetInverseView());
        
       
        while (iter != transparentMeshes.end())
        {
            if (iter->second->hidden)
            {
                iter++;
                continue;
            }
            //TODO: need to batch these for static meshes
            iter->second->PrepareMaterial(); 
            
            if (iter->first != curShaderId) //re initialize for every new shader.
            {
            
                lastMesh->FinishMaterial();
                lastMesh = iter->second;
                //shader changing
                glUseProgram(iter->second->material->program);
                iter->second->material->BindAttributes();
                
                iter->second->PrepareMaterial();
                
                //send view and projection matrices, materials are responsible for registering other matrices 
                
                iter->second->material->SetProjectionMatrix(camera->GetProjection());
                MatrixInvert(inv, camera->GetTransform()->transform);
                iter->second->material->SetViewMatrix(camera->GetInverseView());
            }
            
            //TODO hacky inverse model matrix, you need to use global space for lighting
            //and eliminate this extra transform
            iter->second->transform->GetTransform(tmp);
            MatrixInvert(hackyMatrix, tmp);
            glUniform3fv(iter->second->material->GetUniformLocation("lightDir") , 1, (GLfloat *)sunLight.GetInvDir());
            
            //TODO shadow transparent objects??
            //SendShadowData(iter);

            //send any registered uniforms
            iter->second->material->SendUniformData();
            Matrix3 rotation;
            Matrix4ExtractRotation(rotation, hackyMatrix); //needs inverse rotation
            glUniformMatrix3fv(iter->second->material->GetUniformLocation("invRotation"), 1, GL_FALSE, rotation);
            
            //assuming triangles for now...  
            if (iter->second->isIndexed)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iter->second->triIndices);
            
                glDrawElements(
                    GL_TRIANGLES,      // mode
                    iter->second->numTris * 3,    // count
                    GL_UNSIGNED_INT,   // type
                    (void*)0           // element array buffer offset
                );
            }
            else
                glDrawArrays(GL_TRIANGLES, 0, iter->second->numVerts);
            
            
            iter++;
        }
        lastMesh->FinishMaterial();
    }
}


//renders objects into shadow buffer
void Renderer::RenderShadowDepthBuffer()
{
    
#if !defined(__WEB_GL__) && !defined(MOBILE_SHADOWS)

    multimap<unsigned int, Mesh *>::iterator iter;
    
    biasMat[0] = 0.5f;biasMat[4] = 0.0f;biasMat[8] = 0.0f;biasMat[12] = 0.5f;
    biasMat[1] = 0.0f;biasMat[5] = 0.5f;biasMat[9] = 0.0f;biasMat[13] = 0.5f;
    biasMat[2] = 0.0f;biasMat[6] = 0.0f;biasMat[10] = 0.5f;biasMat[14] = 0.5f;
    biasMat[3] = 0.0f;biasMat[7] = 0.0f;biasMat[11] = 0.0f;biasMat[15] = 1.0f;

// Matrix4Identity(biasMat);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    iter = unsortedMeshes.begin();
    if (iter != unsortedMeshes.end())
    {        
        
        //draw the the whole frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0,0,1024,1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right
        
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        
        
        while (iter != unsortedMeshes.end())
        {
        
            glUseProgram(shadowDepthWrite->GetProgram());
            shadowDepthWrite->BindAttributes();
            iter->second->PrepareMaterial();
            
            //shadowvp
            Matrix4Multiply(depthMVP, shadowVP, (*iter).second->transform->transform);
            
            glUniformMatrix4fv(shadowDepthWrite->GetUniformLocation("MVP"), 1, GL_FALSE, depthMVP); //depth MVP
            //assuming triangles for now...  
            if (iter->second->isIndexed)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iter->second->triIndices);
            
                glDrawElements(
                    GL_TRIANGLES,      // mode
                    iter->second->numTris * 3,    // count
                    GL_UNSIGNED_INT,   // type
                    (void*)0           // element array buffer offset
                );
            }
            else
                glDrawArrays(GL_TRIANGLES, 0, iter->second->numVerts);
                    
            iter++;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void Renderer::SendShadowData(multimap< unsigned int, Mesh* >::iterator& iter)
{
#if !defined(__WEB_GL__) && !defined(MOBILE_SHADOWS)
        //shadow matrix
        Matrix4Multiply(depthMVP, shadowVP, (*iter).second->transform->transform);
        Matrix4Multiply(depthBiasMVP, biasMat, depthMVP );
   
        glUniformMatrix4fv(iter->second->material->GetUniformLocation("DepthBiasMVP"), 1, GL_FALSE, depthBiasMVP);
 
        //TODO this probably isnt optimal to send this for every mesh
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glUniform1i(iter->second->material->GetUniformLocation("shadowMap"), 1);
#endif
}

void Renderer::DebugDraw()
{
    //we can define MOBILE_SHADOWS in pc build to run mobile shadow shader
#if !defined(__WEB_GL__) && !defined(MOBILE_SHADOWS)
    if (DEBUG_SHADOWS)
    {
        //render debug quad
        glViewport(0,0,256,256);
        glUseProgram(dbgQuad->GetProgram());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        // Set our "renderedTexture" sampler to user Texture Unit 0
        glUniform1i(dbgQuad->GetUniformLocation("texture"), 0);

        // glUniform1f(timeID, (float)(glfwGetTime()*10.0f) );
        glEnableVertexAttribArray(dbgQuad->GetAttributeLocation(ATR_POSITION0));
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                dbgQuad->GetAttributeLocation(ATR_POSITION0), // attribute
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void*)0                          // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, 6); // From index 0 to 6 -> 2 triangles
        glViewport(0,0,width,height);
    }



    CheckGLErrors();
    
#endif
#if defined(PC_PLATFORM)
    glUseProgram(0);
    if (debugDraw)  //draw debug info
    {
        glLineWidth(2.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(camera->GetProjection());
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(camera->inverseView);
        Physics::GetInstance()->DrawDebugWireframe();
    }
#endif
}

void Renderer::Render2D()
{
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);



    //after UI so fragments under ui elements can get thrown out?

    
    
    RenderStenciledSprites();
    //glDisable(GL_DEPTH_TEST); //HACK
    RenderSprites(SPRITE_TYPE_NORMAL);
    //glEnable(GL_DEPTH_TEST);
    
    glViewport(0, 0, width, height);
    if (UIManager::GetInstance())
        UIManager::GetInstance()->DrawWindows();

}

void Renderer::RenderStenciledSprites()
{
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_ALPHA_TEST);
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    glAlphaFunc( GL_GREATER, 0.0 );
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

    //draw stencils
    
    RenderSprites(SPRITE_TYPE_STENCILS);
    

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);
    RenderSprites(SPRITE_TYPE_CLIPPED);
    glDepthMask(GL_TRUE);


    
}

void Renderer::RenderSprites(SpriteType spriteType)
{

    unsigned int lastSrc, lastDst;
    glEnable(GL_BLEND);
   // glBlendFunc(GL_DST_COLOR, GL_ZERO);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    
    multimap<unsigned int, Sprite *>::iterator iter;
    multimap<unsigned int, Sprite *>::iterator end;

    if (spriteType == SPRITE_TYPE_NORMAL)
    {
        iter = sprites.begin();
        end  = sprites.end();
    }
    else if (spriteType == SPRITE_TYPE_STENCILS)
    {
        iter = stencils.begin();
        end  = stencils.end();
    }
    else if (spriteType == SPRITE_TYPE_CLIPPED)
    {
        //glDisable();
        iter = clippedSprites.begin();
        end  = clippedSprites.end();
    }
    else
        return;
    
    if (iter != end)
    {        
        Sprite *lastSprite = iter->second;
      
        unsigned int curShaderId = iter->first;
         //TODO: need to batch these properly
        Material *mat = iter->second->material;
        lastSrc = mat->srcBlend;
        lastDst = mat->dstBlend;
        glBlendFunc(lastSrc, lastDst);
        glUseProgram(mat->getProgram());
        iter->second->material->BindAttributes();
        //send view and projection matrices,   
        mat->SetProjectionMatrix(Get2DFrustum()->projMatrix);
        iter->second->PrepareMaterial(); 
            
        while (iter != end)
        {
            mat = iter->second->material;
            if (lastDst != mat->dstBlend || lastSrc != mat->srcBlend)
            {//ughh state changes..
                lastSrc = mat->srcBlend;
                lastDst = mat->dstBlend;
                glBlendFunc(lastSrc, lastDst);
            }
               
            if (iter->first != curShaderId) //re initialize for every new shader.
            {
                //shader changing
                glUseProgram(mat->getProgram());
    
            }
            if (lastSprite->material != mat) //redundant?
            {
                lastSprite->FinishMaterial();
                lastSprite = iter->second;
               
                mat->BindAttributes();
                iter->second->PrepareMaterial();
                mat->SetProjectionMatrix(Get2DFrustum()->projMatrix);
            }
            
            //send any registered uniforms
         //   iter->second->material->setTransformPtr((float *)iter->second->entity->GetTransform()->transform);
          //  iter->second->material->SendUniformData();
            
            
                if (iter->second->spriteResource->isTiled())
                {
                    Matrix4 tPos;
                    Matrix4Copy(tPos, iter->second->entity->GetTransform()->transform);
                    for (int i = 0; i < iter->second->spriteResource->map.numLayers; i++)
                    {
                        //z
                        //renders background to foreground
                        tPos[14] += 0.01;
                        iter->second->entity->GetTransform()->UpdatePosition();
                        iter->second->material->setTransformPtr((float *)tPos);
                        iter->second->material->SendUniformData();
                        
                        glBindBuffer(GL_ARRAY_BUFFER, iter->second->spriteResource->map.vertPositions[i]);  
                        glEnableVertexAttribArray(iter->second->material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
                        glVertexAttribPointer(iter->second->material->GetAttributeLocation(ATR_POSITION0), 2, GL_FLOAT, GL_FALSE, 0, 0);
                        
                        glBindBuffer(GL_ARRAY_BUFFER, iter->second->spriteResource->map.vertTexCoords[i]);
                        glEnableVertexAttribArray(iter->second->material->GetAttributeLocation(ATR_TEXCOORD0));//tc location
                        glVertexAttribPointer(iter->second->material->GetAttributeLocation(ATR_TEXCOORD0), 2, GL_FLOAT, GL_FALSE, 0, 0);
                        glDrawArrays(GL_TRIANGLES, 0, iter->second->spriteResource->map.vertCount[i]);
                        
                    }
                }
                else
                {
                    iter->second->material->setTransformPtr((float *)iter->second->entity->GetTransform()->transform);
                    iter->second->material->SendUniformData();
                    glDrawArrays(GL_TRIANGLES, iter->second->curFrame * iter->second->numVerts, iter->second->numVerts);
                }
            iter++;    
        }
            
        lastSprite->FinishMaterial();
    }
    glDisable(GL_BLEND);
}


void Renderer::RenderParticles()
{
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc( GL_ONE, GL_DST_COLOR );
    multimap<unsigned int, ParticleSystem *>::iterator iter;

    Matrix4 proj;
    Matrix4 view;
    Matrix4 viewProj;
    Matrix4 wvpMatrix;
    Matrix4Copy(proj, camera->GetProjection());
    Matrix4Copy(view, camera->GetInverseView());
    //create view projection, multiply with world transform for wvpMatrix
    Matrix4Multiply(viewProj, proj, view);

    iter = particleSystems.begin();
    if (iter != particleSystems.end())
    {
        ParticleSystem *lastParticle = iter->second;

        unsigned int curShaderId = iter->first;

        glUseProgram(iter->second->material->getProgram());
        iter->second->material->BindAttributes();
        
        iter->second->PrepareMaterial();

        while (iter != particleSystems.end())
        {

            if (iter->first != curShaderId) //re initialize for every new shader.
            {
                //shader changing
                glUseProgram(iter->second->material->getProgram());

            }
            if (lastParticle->material != iter->second->material) //redundant?
            {
                lastParticle->FinishMaterial();
                lastParticle = iter->second;

                iter->second->material->BindAttributes();
                iter->second->PrepareMaterial();
            }

            iter->second->Update();
            //iter->second->material->setTransformPtr((float *)iter->second->entity->GetTransform()->transform);
            Matrix4Multiply(wvpMatrix, viewProj, iter->second->entity->GetTransform()->transform);
            iter->second->material->SetWorldViewProjMatrix(wvpMatrix);
            iter->second->material->SendUniformData();
            
            glDrawArrays(GL_POINTS, 0, iter->second->getNumActive());
                
            iter++;
        }

        lastParticle->FinishMaterial();
    }
    glDisable(GL_BLEND);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}



bool Renderer::AddMesh(Mesh *obj) 
{ 
    if (obj->numVerts && obj->material)
    {
        //sorted by shader ID
        unsortedMeshes.insert(pair<unsigned int, Mesh *>(obj->material->GetShaderId(), obj));
        return true;
    }
    else
    {
        LogError("Renderer could not AddMesh");
        return false;
    }
}


//TODO change this to unsorted meshes...
void Renderer::RemoveMesh (Mesh* obj)
{
    std::multimap<unsigned int, Mesh*>::iterator iter;
    
    for (iter = unsortedMeshes.begin(); iter != unsortedMeshes.end(); iter++)
    {
        if ((*iter).second == obj)
        {
            unsortedMeshes.erase(iter);
            return;            
        }
    }
}

bool Renderer::AddTransparentMesh(Mesh *obj) 
{ 
    if (obj->numVerts && obj->material)
    {
        //sorted by shader ID
        transparentMeshes.insert(pair<unsigned int, Mesh *>(obj->material->GetShaderId(), obj));
        return true;
    }
    else
    {
        LogError("Renderer could not AddMesh");
        return false;
    }
}

//TODO change this to unsorted meshes...
void Renderer::RemoveTransparentMesh (Mesh* obj)
{
    std::multimap<unsigned int, Mesh*>::iterator iter;
    
    for (iter = transparentMeshes.begin(); iter != transparentMeshes.end(); iter++)
    {
        if ((*iter).second == obj)
        {
            transparentMeshes.erase(iter);
            return;            
        }
    }
}

bool Renderer::AddSprite(Sprite* obj)
{
    if (obj->getMaterial())
    {
        SpriteType type = obj->spriteType;
        if (type == SPRITE_TYPE_NORMAL)
            sprites.insert(pair<unsigned int, Sprite *>(obj->getShaderId(), obj));
        else if (type == SPRITE_TYPE_STENCILS)
            stencils.insert(pair<unsigned int, Sprite *>(obj->getShaderId(), obj));
        else if (type == SPRITE_TYPE_CLIPPED)
            clippedSprites.insert(pair<unsigned int, Sprite *>(obj->getShaderId(), obj));
        else return false;
        return true;
    }
    else
    {
        LogError("Renderer::AddSprite()  - no material!");
        return false;
    }
}

//TODO for all of these... instead of iteraing the whole damn map
//see if we can just get a list of that shader id key
void Renderer::RemoveSprite(Sprite* obj)
{
    SpriteType type = obj->spriteType;
    std::multimap<unsigned int, Sprite*>::iterator iter;
    std::multimap<unsigned int, Sprite*>::iterator end;
    if (type == SPRITE_TYPE_NORMAL)
    {
        iter = sprites.begin();
        end  = sprites.end();
    }
    else if (type == SPRITE_TYPE_STENCILS)
    {
        iter = stencils.begin();
        end  = stencils.end();
    }
    else if (type == SPRITE_TYPE_CLIPPED)
    {
        iter = clippedSprites.begin();
        end  = clippedSprites.end();
    }
    else
        return;
    
    
    for ( ; iter != end; iter++)
    {
        if ((*iter).second == obj)
        {
            sprites.erase(iter);
            return;            
        }
    }
}

bool Renderer::AddParticleSystem(ParticleSystem *obj)
{
    if (obj->hasMaterial())
    {
        particleSystems.insert(pair<unsigned int, ParticleSystem *>(obj->getShaderId(), obj));
        return true;
    }
    else
    {
        LogError("Renderer::AddParticleSystem()  - no material!");
        return false;
    }
}

void Renderer::RemoveParticleSystem(ParticleSystem *obj)
{
    std::multimap<unsigned int, ParticleSystem*>::iterator iter;
    for (iter = particleSystems.begin(); iter != particleSystems.end(); iter++)
    {
        if ((*iter).second == obj)
        {
            particleSystems.erase(iter);
            return;//otherwise iterator is screwed, probably a better way to bedoing this
            //since right now it doesnt check for duplicates when adding or removing :[
        }
    }
}

bool Renderer::AddRenderCallback(Logic *logic)
{
    //i guess just check for duplicates
    for (int i = 0; i < renderCallbacks.size(); i++)
    {
        if (renderCallbacks[i] == logic)
            return false;
    }

    renderCallbacks.push_back(logic);
    return true;
}

void Renderer::RemoveRenderCallback(Logic *logic)
{
    for (int i = 0; i < renderCallbacks.size(); i++)
    {
        if (renderCallbacks[i] == logic)
        {
            renderCallbacks.erase(renderCallbacks.begin()+i);
            return;
        }
    }
}
