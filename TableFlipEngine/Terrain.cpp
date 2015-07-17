/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

//TODO Flat terrain is broken...  doh!

#include "Terrain.h"
#include "Renderer.h"
#include <stdio.h>
Terrain::Terrain()
{
    collisionMesh  = 0;
    fractalGain = 1.23646;
    fractalLucanarity = 0.8251;
    fractalOctaves = 17;
    fractalFrequency = 0.105;
    isFlat = true;
    vertices = 0;
    normals = 0;
    indices = 0;
    numIndices = 0;
    fractalMultiplier = 4;
    heightMap.power = 150;
    vertexBuff = 0;
    normalBuff = 0;
    texcoordBuff = 0;
    texCoords = 0;
}

Terrain::~Terrain()
{
    gPhysics->RemoveStaticGeometry("XFTERRAIN");
    delete collisionMesh;
    if (texCoords)
        delete[] texCoords;
    if (heightMap.data)
        delete[] heightMap.data;
    if (vertices)
        delete[] vertices;
    if (normals)
        delete[] normals;
    if (indices)
        delete[] indices;
    if (vertexBuff)
        glDeleteBuffers(1, &vertexBuff);
    if (normalBuff)
        glDeleteBuffers(1, &normalBuff);
    if (texcoordBuff)
        glDeleteBuffers(1, &texcoordBuff);
     
}

bool Terrain::Init(float _radius)
{
    if (_radius > 0.0f)
    {
        radius = _radius;
        isFlat = false;
    }
    
    material.LoadShader("Data/Shaders/Textured.shader");
    material.RegisterViewMatrix("ViewMatrix");
    material.RegisterProjectionMatrix("ProjectionMatrix");
    material.RegisterUniform("WorldMatrix", SV_MAT4, transform.transform);
    material.RegisterUniform("invRotation", SV_MAT3, inverseTransform);

    Matrix4 tmp;
   
   // transform.position.z = 800.0f;
    
   //transform.RotateRadians(DegToRad(65), 0, 0.0);
    transform.Update();
    transform.GetTransform(tmp);
    MatrixInvert(inverseTransform, tmp);
    
    return true;
}

bool Terrain::LoadHeightmap ( const char* filename )
{
   
    if (heightMap.data)
    {
        LogError("Heightmap Already Loaded for this terrain");
        return false;
    }
    
    int channels; // im just going to assume RGB, no need for A really at this time.
    unsigned char *img = SOIL_load_image(filename, &heightMap.width, &heightMap.height, &channels, 0);
    
    if (img == 0)
    {
        LOGOUTPUT << "Error Loading Terrain Heightmap: " << filename;
        LogError();
        return false; //couldnt load?
    }
    
    if (channels > 3 || channels < 3)
    {
        LOGOUTPUT << "Terrain Heightmap: " << filename << " - Must be 3 channel R8G8B8";
        LogError();
        return false;
    }
    
    //fractal multiplier sets how many levels deep we procedurally generate, as opposed to sample
  //  heightMap.height *= fractalMultiplier;
   // heightMap.width *= fractalMultiplier;
    unsigned int count = (heightMap.width ) * (heightMap.height );
    //we need to sum the rgb channels
    heightMap.data = new unsigned short[count];
    
    unsigned int component = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        heightMap.data[i] = img[component++];
        heightMap.data[i] += img[component++];
        heightMap.data[i] += img[component++];
    }
    
    //dont need this anymore
    SOIL_free_image_data(img);
    
    //765 tall if 8bpp
    
    //height data loaded, create the mesh!
    numVerts = count + 2; // +1 for south pole
    vertices = new Vector3[numVerts];
    normals = new Vector3[numVerts]; //vert normals
    //texCoords = new Vector3[numVerts];
    
    unsigned int southPole = numVerts - 1;
    unsigned int northPole = numVerts - 2;
    normals[northPole] = Vector3(0.0f, 0.0f, -1.0f);
    normals[southPole] = Vector3(0.0f, 0.0f, 1.0f);
    
    unsigned int cursor = 0;
    for (unsigned int y = 0; y < heightMap.height; y++)
    {
        for (unsigned int x = 0; x < heightMap.width; x++)
        {
            vertices[cursor] = Vector3(y, 0.0f, x);
            normals[cursor] =  cursor < numVerts / 2 ?  Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
            cursor++;
        }
    }
     
    //numIndices = ((heightMap.height -1  ) * ((heightMap.width) * 2)) + ((heightMap.height -1) * 4) -2;//+ 2;
  //  if (!isFlat)
   //     LoadMapWrapped(); //loads the heightmap and welds all the seams together
  //  else
        LoadMap();  //FIXME - yep its busted. couldnt decide if i want smoothed tri strips or tris with per face normals
   
    Vector3 a, b, c, v1, v2, n;
    Vector3 last;
    unsigned int lastIdx;

    
              

    for (unsigned int i = 2; i < numIndices; )
    {
        a = vertices[indices[i-2]];
        b = vertices[indices[i-1]];
        c = vertices[indices[i]];

        v1 = a - c;
        v2 = b - c;
        
        n = v1.Cross(v2);
   
       
        normals[indices[i]] += n;
        normals[indices[i]].Normalize();
        
        //if first one
        normals[indices[i-1]] += n;
        normals[indices[i-1]].Normalize();
        normals[indices[i-2]] += n;
        normals[indices[i-2]].Normalize();
       
        lastIdx = indices[i];
        last = n;
        i++;

        normals[northPole] = Vector3(0.0f, 0.0f, -1.0f);
        normals[southPole] = Vector3(0.0f, 0.0f, 1.0f);
    }
    
  
    //send opengl our data
       
    glGenBuffers(1, &vertexBuff);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_POSITION0)); //vertpos location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalBuff);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_NORMAL0)); //normal location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_NORMAL0), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);
   
    glGenBuffers(1, &indexBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    return true;
}



