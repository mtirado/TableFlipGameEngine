/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _SPRITE_H__
#define _SPRITE_H__

#include "Utilities.h"
#include "Materials.h"
//#include <Renderer.h>

enum SpriteType { SPRITE_TYPE_NORMAL= 0, SPRITE_TYPE_STENCILS, SPRITE_TYPE_CLIPPED, SPRITE_NUMTYPES };

class Sprite : public Engine::Component
{
    //this would be proper, as opposed to the material being PUBLIC TODO do the same in Mesh!!
    friend class Renderer;
    Sprite() { }; //dont do it!
private:
    SpriteResource *spriteResource;
    unsigned short numVerts;
    unsigned short numTris;
    //mesh data 
    //Vector2 *positions;
    //Vector2 *texCoords;
    //GL buffer id's
    unsigned short numFrames;
    unsigned short curFrame;
    //SpriteFrame *frames;
    GLuint vertPositions;
    GLuint vertTexCoords;
    bool animated; //animated texcoords
    SpriteType spriteType;
        
    void Init();
protected:
    Material *material;
public:
    
    //passing the reference will invoke copy constructor
   // Sprite(Entity *ent, Material &mat);
    ~Sprite();
    //but passing pointer, will just straight set the pointer, shader reference counting done through loadShader(see builder)
    Sprite(Entity *ent, SpriteResource *resource, SpriteType type);
    unsigned int getShaderId()    { return material->GetShaderId(); }
    int getNumVerts() { return numVerts; }
    Material *getMaterial() { return material; }
    unsigned int getFrame() { return curFrame; }
    void setFrame(unsigned int frame) { curFrame = frame; if (curFrame >= numFrames) curFrame = numFrames - 1; }
    virtual void PrepareMaterial();
    virtual void FinishMaterial();
    
};

class TiledSprite : public Sprite
{
private:
    TiledSprite();
};

#endif
