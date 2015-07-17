/* 
 * contact: mtirado418@gmail.com
 *
 * this is a heavily modified version of David Henry's md5 loader.
 * which can be found at http://tfc.duke.free.fr/coding/md5-specs-en.html
 * below is the copyright notice from the original files:
 *
 * md5mesh.c/md5anim.c -- md5mesh model loader + animation
 * last modification: aug. 14, 2007
 *
 * Doom3's md5mesh viewer with animation.  Mesh portion.
 * Dependences: md5model.h, md5anim.c.
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "AnimationResource.h"
#include "Renderer.h"
#include <stdio.h>
AnimatedModelResource::AnimatedModelResource()
{
}

AnimatedModelResource::~AnimatedModelResource()
{
     Destroy();
}

void AnimatedModelResource::Destroy()
{
    for (int i = 0; i < numMeshes; i++)
    {
        if (meshes[i].verts != 0) delete[] meshes[i].verts;
        if (meshes[i].tris != 0) delete[] meshes[i].tris;
        if (meshes[i].weights != 0) delete[] meshes[i].weights;       
    }
    
    if (bones != 0) delete[] bones;
    if (meshes != 0) delete[] meshes;

    bones = 0;
    meshes = 0;
}

//credit:  loading code is based on - David Henry - http://tfc.duke.free.fr/coding/md5-specs-en.html
bool AnimatedModelResource::LoadMd5(const char *file, ResourceLoadMemInfo *mem)
{
        if (!file)
            return false;
#if defined(__ANDROID__)
        if (!mem) return 0;
        FILE *fp = fmemopen(mem->mem, mem->size, "r");
#else
        FILE *fp = fopen(file, "r");
#endif
        //buffer for reading
        char buff[512]; //if 512 aint big enough, we got issues

        int                      version;
        unsigned int curMesh = 0;
        unsigned int vertIndex, triIndex, weightIndex, weightJoint;
        float            vert_s, vert_t, weightBias, weightPosX, weightPosY, weightPosZ, temp;
        int                      vert_start, vert_count;
        int                      triData[3];
        //start loader off on version
        unsigned int loaderState = LOADER_STATE_VERSION;

        //since we cant really gaurantee that the whitespace will be the same across
        //all md5 models, this is going to be one huge loop, hoooray

        //keep on crankin' till we hit the end of file.
        while (!feof(fp))
        {
            //read a line
            fgets (buff, sizeof(buff), fp);
            switch(loaderState)
            {
                case LOADER_STATE_VERSION:
                {
                    //check for version
                    if (sscanf(buff, "MD5Version %d", &version) == 1)
                    {
                        //we are only supporting version 10 for now (doom 3)
                        if (version != 10)
                            return false;
                        
                        
                        //load the default material
                        if (!material.LoadShader("Data/Shaders/Textured.shader"))
                        {
                            LogError("error loading Animated Mesh Material");
                            return false;
                        }
                        loaderState = LOADER_STATE_NUMJOINTS;
                    }
                    break;
                }

                case LOADER_STATE_NUMJOINTS:
                {
                    //check for num joints
                    if (sscanf(buff, "numJoints %d", &numBones) == 1)
                    {
                        //allocate memory for our models joints
                        bones = new Md5Bone[numBones];
                        loaderState = LOADER_STATE_NUMMESHES;
                    }
                    break;
                }

                case LOADER_STATE_NUMMESHES:
                {
                    if (sscanf(buff, "numMeshes %d", &numMeshes) == 1)
                    {
                        meshes = new Md5Mesh[numMeshes];
                        loaderState = LOADER_STATE_JOINTS;
                    }
                    break;
                }

                case LOADER_STATE_JOINTS:
                {
                    if (!strncmp (buff, "joints {", 8))
                    {       
                        //load in each joint
                        for (unsigned int i = 0; i < numBones; i++)
                        {
                            fgets (buff, sizeof (buff), fp);
                            //make sure we read in 8 elements, if not soemthing is FUCKED!!
                            if (sscanf (buff, "%s %d ( %f %f %f ) ( %f %f %f )", &bones[i].name, &bones[i].parent,
                                    &bones[i].position.x, &bones[i].position.y, &bones[i].position.z,
                                    &bones[i].rotation.x, &bones[i].rotation.y, &bones[i].rotation.z) == 8)
                            {
                                //compute w component
                                temp = 1.0f - 
                                (bones[i].rotation.x * bones[i].rotation.x) - 
                                (bones[i].rotation.y * bones[i].rotation.y) - 
                                (bones[i].rotation.z * bones[i].rotation.z);

                                if (temp < 0.0f)
                                    bones[i].rotation.w = 0.0f;
                                else
                                    bones[i].rotation.w = -sqrt (temp);
                            }
                        }
                        loaderState = LOADER_STATE_MESHES;
                    }


                    break;
                }


                case LOADER_STATE_MESHES:
                {
                    if (!strncmp (buff, "mesh {", 6))
                    {
                        loaderState = LOADER_MESH_SHADER;
                        //loop untill we hit end of mesh block, or eof
                        while ((buff[0] != '}') && !feof(fp))
                        {
                            fgets (buff, sizeof (buff), fp);

                            switch(loaderState)
                            {
                                //TODO load shader file 
                                case LOADER_MESH_SHADER:
                                {
                                    //maybe we can use this somehow??
                                    // if (sscanf(buff, " shader %s", m_pMeshes[curMesh].szShaderFile) == 1)
                                    
                                    //load default material
                                    //TODO add per mesh materials plzzz
                                    meshes[curMesh].SetMaterial(material);
                                    
                                    loaderState = LOADER_MESH_VERTS;
                                    break;
                                }

                                //load in numverts  and then every vert
                                case LOADER_MESH_VERTS:
                                {
                                    if (sscanf(buff, " numverts %d", &meshes[curMesh].numVerts) == 1)
                                    {
                                        //allocate the verts
                                        meshes[curMesh].verts = new Md5Vert[meshes[curMesh].numVerts];

                                        //loop through all the verts, and slap those fuckers in the mesh
                                        for (unsigned int i = 0; i < meshes[curMesh].numVerts; i++)
                                        {
                                            fgets (buff, sizeof (buff), fp);
                                            if (sscanf (buff, " vert %d ( %f %f ) %d %d", &vertIndex,
                                                            &vert_s, &vert_t, &vert_start, &vert_count) == 5)
                                            {
                                                //set the vert data
                                                meshes[curMesh].verts[vertIndex].s = vert_s;
                                                meshes[curMesh].verts[vertIndex].t = vert_t;
                                                meshes[curMesh].verts[vertIndex].startWeight = vert_start;
                                                meshes[curMesh].verts[vertIndex].weightCount = vert_count;

                                            }

                                        }
                                        loaderState = LOADER_MESH_TRIS;
                                    }
                                    break;
                                }
                                //triangle vert indices
                                case LOADER_MESH_TRIS:
                                {
                                    if (sscanf(buff, " numtris %d", &meshes[curMesh].numTris) == 1)
                                    {
                                        //allocate triangle indices
                                        meshes[curMesh].tris = new Md5Tri[meshes[curMesh].numTris];
                                        for (unsigned int i = 0; i < meshes[curMesh].numTris; i++)
                                        {
                                            fgets (buff, sizeof (buff), fp);
                                            if (sscanf (buff, " tri %d %d %d %d", &triIndex,
                                                    &triData[0], &triData[1], &triData[2]) == 4)
                                            {
                                                //set the vert data
                                                meshes[curMesh].tris[triIndex].index[0] = triData[0];
                                                meshes[curMesh].tris[triIndex].index[1] = triData[1];
                                                meshes[curMesh].tris[triIndex].index[2] = triData[2];
                                            }
                                        }
                                        loaderState = LOADER_MESH_WEIGHTS;
                                    }
                                    break;
                                }

                                case LOADER_MESH_WEIGHTS:
                                {
                                    if (sscanf(buff, " numweights %d", &meshes[curMesh].numWeights) == 1)
                                    {
                                        meshes[curMesh].weights = new Md5Weight[meshes[curMesh].numWeights];
                                        for (unsigned int i = 0; i < meshes[curMesh].numWeights; i++)
                                        {
                                            fgets (buff, sizeof (buff), fp);
                                            if (sscanf (buff, " weight %d %d %f ( %f %f %f )",
                                                                &weightIndex, &weightJoint, &weightBias,
                                                                &weightPosX, &weightPosY, &weightPosZ) == 6)
                                            {
                                                //set the weight data
                                                meshes[curMesh].weights[weightIndex].index = weightJoint;
                                                meshes[curMesh].weights[weightIndex].bias = weightBias;
                                                meshes[curMesh].weights[weightIndex].position.x = weightPosX;
                                                meshes[curMesh].weights[weightIndex].position.y = weightPosY;
                                                meshes[curMesh].weights[weightIndex].position.z = weightPosZ;

                                            }
                                        }

                                        loaderState = LOADER_STATE_MESHES;
                                    }
                                    break;
                                }
                            }
                        }
                        loaderState = LOADER_STATE_MESHES;
                        curMesh++;

                    }
                    break;
                }

                default:
                        break;
            }
        }
        
        if (loaderState != LOADER_STATE_MESHES)
        {
            LOGOUTPUT << "Md5 Mesh Did not Load Properly : " << file;
            LogError();
            Destroy();
            return false;
        }

        //setup static GL arrays for indices and texcoords
        int vertIter = 0;
        for (int i = 0; i < numMeshes; i++) //TODO add multiple mesh loading...
        {
            // normals = new Vector3[numTris]; //per face for now...
            meshes[i].indices = new GLuint[meshes[i].numTris * 3];
            // positions = new Vector3[numVerts];
            meshes[i].texCoords = new Vector2[meshes[i].numVerts];

            //generate the index array - in reverse becaue front faces are CCW
            for (unsigned int triIter = 0, indexIter = 0; triIter < meshes[i].numTris; triIter++)
            {
                for (vertIter = 2; vertIter >= 0; vertIter--, indexIter++)
                    meshes[i].indices[indexIter] = meshes[i].tris[triIter].index[vertIter];
                    
            }
                       
            glGenBuffers(1, &meshes[i].triIndices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].triIndices);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes[i].numTris * 3 * sizeof(unsigned int), meshes[i].indices, GL_STATIC_DRAW);
            
            //gather texcoords
            for (unsigned int curVert = 0; curVert < meshes[i].numVerts; curVert++)
            {
                meshes[i].texCoords[curVert].x = meshes[i].verts[curVert].s;
                meshes[i].texCoords[curVert].y = 1.0 - meshes[i].verts[curVert].t;
            }
            
            //TODO this may be bad... attrib locations may be different with different materials
            //like if one entity has a different material than the first one that was loaded as resource...
            //figure out a solution prreaze
            glGenBuffers(1, &meshes[i].vertTexCoords);
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].vertTexCoords);
            glEnableVertexAttribArray(material.GetAttributeLocation(ATR_TEXCOORD0));
            glVertexAttribPointer(material.GetAttributeLocation(ATR_TEXCOORD0), 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBufferData(GL_ARRAY_BUFFER, meshes[i].numVerts * sizeof(Vector2), meshes[i].texCoords, GL_STATIC_DRAW);
        }


        //FileName = file;
        return true;
}



//***** anim resource
AnimationResource::AnimationResource(int _priority)
{
    priority = _priority;
    keyframes = 0; numBones = 0; frameTime = 0; frameRate = 0; 
}

AnimationResource::~AnimationResource()
{
    if (keyframes && numBones)
    {
        for (int i = 0; i < numFrames; i++)
        {
            delete keyframes[i];
        }
        delete[] keyframes;
        delete[] boundingBoxes;
    }
}


//load animation
bool AnimationResource::LoadAnimation (const char* filename, ResourceLoadMemInfo *mem)
{
    FILE *fp = 0;
    char buff[512];

    //m_pModel = model;

    Md5BoneInfo *jointInfo = 0;
    Md5Bone *pJointBaseFrame;

    float *frameData = 0;
    int animatedComponents;
    int version;
    int loaderState = LOADER_STATE_VERSION;
    int frameIndex;

#if defined(__ANDROID__)
        if (!mem) return 0;
        fp = fmemopen(mem->mem, mem->size, "r");
#else
        fp = fopen(filename, "r");
#endif
        
    if (!fp)
        return false;

    while (!feof(fp))
    {
        //read a line.
        fgets (buff, sizeof(buff), fp);

        switch(loaderState)
        {
            case LOADER_STATE_VERSION:
            {
                //check for version
                if (sscanf(buff, "MD5Version %d", &version) == 1)
                {
                    //we are only supporting version 10 (doom 3)
                    if (version != 10)
                        return false;

                    loaderState = LOADER_ANIM_NUMFRAMES;
                }
                break;
            }

            case LOADER_ANIM_NUMFRAMES:
            {
                if (sscanf (buff, " numFrames %d", &numFrames) == 1)
                {
                    if (numFrames <= 0)
                        return false;

                    //allocate the keyframes and bounding boxes
                    keyframes = new Md5Bone * [numFrames];
                    boundingBoxes = new Md5BoundingBox[numFrames];

                    loaderState = LOADER_STATE_NUMJOINTS;
                }
                break;
            }

            case LOADER_STATE_NUMJOINTS:
            {
                if (sscanf (buff, " numJoints %d", &numBones) == 1)
                {
                    if (numBones <= 0)
                        return false;

                    //allocate base frame
                    pJointBaseFrame = new Md5Bone[numBones];
                    
                    //allocate "buffer" for crafting keyframes
                    jointInfo = new Md5BoneInfo[numBones];

                    //allocate the memory for the joints in every frame
                    for (int i = 0; i < numFrames; i++)
                        keyframes[i] = new Md5Bone[numBones];
                    
                    loaderState = LOADER_ANIM_FRAMERATE;
                }

                break;
            }

            case LOADER_ANIM_FRAMERATE:
            {
                if (sscanf (buff, " frameRate %d", &frameRate) == 1)
                {              
                    if (frameRate <= 0.0f)
                        return false;

                    frameTime = (float)(1000.0f / frameRate);
                    loaderState = LOADER_ANIM_COMPONENTS;
                }
                break;
            }

            case LOADER_ANIM_COMPONENTS:
            {
                if (sscanf (buff, " numAnimatedComponents %d", &animatedComponents) == 1)
                {
                    //allocate memory for frame data
                    frameData = new float[animatedComponents];
                    loaderState = LOADER_ANIM_HIERARCHY;
                }
                break;
            }

            case LOADER_ANIM_HIERARCHY:
            {
                if (strncmp (buff, "hierarchy {", 11) == 0)
                {
                    //we dont care about joint names at the moment
                    //char throwMeOut[64];
                    for (int i = 0; i < numBones; i++)
                    {
                        //read in a line for each joint
                        fgets(buff, sizeof(buff), fp);

                        //sort out joint info
                        sscanf(buff, " %s %d %d %d", jointInfo[i].name,
                                        &jointInfo[i].parent, &jointInfo[i].flags, &jointInfo[i].startIndex);
                    }
                    loaderState = LOADER_ANIM_BOUNDS;
                }

                break;
            }

            case LOADER_ANIM_BOUNDS:
            {
                if (strncmp(buff, "bounds {", 8) == 0)
                {       
                    for (int i = 0; i < numFrames; i++)
                    {
                        fgets(buff, sizeof(buff), fp);
                        
                        sscanf(buff, " ( %f %f %f ) ( %f %f %f )",
                        &boundingBoxes[i].min.x, &boundingBoxes[i].min.y, &boundingBoxes[i].min.z, 
                        &boundingBoxes[i].max.x, &boundingBoxes[i].max.y, &boundingBoxes[i].max.z);
                    }
                    loaderState = LOADER_ANIM_BASEFRAME;                    
                }
                break;
            }

            case LOADER_ANIM_BASEFRAME:
            {
                if (strncmp (buff, "baseframe {", 10) == 0)
                {
                    float temp;
                    //read in the base frame skeleton
                    for (int i = 0; i < numBones; i++)
                    {
                        fgets(buff, sizeof(buff), fp);
                        if (sscanf(buff, " ( %f %f %f ) ( %f %f %f )",
                        &pJointBaseFrame[i].position.x, &pJointBaseFrame[i].position.y,
                        &pJointBaseFrame[i].position.z, &pJointBaseFrame[i].rotation.x,
                        &pJointBaseFrame[i].rotation.y, &pJointBaseFrame[i].rotation.z) == 6)
                            {
                                // compute w component
                                temp = 1.0f - 
                                (pJointBaseFrame[i].rotation.x * pJointBaseFrame[i].rotation.x) - 
                                (pJointBaseFrame[i].rotation.y * pJointBaseFrame[i].rotation.y) - 
                                (pJointBaseFrame[i].rotation.z * pJointBaseFrame[i].rotation.z);

                                if (temp < 0.0f)
                                    pJointBaseFrame[i].rotation.w = 0.0f;
                                else
                                    pJointBaseFrame[i].rotation.w = -sqrt (temp);
                            }
                    }

                    loaderState = LOADER_ANIM_KEYFRAME;
                }                       
                break;
            }

            case LOADER_ANIM_KEYFRAME:
            {
                if (sscanf (buff, " frame %d", &frameIndex) == 1)
                {
                    //read in all of the frame data
                    for (int i = 0; i < animatedComponents; i++)
                    {
                        fscanf(fp, "%f", &frameData[i]);
                    }

                    //after we get the frame data, build the keyframe out of it
                    BuildKeyframe(jointInfo, pJointBaseFrame, frameData, numBones, keyframes[frameIndex]);
                }       
                break;
            }


        default:
            return false;
            break;
        }
    }

    if (frameData) delete[] frameData;
    if (pJointBaseFrame) delete[] pJointBaseFrame;
    if (jointInfo) delete[] jointInfo;

    FileName = filename;
    return true;
}


void AnimationResource::BuildKeyframe ( const Md5BoneInfo* boneInfo, Md5Bone* baseFrame, const float* frameData, int numBones, Md5Bone* outKeyframe )
{       
    Md5Bone *baseJoint;
    Vector3 animPos;
    Quaternion  animOrient;
    int indexOffset;
    float temp;
    Md5Bone *thisJoint;
    Md5Bone *parent;
    Vector3 rotatedPos;
    //go through all joints
    for (int i = 0; i < numBones; i++)
    {
        baseJoint = &baseFrame[i];

        //copy base joint orientation and position into our temp variables
        animPos = baseJoint->position;
        animOrient = baseJoint->rotation;

        indexOffset = 0;
        //check the joints flags to see what values need to be swapped out of frame data
        if (boneInfo[i].flags & 1)
        {
                animPos.x = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }
        if (boneInfo[i].flags & 2)
        {
                animPos.y = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }
        if (boneInfo[i].flags & 4)
        {
                animPos.z = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }
        if (boneInfo[i].flags & 8)
        {
                animOrient.x = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }
        if (boneInfo[i].flags & 16)
        {
                animOrient.y = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }
        if (boneInfo[i].flags & 32)
        {
                animOrient.z = frameData[boneInfo[i].startIndex + indexOffset];
                indexOffset++;
        }

        //compute w
        temp = 1.0f - 
        (animOrient.x * animOrient.x) - 
        (animOrient.y * animOrient.y) - 
        (animOrient.z * animOrient.z);

        if (temp < 0.0f)
            animOrient.w = 0.0f;
        else
            animOrient.w = -sqrt (temp);
                                
        thisJoint = &outKeyframe[i];
        thisJoint->parent = boneInfo[i].parent;
        //dont care about name, but if i did.. copy it in here.
        strcpy(thisJoint->name, boneInfo[i].name);

        if (thisJoint->parent < 0)
        {       
            //no parent
            thisJoint->position = animPos;
            thisJoint->rotation = animOrient;
        }
        else
        {
            //has parent.. get this joint in the right position / orientation
            parent = &outKeyframe[thisJoint->parent];

            //apply parents transformations
            rotatedPos = parent->rotation.RotatePoint(animPos);
            thisJoint->position.x = rotatedPos.x + parent->position.x;
            thisJoint->position.y = rotatedPos.y + parent->position.y;
            thisJoint->position.z = rotatedPos.z + parent->position.z;
            
            thisJoint->rotation = parent->rotation * animOrient;
            
           // thisJoint->position = baseJoint->position;
           // thisJoint->rotation = baseJoint->rotation;
            thisJoint->rotation.Normalize();
        }               
    }
}