void Terrain::LoadMap()
{
    unsigned int cursor = 0;
    numIndices = ((heightMap.height ) * ((heightMap.width ) * 2) + heightMap.height); //degenerates are 4 per row
    
    indices = new GLuint[numIndices];
  
    memset(indices, 0, sizeof(GLuint) * numIndices);

    //fill out indices
    //built for triangle strips, degenerate is the last in X, then restarts
    cursor = 0;    
    unsigned int y = 0;
    bool alternate = true;
        
    for (y = 0; y < heightMap.height; y++)
    {   //need  degenerate to flip polygon direction
        
        if (y > 0)
        {
            indices[cursor] = indices[cursor-1];
            cursor++;
        }
            
        if (alternate)
        {
            
            indices[cursor++] = ((y) * (heightMap.width));
            indices[cursor++] = ((y+1) * (heightMap.width));
            for (int x = 1; x < heightMap.width; x++ )
            {
                //going left to right
                indices[cursor++] = ((y) * (heightMap.width) )+ x;
                indices[cursor++] = ((y+1) * (heightMap.width) )+ x;
            }
        }
        else
        {
            
            //flipped, going right to left
            indices[cursor++] = ((y) * (heightMap.width) + heightMap.width - 1);
            indices[cursor++] = ((y+1) * (heightMap.width) + heightMap.width - 1);
            for (int x = heightMap.width - 2; x >= 0 ; x-- )
            {
                //going left to right
                indices[cursor++] = ((y) * (heightMap.width) )+ x;
                indices[cursor++] = ((y+1) * (heightMap.width) )+ x;
            }
        }  
        alternate = !alternate;   
    }
}



void Terrain::Draw()
{
   // transform.RotateRadians(0.0033, 0.0021, 0.0015f);
    transform.Update();
    Matrix4 tmp;
    Matrix4 hackyMatrix;
    transform.GetTransform(tmp);
    MatrixInvert(inverseTransform, tmp);
    
    
    glUseProgram(material.program);
    material.BindAttributes();
    material.SendUniformData();
    material.SetProjectionMatrix(Renderer::GetInstance()->camera->GetProjection());
    material.SetViewMatrix(Renderer::GetInstance()->camera->GetInverseView());

    transform.GetTransform(tmp);
    MatrixInvert(hackyMatrix, tmp);
    glUniform3fv(material.GetUniformLocation("lightDir") , 1, (GLfloat *)Renderer::GetInstance()->sunLight.GetInvDir());

    //
    //SendShadowData(iter);

    //send any registered uniforms
    material.SendUniformData();
    Matrix3 rotation;
    Matrix4ExtractRotation(rotation, hackyMatrix); //needs inverse rotation
    glUniformMatrix3fv(material.GetUniformLocation("invRotation"), 1, GL_FALSE, rotation);
                



    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuff);  
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_POSITION0)); //vertpos location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, 0);  


    glBindBuffer(GL_ARRAY_BUFFER, normalBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_NORMAL0));//normal location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_NORMAL0), 3, GL_FLOAT, GL_FALSE, 0, 0);

    /*glBindBuffer(GL_ARRAY_BUFFER, texcoordBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_BARYCENTRIC)); //vertpos location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_BARYCENTRIC), 3, GL_FLOAT, GL_FALSE, 0, 0);
*/


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuff);
  //  glDrawArrays(GL_TRIANGLES, 0, numVerts);
  //glPolygonMode(GL_FRONT, GL_POINT);
   // glPointSize(4.5f);

 //DRAW INDEXED
