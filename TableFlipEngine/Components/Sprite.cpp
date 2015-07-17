/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Sprite.h"
#include "Entity.h"

/*Sprite::Sprite ( Entity* ent, Material& mat )
{
    entity = ent;
    transform = entity->GetTransform();
    material = new Material(mat);
    Init();
}*/
//TODO needed for builder to work in its current state, need to add shared materials 
Sprite::Sprite ( Entity* ent, SpriteResource *resource, SpriteType type)
{
    spriteType = type;
    entity = ent;
    transform = entity->GetTransform();
    material = resource->material;  
    spriteResource = resource;
    numFrames = resource->getNumFrames();
    Init();
}

void Sprite::Init()
{
    
    curFrame = 0;
    numVerts = spriteResource->getNumVerts();
    vertPositions = spriteResource->getVertPositions();
    vertTexCoords = spriteResource->getVertTexcoords(); 
}

Sprite::~Sprite()
{
    spriteResource->RemoveRef();
    //delete[] positions;
   // delete[] texCoords;
}

void Sprite::PrepareMaterial()
{
    if (spriteResource->isTiled())
        return;
    if (material->GetAttributeLocation(ATR_POSITION0) < 0)
    {
        LogWarning("Sprite ATR_POSITION0 Is Not Registered");
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertPositions);  
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_POSITION0), 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    if (material->GetAttributeLocation(ATR_TEXCOORD0) < 0)
    {
       // LogWarning("Mesh NORMAL0 Attribute Is Not Registered");
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_TEXCOORD0));//tc location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_TEXCOORD0), 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
}

void Sprite::FinishMaterial()
{
    if (material->GetAttributeLocation(ATR_POSITION0) >= 0)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0));
    if (material->GetAttributeLocation(ATR_TEXCOORD0) >= 0)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_TEXCOORD0));
}
