/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef RENDERER_H__
#define RENDERER_H__

/*#if defined(__ANDROID__)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #define MOBILE_PLATFORM 
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
	#define PC_PLATFORM
#endif*/
#include "Utilities.h"
#include "BoundingVolumes.h"
#include <vector>
#include "Entity.h"
#include "Camera.h"
#include "Light.h"
#include "Components.h"
#include <ParticleSystem.h>
#include "Materials.h"
#include "Terrain.h"
#include "GLCapture.h"
#include <map>


class Renderer
{
private:
    //the singletons instance.
    static Renderer *instance;
    
    //disable functions for singleton use
    Renderer();
    ~Renderer();
    Renderer(const Renderer &) {}
    Renderer &operator = (const Renderer &) { return *this; }

    BlueprintResource *bpSkydome;
    Entity *skydome;
    bool initDone;

    std::vector<Mesh *> renderMeshes;
    
    //not render sorted, key = shader ID
    std::multimap<unsigned int, Mesh*> unsortedMeshes;
    std::multimap<unsigned int, Mesh*> transparentMeshes;
    std::multimap<unsigned int, Sprite*> stencils; //fill up stencil buffer for masking
    std::multimap<unsigned int, Sprite*> clippedSprites; //to be clipped...
    std::multimap<unsigned int, Sprite*> sprites; //unconstrained sprites render over everything
    std::multimap<unsigned int, ParticleSystem *> particleSystems;

    std::vector<Logic *> renderCallbacks;
       
    //GL texture that holds the font
   
    Material textMaterial;

    GLFrustum projection2D;
    GLFrustum projectionUI;
    GLFrustum projectionFont; //yup, inverted y for fonts
    
    
    GLuint framebuffer;
    ShaderResource *skyMapShader;
    ModelResource *skyCube; //bypass resource manager because i removed primitive loading :[
    GLuint skyMap;
    
    //render pass   seems like a crude method, but lets keep it untill something else is needed
    int currentPass;
    
    void RenderUnsortedMeshes();
    void RenderTransparentMeshes();
    void RenderStenciledSprites();
    void RenderSprites(SpriteType spriteType);
    
    //i have no idea what i'm doing with this, we'll see how many special cases for rendering we run into
    void RenderShadowDepthBuffer();
    void SendShadowData(std::multimap<unsigned int, Mesh *>::iterator &iter);
    void DebugDraw();

    void RenderParticles();

    GLCapture glCapture;

public:
    GLuint fontTex;
    Material defaultMaterial;
    ShaderResource *shadowDepthWrite; //
    ShaderResource *dbgQuad;
    GLuint quad_vertexbuffer;
    GLint shadowMVPLoc;
    
    Matrix4 biasMat; //alleviating errors?
    Matrix4 depthBiasMVP;
    GLuint depthTexture;

    Terrain terrain;
    void ReloadAsteroid(int subdivisions, unsigned int numOctaves, unsigned int seed, double lucanarity, double persistence)
    { terrain.Reload(subdivisions, numOctaves, seed, lucanarity, persistence); }
     
    GLuint testTex;
    //debugging variables
    int mouseX, mouseY;
    float dbgFloat;
    int dbgInt;
    bool mBtn;
    bool debugDraw;

    BVHOctreeNode *debugOctree;
   
    Vector2 resolution; 
    float width;
    float height;
    
    Matrix4 hackyMatrix;
    float nearPlaneHeight; // for correct point scaling
    
    //hacky light...
    PointLight ptLight;
    
    //direction light
    SunLight sunLight;

    //camera, only one for now
    Camera *camera;
    
    //inlines
    inline static Renderer *GetInstance() { return instance; }

    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();

    //initialize the render system
    bool Init();

    //pre render stage
    void PreRender();
    
    void DrawSky();
    
    //draw camera 1
    void DrawScene();

    //render stages
    void Render3D();
    void Render2D();

    //post render
    void PostRender();

    //shutdown the render system
    void Shutdown();

    //setup the viewport size
    void ResizeWindow(unsigned int w, unsigned int h);

    //returns the old font
    GLuint SetFont(GLuint hFontTex);


    //add / remove stuff
    bool AddMesh(Mesh *obj);
    void RemoveMesh (Mesh* obj);
    bool AddTransparentMesh(Mesh *obj);
    void RemoveTransparentMesh(Mesh *obj);
    bool AddSprite(Sprite *obj);
    void RemoveSprite(Sprite *obj);
    bool AddParticleSystem(ParticleSystem *obj);
    void RemoveParticleSystem(ParticleSystem *obj);

    //i think compute shaders will need render callbacks
    //maybe at various stages... adding to endfor now....
    bool AddRenderCallback( Logic *logic );
    void RemoveRenderCallback( Logic *logic );

    inline Camera *GetCamera()         { return camera;          }
    inline GLFrustum *Get2DFrustum()   { return &projection2D;   }
    inline GLFrustum *GetUIFrustum()   { return &projectionUI;   }
    inline GLFrustum *GetFontFrustum() { return &projectionFont; }
    inline int GetCurrentPass()        { return currentPass;     }

    //some stuff depends on renderer being initialized
    inline bool FinishInit() { initDone = true; }
    inline bool isReady()    { return initDone; }
    
   
};


#endif