// num
       glDrawElements(
     GL_TRIANGLES,      // mode
     numIndices,    // count
     GL_UNSIGNED_INT,   // type
     (void*)0           // element array buffer offset
 );
    
  /*  glPolygonMode(GL_FRONT, GL_LINES);
    glPointSize(3.0f);
    glDrawElements(
     GL_TRIANGLE_STRIP,      // mode
     numIndices,    // count
     GL_UNSIGNED_INT,   // type
     (void*)0           // element array buffer offset
 );*/


    glDisableVertexAttribArray(material.GetAttributeLocation(ATR_POSITION0));
    glDisableVertexAttribArray(material.GetAttributeLocation(ATR_NORMAL0));
   // glDisableVertexAttribArray(material.GetAttributeLocation(ATR_BARYCENTRIC));

      float ns = 6.0f;


//   for (int i = 0; i < numVerts; i++)
//   {
//     glColor3f(1.0, 0.0, 0.0);
//     glBegin(GL_LINES);
//     glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z) ;//+ (normals[i].x * ns),
//     glVertex3f(vertices[i].x + (normals[i].x * ns), vertices[i].y + (normals[i].y * ns), vertices[i].z + (normals[i].z * ns));
//     glEnd();
//   }


    glUseProgram(0);

    //CheckGLErrors();
    
}




