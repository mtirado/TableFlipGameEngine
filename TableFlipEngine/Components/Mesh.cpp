/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#include "Components.h"
#include "Renderer.h"
#include "AnimationResource.h"
Mesh::Mesh(Entity *ent, Transform3d *trans, ModelResource *mdl, Material &mat, bool indexed, bool dynamic) 
{
    hidden = false;
    entity = ent;
    transform = trans;
    isIndexed = indexed;
    isDynamic = dynamic;
    model = 0;
    material = 0;
    numVerts = 0;
    barycentricPositions = 0;
    
    if (!mdl){ LogError("No Model to attach Mesh to!"); return; }
    
    //should invoke copy constructor...
    material = new Material(mat);
    model = mdl; mdl->AddRef(); numVerts = mdl->numVerts; 
    vertPositions = mdl->vertPositions;
    vertNormals = mdl->vertNormals;
    vertTexCoords = mdl->vertTexCoords;

    if(material->GetAttributeLocation(ATR_BARYCENTRIC) >= 0)
        barycentricPositions = model->GetBarycentric();

    //HACK
       
    if (strcmp(material->GetShaderName(), "QuadishGrid.shader") == 0)
    {
        material->RegisterUniform("whRatio", SV_FLOAT, &model->widthToHeightAspect);
    }
    
}

Mesh::Mesh(Entity *ent, Transform3d *trans, ModelResource *mdl, Material *mat, bool indexed, bool dynamic) 
{
    hidden = false;
    entity = ent;
    transform = trans;
    isIndexed = indexed;
    isDynamic = dynamic;
    model = 0;
    material = 0;
    numVerts = 0;
    barycentricPositions = 0;
    if (!mdl){ LogError("No Model to attach Mesh to!"); return; }
    
    //should invoke copy constructor...
    material = mat;
    model = mdl; mdl->AddRef(); numVerts = mdl->numVerts; 
    vertPositions = mdl->vertPositions;
    vertNormals = mdl->vertNormals;
    vertTexCoords = mdl->vertTexCoords;

    if(material->GetAttributeLocation(ATR_BARYCENTRIC) >= 0)
        barycentricPositions = model->GetBarycentric();

    //HACK
    if (strcmp(material->GetShaderName(), "Data/Shaders/QuadishGrid.shader") == 0)
    {
        material->RegisterUniform("whRatio", SV_FLOAT, &model->widthToHeightAspect);
    }
}

 Mesh::~Mesh() 
 { 
     
     if(model) 
         model->RemoveRef();
     if (material) 
         delete material;
     
}

//TODO   oh yeah... material changes need to happen...
void Mesh::SetMaterial(Material &mat)
{
    delete material;
    material = new Material(mat);
}

void Mesh::PrepareMaterial()
{
   
    if (material->GetAttributeLocation(ATR_POSITION0) < 0)
    {
        LogWarning("Mesh POSITION0 Attribute Is Not Registered");
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertPositions);  
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
   

   if (material->GetAttributeLocation(ATR_NORMAL0) < 0)
    {
        //LogWarning("Mesh NORMAL0 Attribute Is Not Registered");
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_NORMAL0));//normal location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_NORMAL0), 3, GL_FLOAT, GL_FALSE, 0, 0);
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

    if (barycentricPositions)
    {
        glBindBuffer(GL_ARRAY_BUFFER, barycentricPositions);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_BARYCENTRIC));//tc location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_BARYCENTRIC), 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    else
    {
        // LogWarning("Mesh NORMAL0 Attribute Is Not Registered");
    }
}

void Mesh::FinishMaterial()
{
    if (material->GetAttributeLocation(ATR_POSITION0) >= 0)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0));
    if (material->GetAttributeLocation(ATR_NORMAL0) >= 0)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_NORMAL0));
    if (material->GetAttributeLocation(ATR_TEXCOORD0) >= 0)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_TEXCOORD0));
    if (barycentricPositions)
        glDisableVertexAttribArray(material->GetAttributeLocation(ATR_BARYCENTRIC));
}



