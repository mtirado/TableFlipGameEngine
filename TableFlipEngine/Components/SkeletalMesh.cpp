/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "SkeletalMesh.h"
#include "AnimationResource.h"



SkinnedMesh::SkinnedMesh(Entity *ent, Transform3d *trans, AnimatedModelResource *mesh, Material *_material)
{
    entity = ent;
    transform = trans;
    animatedMesh = mesh;
    numMeshes = animatedMesh->numMeshes;
    numBones = animatedMesh->numBones;
    meshData = 0;
    bones = 0;
    //just making sure...
    if (animatedMesh->numMeshes <= 0 || animatedMesh->numBones <= 0)
    {
        LogError("SkinnedMesh() numMeshes or numBones is <= 0");
        return;
    }
    meshData = new Md5Mesh[animatedMesh->numMeshes];
    bones = new Md5Bone[animatedMesh->numBones];
    memcpy(bones, animatedMesh->bones, sizeof(Md5Bone) * numBones);
    material = _material;
    //duplicate the mesh data but create new local copies of verts and normals that we will deform
    for (int i = 0; i < animatedMesh->numMeshes; i++)
    {
        memcpy(&meshData[i], &animatedMesh->meshes[i], sizeof(Md5Mesh));
        meshData[i].material = material; //NOTE still using one material for the whole animated mesh
       // material->RegisterProjectionMatrix("ProjectionMatrix");
       // material->RegisterViewMatrix("ViewMatrix");
        
        meshData[i].positions = new Vector3[meshData[i].numVerts];
        meshData[i].normals   = new Vector3[meshData[i].numVerts];
        
        glGenBuffers(1, &meshData[i].vertPositions);
        glGenBuffers(1, &meshData[i].vertNormals);
        //upload vert data in update pose
    }
    
    
    animatedMesh->AddRef();
    
    UpdatePose();
    
    //yep this is a HACK because blender uses a funky coordinate system
    transform->RotateRadians(DegToRad(90), DegToRad(270), 0.0f);
    transform->Update();
    
}

SkinnedMesh::~SkinnedMesh()
{
    if (meshData != 0) delete[] meshData;     
    if (bones != 0) delete[] bones;
    animatedMesh->RemoveRef();
}

int SkeletalMesh::GetBone(const char *name)
{
    for (int i = 0; i < numBones; i++)
        if (!strcmp(name, bones[i].name))
            return i;
        
    return -1;
}

void SkinnedMesh::UpdatePose()
{ 
    Vector3      vertPos, weightVert;
    unsigned int curWeight;
    Md5Bone        *bone;
    Md5Weight   *weight;

    for (unsigned int i = 0; i < animatedMesh->numMeshes; i++)
    {       
        for (unsigned int curVert = 0; curVert < meshData[i].numVerts; curVert++)
        {
            vertPos.x = 0.0f;
            vertPos.y = 0.0f;
            vertPos.z = 0.0f;

            //go through all the weights that affect the vert
            for (curWeight = 0; curWeight < meshData[i].verts[curVert].weightCount; curWeight++)
            {
                //get the weight and joint to work with
                weight = &meshData[i].weights[meshData[i].verts[curVert].startWeight + curWeight];
                bone = &bones[weight->index];

                //rotate joint orientation by weight position  to get position of vert
                //QuaternionRotatePoint(bone->rotation, weight->position, weightVert);
               
                weightVert = bone->rotation.RotatePoint(weight->position);
               
                vertPos.x += (bone->position.x + weightVert.x) * weight->bias;
                vertPos.y += (bone->position.y + weightVert.y) * weight->bias;
                vertPos.z += (bone->position.z + weightVert.z) * weight->bias;
                    
            }
            //m_pMeshes[i].pTexCoords[curVert].st.x = m_pMeshes[i].pVerts[curVert].fS;
            //m_pMeshes[i].pTexCoords[curVert].st.y = m_pMeshes[i].pVerts[curVert].fT;

            meshData[i].positions[curVert].x = vertPos.x;
            meshData[i].positions[curVert].y = vertPos.y;
            meshData[i].positions[curVert].z = vertPos.z;
        }
        
        //send verts
        glBindBuffer(GL_ARRAY_BUFFER, meshData[i].vertPositions);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_POSITION0)); //vertpos location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, meshData[i].numVerts * sizeof(Vector3), meshData[i].positions, GL_DYNAMIC_DRAW);
       
        

        //each vert just has face normal copied on it
        Vector3 normal;
        for (unsigned int curTri = 0; curTri < meshData[i].numTris; curTri++)
        {
            //dammit they're backwards...
            normal = 
            ComputeNormal(meshData[i].positions[meshData[i].tris[curTri].index[0]],
            meshData[i].positions[meshData[i].tris[curTri].index[1]],
            meshData[i].positions[meshData[i].tris[curTri].index[2]]);
            
            //got dang this is ugly
            meshData[i].normals[meshData[i].tris[curTri].index[0]] = normal;
            meshData[i].normals[meshData[i].tris[curTri].index[1]] = normal;
            meshData[i].normals[meshData[i].tris[curTri].index[2]] = normal;

        }
        
        //send normals 
        glBindBuffer(GL_ARRAY_BUFFER, meshData[i].vertNormals);
        glEnableVertexAttribArray(material->GetAttributeLocation(ATR_NORMAL0)); //normal location
        glVertexAttribPointer(material->GetAttributeLocation(ATR_NORMAL0), 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, meshData[i].numVerts * sizeof(Vector3), meshData[i].normals, GL_DYNAMIC_DRAW);
    }
}