//creates the verts for a cube
void Terrain::GenerateCube(int subdivisions)
{

    //setup adjacency pointers
    
    SetupCubeAdjacency();
    
    //how many verts per row/col?
    vertsPerRow = 2;
    for (int i = 1; i < subdivisions; i++)
        vertsPerRow += vertsPerRow - 1;

    int num = vertsPerRow * vertsPerRow * 6;
    numVerts = num;
    //edges are duplicates.
    Vector3 *mesh = new Vector3[num];
    //texCoords = new Vector3[numVerts]; //3d vecotr for barycentric,m or 3d coords


    

    int face = 0;
    double vy, vx;
    double uscale = vertsPerRow - 1.0;  //0 based array, needs -1
    unsigned int cur = 0;
    //front
    face = 0;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            
            if (y == 0) //south edge
                front.south.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                front.north.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                front.east.push_back(cur);
            else if ( (x+1) % vertsPerRow == 0) //east
                front.west.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            vx *= -1.0;
            mesh[cur++] = Vector3(vx, vy, 1.0);
            
        }
    }
    
    
    //right
    face++;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            if (y == 0) //south edge
                right.south.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                right.north.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                right.east.push_back(cur);//NOTE all this e/w shit is backwards because -z is forward
            else if ( (x+1) % vertsPerRow == 0) //east
                right.west.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            mesh[cur++] = Vector3(1.0, vy, vx);
        }
    }
    //back
    face++;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            if (y == 0) //south edge
                back.south.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                back.north.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                back.east.push_back(cur);
            else if ( (x+1) % vertsPerRow == 0) //east
                back.west.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            mesh[cur++] = Vector3(vx, vy, -1.0);
        }
    }
    //left
    face++;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            if (y == 0) //south edge
                left.south.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                left.north.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                left.east.push_back(cur);
            else if ( (x+1) % vertsPerRow == 0) //east
                left.west.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            vx *= -1.0;
            mesh[cur++] = Vector3(-1.0, vy, vx);
        }
    }
    //top
    face++;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            if (y == 0) //south edge NOTE opengl z backwards to common sense
                top.north.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                top.south.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                top.west.push_back(cur);
            else if ( (x+1) % vertsPerRow == 0) //east
                top.east.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            mesh[cur++] = Vector3(vx, 1.0, vy);
        }
    }
    //bottom
    face++;
    for (int y = 0; y < vertsPerRow; y++)
    {
        vy = ( (double)y / (double)uscale * 2.0) - 1.0;
        for (int x = 0; x < vertsPerRow; x++)
        {
            if (y == 0) //south edge
                bottom.south.push_back(cur);
            else if (y == vertsPerRow -1) //north edge
                bottom.north.push_back(cur);

            if (x % vertsPerRow == 0) //west edge
                bottom.west.push_back(cur);
            else if ( (x+1) % vertsPerRow == 0) //east
                bottom.east.push_back(cur);
            
            vx = ( (double)x / (double)uscale * 2.0) - 1.0;
            vx *= -1.0;
            mesh[cur++] = Vector3(vx, -1.0, vy);
        }
    }


    
    
    //make indices NOTE triangles, not tri strips
    num = (vertsPerRow-1) * (vertsPerRow-1) * 6 * 6;
    GLuint *ind = new GLuint[num];
    indices = ind;
    numIndices = num;
    cur = 0;
    //front
    face = 0;
    int maxIdx = 0;
    for(face = 0; face < 6; face++)
    {
        for (int y = 0; y < vertsPerRow - 1; y++)
        {
            for (int x = 0; x < vertsPerRow - 1; x++)
            {
                GLuint idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + (y * vertsPerRow) + x;
                idx = ind[cur -1];
                if (idx > maxIdx)
                    maxIdx = idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + ((y + 1) * vertsPerRow) + x;
                idx = ind[cur -1];
                if (idx > maxIdx)
                    maxIdx = idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + (y * vertsPerRow) + (x + 1);
                idx = ind[cur -1];
                if (idx > maxIdx)
                    maxIdx = idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + ((y + 1) * vertsPerRow) + x;
                idx = ind[cur -1];
                if (idx > maxIdx)
                    maxIdx = idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + ((y + 1) * vertsPerRow) + (x + 1);
                idx = ind[cur-1];
                if (idx > maxIdx)
                    maxIdx = idx;
                ind[cur++] = (face * (vertsPerRow * vertsPerRow)) + (y * vertsPerRow) + (x + 1);
                idx = ind[cur -1];
                if (idx > maxIdx)
                    maxIdx = idx;
            }
        }
    }

    LOGOUTPUT << "Maximum index found: " << maxIdx;
    LogWarning();

    vertices = mesh;
}

//TODO  optimize, check if within range before calling hasIndx  -- if you ever plan on actually using this function.
bool Terrain::isEdgeIndex(int idx)
{
    if (left.hasIndex(idx))
        return true;
    else if (front.hasIndex(idx))
        return true;
    else if (back.hasIndex(idx))
        return true;
    else if (right.hasIndex(idx))
        return true;
    else if (top.hasIndex(idx))
        return true;
    else if (bottom.hasIndex(idx))
        return true;
    else
        return false;
}

void Terrain::SetupCubeAdjacency()
{
    front.N = &top;
    front.S = &bottom;
    right.N = &top;
    right.S = &bottom;
    back.N = &top;
    back.S = &bottom;
    left.N = &top;
    left.S = &bottom;

    front.W = &left; 
    front.E = &right;
    right.W = &front;
    right.E = &back;
    back.W = &right;
    back.E = &left;
    left.W = &back; 
    left.E = &front;

    top.N = &back;
    top.S = &front;
    top.E = &right;
    top.W = &left;
    bottom.N = &front;
    bottom.S = &back;
    bottom.E = &right;
    bottom.W = &left;

}

void Terrain::CalculateNormals()
{
    Vector3 a, b, c, v1, v2, n;
    int idxPerRow = (vertsPerRow -1)  * 6;
    int restart = idxPerRow;;
    for (unsigned int i = 2; i < numIndices; i++ )
    {
        if (i == restart)
        {
            restart += idxPerRow;
            i++; //increment 2 and try again
            continue;
        }
        //if its on an edge, we need to increment so we can average the edges in the next stage

        a = vertices[indices[i-2]];
        b = vertices[indices[i-1]];
        c = vertices[indices[i]];

        v1 = a - c;
        v2 = b - c;

        n = v1.Cross(v2);


        normals[indices[i]] += n;
        normals[indices[i]].Normalize();
        normals[indices[i-1]] += n;
        normals[indices[i-1]].Normalize();
        normals[indices[i-2]] += n;
        normals[indices[i-2]].Normalize();

//         normals[indices[i]] *=9;
//         normals[indices[i-1]] *= 9;
//         normals[indices[i-2]] *= 9;

    }

    Vector3 *c1;
    Vector3 *c2;
    Vector3 *p1;
    Vector3 *p2;

    Vector3 *prevFaceEdgeNormal;
    Vector3 *curFaceEdgeNormal;


    // *c1 *= 3;
            //TODO /s/tomorrow/someday ...   instead of doing it this way, when in above normal calculation,
            //check if its an edge vert, and if so incorporate the other edge neighbors into the calc.
            //it is never going to look "right" (tm) doing it this way!


    int p1Idx;
    int p2Idx;
    std::vector<int> *tempIndexArray = 0;

    for (int i = 0; i < vertsPerRow-1; i++)
    {
        //front
        tempIndexArray      = left.getEdgeIndices(&front);
        curFaceEdgeNormal   = &normals[(*front.getEdgeIndices(&left))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*3 + *curFaceEdgeNormal) / 4.0;
        *prevFaceEdgeNormal = *curFaceEdgeNormal;
        //front top
        tempIndexArray      = top.getEdgeIndices(&front);
        curFaceEdgeNormal   = &normals[(*front.getEdgeIndices(&top))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[vertsPerRow-1-i]];  //front x is inverted order
       // p1                  = &normals[(*tempIndexArray)[vertsPerRow-i]];
        p1Idx               = (*front.getEdgeIndices(&top))[i]-vertsPerRow;        //get previous vertically normal index
        p2Idx               = (*tempIndexArray)[vertsPerRow-1-i]-vertsPerRow;
        p1                  = &normals[p1Idx];
        p2                  = &normals[p2Idx];

        *curFaceEdgeNormal  = (*p1 + *p2)*2 + *curFaceEdgeNormal + *prevFaceEdgeNormal;
        curFaceEdgeNormal->Normalize();
         *prevFaceEdgeNormal  = *curFaceEdgeNormal;
       // *p1 += *curFaceEdgeNormal; *p1 /= 2.0;
        //*p2 += *prevFaceEdgeNormal; *p2 /= 2.0;
        //front botttom
        tempIndexArray      = bottom.getEdgeIndices(&front);
        curFaceEdgeNormal   = &normals[(*front.getEdgeIndices(&bottom))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];  //front x is inverted order
        p1                  = &normals[(*front.getEdgeIndices(&bottom))[i+1]];
        *curFaceEdgeNormal  = (*p1+ *curFaceEdgeNormal*2) / 3.0;
        *prevFaceEdgeNormal  = *curFaceEdgeNormal;
        //right
        tempIndexArray      = front.getEdgeIndices(&right);
        curFaceEdgeNormal   = &normals[(*right.getEdgeIndices(&front))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*3 + *curFaceEdgeNormal) / 4.0;
        *prevFaceEdgeNormal = *curFaceEdgeNormal;
        //right top
        tempIndexArray      = top.getEdgeIndices(&right);
        curFaceEdgeNormal   = &normals[(*right.getEdgeIndices(&top))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*2+ *curFaceEdgeNormal) / 3.0;
        *prevFaceEdgeNormal  = *curFaceEdgeNormal;
        //back
        tempIndexArray      = right.getEdgeIndices(&back);
        curFaceEdgeNormal   = &normals[(*back.getEdgeIndices(&right))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*3 + *curFaceEdgeNormal) / 4.0;
        *prevFaceEdgeNormal = *curFaceEdgeNormal;
        //back top
        tempIndexArray      = top.getEdgeIndices(&back);
        curFaceEdgeNormal   = &normals[(*back.getEdgeIndices(&top))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*2+ *curFaceEdgeNormal) / 3.0;
        *prevFaceEdgeNormal  = *curFaceEdgeNormal;
        //left
        tempIndexArray      = back.getEdgeIndices(&left);
        curFaceEdgeNormal   = &normals[(*left.getEdgeIndices(&back))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal /*+ *curFaceEdgeNormal*/);
        *prevFaceEdgeNormal = *curFaceEdgeNormal;
        //left top
        tempIndexArray      = top.getEdgeIndices(&left);
        curFaceEdgeNormal   = &normals[(*left.getEdgeIndices(&top))[i]];
        prevFaceEdgeNormal  = &normals[(*tempIndexArray)[vertsPerRow-1-i]];
        *curFaceEdgeNormal  = (*prevFaceEdgeNormal*2+ *curFaceEdgeNormal) / 3.0;
        *prevFaceEdgeNormal  = *curFaceEdgeNormal;

    }
}


bool Terrain::LoadTimeCube(int subdivisions, unsigned int numOctaves, unsigned int seed, double lucanarity, double persistence, bool staticCollision)
{

    
    LogInfo("Generate cube");
    GenerateCube(subdivisions);
    CheckGLErrors();
    normals = new Vector3[numVerts];
    
    Vector3 tmp;
     //calculate new normals
     
    Vector3 last;
    //now that all the data is loaded, we need to construct something openGL can use, from the indices
    for (int i = 0; i < numVerts; i++)
    {
        //vertices[i] = temp_vertices[vertexIndices[i]-1];
        normals[i] = Vector3(0,0,0);
        //normals[i]   = temp_normals[normalIndices[i]-1];
      //texCoords[i] = temp_uvs[uvIndices[i]-1];
    }

    Vector3 barys[3];
    barys[0] = Vector3(1.0, 0.0, 0.0);
    barys[1] = Vector3(0.0, 1.0, 0.0);
    barys[2] = Vector3(0.0, 0.0, 1.0);

    int b = 0;
    //make it spherical  TIME CUBE!!
    for (int i = 0; i < numVerts; i ++)
    {
        Vector3 pos = vertices[i];
        vertices[i].x = pos.x * sqrt( 1 - (pos.y * pos.y) / 2 - (pos.z * pos.z) / 2 + ((pos.y * pos.y)*(pos.z * pos.z)) / 3 );
        vertices[i].y = pos.y * sqrt( 1 - (pos.z * pos.z) / 2 - (pos.x * pos.x) / 2 + ((pos.z * pos.z)*(pos.x * pos.x)) / 3 );
        vertices[i].z = pos.z * sqrt( 1 - (pos.x * pos.x) / 2 - (pos.y * pos.y) / 2 + ((pos.x * pos.x)*(pos.y * pos.y)) / 3 );

       /* texCoords[i] = barys[b++];
        if (b > 2)
            b = 0;*/
    }

   
    SimplexNoise noise;
    noise.Seed(seed);
    double val;
    double amplitude;
    double freq = 0.1;

    if (lucanarity < 0.01)
        lucanarity = 0.01;
    if (persistence < 0.01)
        persistence = 0.01;
    if (numOctaves < 1)
        numOctaves = 1;

    //double lucanarity = 1.2;
    //double persistence = 1.13;


    for (int i = 0; i < numVerts; i ++)
    {
        freq = 1.0 / vertsPerRow;
        val = 0.0;
        amplitude = persistence;
        Vector3 v = vertices[i];
        for (int oct = 0; oct < numOctaves; oct++)
        {
            val += (1.01 + noise.noise(v.x, v.y, v.z)) * amplitude;
            v *= lucanarity;
            amplitude *= persistence;

        }
        vertices[i] *=  (val / (numOctaves*persistence) );

    }
    
    
 for (int i = 0; i < numVerts; i ++)
        vertices[i] *= 333.3;

    CalculateNormals();
//needsnormals to apply fractal noise in an outward direction.
    int curFace = 0;
    int x, y;
    x = y = 0;
    double unit;
    for (int i = 0; i < numVerts; i++)
    {
        if (y == (vertsPerRow) * (vertsPerRow))
            y = 0;
        if (x == vertsPerRow)
            x = 0;
        
        freq = fractalFrequency;//.0 / vertsPerRow;
        val = 0.0;
        amplitude = fractalGain;
        Vector3 v = vertices[i];
        for (int oct = 0; oct < fractalOctaves; oct++)
        {
            unit = 1.0 + (noise.noise(v.x*freq, v.y*freq, v.z*freq)) ;
            unit /= 2.0;
            val += unit* amplitude;//
            freq *= fractalLucanarity;
            amplitude *= fractalGain;

        }
        vertices[i] += normals[i] * val;
    }
    
    //recalc new normals
    //normalize and scale to radius
   /* for (int i = 0; i < numVerts; i++)
    {
        vertices[i].Normalize();
        vertices[i] *= radius;
    }*/
    
    CalculateNormals();

//  for (unsigned int i = 2; i < numIndices; i++ )
//     {
//         if (i == restart)
//         {
//             restart += idxPerRow;
//             i++; //increment 2 and try again
//             continue;
//         }
//         
//         //if its on an edge, we need to increment so we can average the edges in the next stage
// 
//         a = normals[indices[i-2]];
//         b = normals[indices[i-1]];
//         c = normals[indices[i]];
// 
//         n = a + b + c;
//         n.Normalize();
// 
//        // if (!isEdgeIndex(indices[i]))
//         {
//             normals[indices[i]] += n;
//             normals[indices[i]].Normalize();
//         }
//        //  if (!isEdgeIndex(indices[i-1]))
//         {
//             normals[indices[i-1]] += n;
//             normals[indices[i-1]].Normalize();
//         }
//        //  if (!isEdgeIndex(indices[i-2]))
//         {
//             normals[indices[i-2]] += n;
//             normals[indices[i-2]].Normalize();
//         }
// 
// 
//     }


    
    CheckGLErrors();
    LogInfo("Uploading GL data");
    //assign static buffers to be shared by all!
    glGenBuffers(1, &vertexBuff);
    glGenBuffers(1, &normalBuff);
   // glGenBuffers(1, &texcoordBuff);
   // glGenBuffers(1, &vertTexCoords);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_POSITION0)); //vertpos location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_POSITION0), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_NORMAL0));//normal location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_NORMAL0), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);

    /*glBindBuffer(GL_ARRAY_BUFFER, texcoordBuff);
    glEnableVertexAttribArray(material.GetAttributeLocation(ATR_BARYCENTRIC)); //bary location
    glVertexAttribPointer(material.GetAttributeLocation(ATR_BARYCENTRIC), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), texCoords, GL_STATIC_DRAW);
    */
    glGenBuffers(1, &indexBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);
    CheckGLErrors();

  /*  glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
    glEnableVertexAttribArray(2);//texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector2), texCoords, GL_STATIC_DRAW);*/


    //generate collision data
    if (staticCollision)
    {
        if (collisionMesh)
            delete collisionMesh;
        collisionMesh = new btTriangleMesh();
        for (int i = 0; i < numIndices; i += 3)
        {
            collisionMesh->addTriangle(btVector3(vertices[indices[i]].x, vertices[indices[i]].y, vertices[indices[i]].z),
                            btVector3(vertices[indices[i+1]].x, vertices[indices[i+1]].y, vertices[indices[i+1]].z),
                            btVector3(vertices[indices[i+2]].x, vertices[indices[i+2]].y, vertices[indices[i+2]].z),
                            true); // removes duplicates
        }

        gPhysics->AddStaticTriangleMesh("XFTERRAIN", Vector3(0.0, 0.0, 0.0), collisionMesh);
    }
    
   // fclose(file);
    //FileName = filename;
    return true;
}



void Terrain::Reload(int subdivisions, unsigned int numOctaves, unsigned int seed, double lucanarity, double persistence)
{

    bool collision = collisionMesh;
    LogInfo("Generating new 'stroid");
    delete[] vertices;
    vertices = 0;
    delete[] normals;
    normals = 0;
    delete[] indices;
    indices = 0;
    if (texCoords)
        delete[] texCoords;
    texCoords = 0;
    glDeleteBuffers(1, &vertexBuff);
    glDeleteBuffers(1, &normalBuff);
    glDeleteBuffers(1, &texcoordBuff);
    glDeleteBuffers(1, &indexBuff);

    if (collisionMesh)
    {
        gPhysics->RemoveStaticGeometry("XFTERRAIN");
        delete collisionMesh;
        collisionMesh = 0;
    }
    
    top.Clear();
    bottom.Clear();
    left.Clear();
    front.Clear();
    back.Clear();
    right.Clear();

    
    LoadTimeCube(subdivisions, numOctaves, seed, lucanarity, persistence, collision);
}
