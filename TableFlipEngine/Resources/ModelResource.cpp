/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 



#include "Renderer.h"
#include "Resources.h"
#include <stdio.h>

//cube primitive data (blender obj)
static const int numCubeVerts = 8;
static const Vector3 cubeVerts[numCubeVerts] =
{
        Vector3( 0.5000000, -0.5000000, -0.5000000),
        Vector3( 0.5000000, -0.5000000,  0.5000000),
        Vector3(-0.5000000, -0.5000000,  0.5000000),
        Vector3(-0.5000000, -0.5000000, -0.5000000),
        Vector3( 0.5000000,  0.5000000, -0.5000000),
        Vector3( 0.5000000,  0.5000000,  0.5000000),
        Vector3(-0.5000000,  0.5000000,  0.5000000),
        Vector3(-0.5000000,  0.5000000, -0.5000000),
};
static const int numCubeTexCoords = 4;
static const Vector2 cubeTexCoords[numCubeTexCoords] =
{
        Vector2(0.000000, 0.000000),
        Vector2(1.000000, 0.000000),
        Vector2(1.000000, 1.000000),
        Vector2(0.000000, 1.000000),
};
static const int numCubeNormals = 7;
static const Vector3 cubeNormals[numCubeNormals] =
{
        Vector3(0.000000, -1.000000, 0.000000),
        Vector3(-0.000000, 1.000000, 0.000000),
        Vector3(1.000000, -0.000000, 0.000001),
        Vector3(-0.000000, -0.000000, 1.000000),
        Vector3(-1.000000, -0.000000, -0.000000),
        Vector3(0.000000, 0.000000, -1.000000),
        Vector3(1.000000, 0.000000, -0.000000)
};
static const int numCubeFaces = 12;
static const int numCubeIndices = 108;
static const int cubeFaceIndices[numCubeIndices] =
{
        1,1,1,2,2,1,3,3,1,
        5,1,2,8,2,2,7,3,2,
        1,1,3,5,2,3,6,3,3,
        2,1,4,6,2,4,3,4,4,
        3,1,5,7,2,5,4,4,5,
        5,1,6,1,2,6,4,3,6,
        4,4,1,1,1,1,3,3,1,
        6,4,2,5,1,2,7,3,2,
        2,4,7,1,1,7,6,3,7,
        6,2,4,7,3,4,3,4,4,
        7,2,5,8,3,5,4,4,5,
        8,4,6,5,1,6,4,3,6
};
/*   (-w, -h,  d),  # front-bottom-left     0
     ( w, -h,  d),  # front-bottom-right    1
     (-w,  h,  d),  # front-top-left        2
     ( w,  h,  d),  # front-top-right       3
     (-w, -h, -d),  # back-bottom-left      4
     ( w, -h, -d),  # back-bottom-right     5
     (-w,  h, -d),  # back-top-left         6
     ( w,  h, -d))  # back-top-right        7*/
//tristripped cube
float TS_SCALE = 0.5;
static const Vector3 tsCubeVerts[numCubeVerts] = {
    Vector3( -0.5*TS_SCALE, -0.5*TS_SCALE,  0.5*TS_SCALE),
    Vector3(  0.5*TS_SCALE, -0.5*TS_SCALE,  0.5*TS_SCALE),
    Vector3( -0.5*TS_SCALE,  0.5*TS_SCALE,  0.5*TS_SCALE),
    Vector3(  0.5*TS_SCALE,  0.5*TS_SCALE,  0.5*TS_SCALE),
    Vector3( -0.5*TS_SCALE, -0.5*TS_SCALE, -0.5*TS_SCALE),
    Vector3(  0.5*TS_SCALE, -0.5*TS_SCALE, -0.5*TS_SCALE),
    Vector3( -0.5*TS_SCALE,  0.5*TS_SCALE, -0.5*TS_SCALE),
    Vector3(  0.5*TS_SCALE,  0.5*TS_SCALE, -0.5*TS_SCALE),
};


/*strip_vertices = (v[7] + v[6] + v[3] + v[2] + v[0] + v[6] + v[4] +
                  v[7] + v[5] + v[3] + v[1] + v[0] + v[5] + v[4])*/
static const unsigned int numTSCubeIndices = 14;
static const int tsCubeIndices[numTSCubeIndices] = {
    7,6,3,2,0,6,4,
    7,5,3,1,0,5,4
};

ModelResource::~ModelResource()
{

     if (positions != 0) delete[] positions;
     if (normals != 0) delete[] normals;
     if (texCoords != 0) delete[] texCoords;
     if (collisionMesh != 0) delete collisionMesh;
     if (barycentric != 0) delete[] barycentric;

     glDeleteBuffers(1, &vertNormals);
     glDeleteBuffers(1, &vertPositions);
     glDeleteBuffers(1, &vertTexCoords);
     glDeleteBuffers(1, &barycentricPositions);

}

GLuint ModelResource::GetBarycentric()
{
    if (barycentricPositions)
        return barycentricPositions;


    if (numVerts % 3 != 0)
    {

        LogError("Numverts not divisible by 3, BAD MESH, cannot generate barycentric coordinates");
        return 0;
    }

    Vector2 tc;
    barycentric = new Vector3[numVerts];
    for (int i = 0; i < numVerts; i+=3)
    {
        barycentric[i]   = Vector3(1,0,0);
        barycentric[i+1] = Vector3(0,1,0);
        barycentric[i+2] = Vector3(0,0,1);
    }



    glGenBuffers(1, &barycentricPositions);
    glBindBuffer(GL_ARRAY_BUFFER, barycentricPositions);
    glEnableVertexAttribArray(0);//this probably doesnt matter
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), barycentric, GL_STATIC_DRAW);

    return barycentricPositions;
}


//its an obj loader, but stored in program memory, not on disk.
bool ModelResource::LoadCube(bool skybox)
{
    if (positions)
    {
        LogError("Trying to load cube into an already existing model?");
        return false; //something is already loaded here...
    }
    numVerts  =     numCubeIndices / 3;
    positions =     new Vector3[numVerts];
        normals   =     new Vector3[numVerts];
        texCoords =     new Vector2[numVerts];
    int vertIdx = 0;
    //now that all the data is loaded, we need to construct something openGL can use, from the indices
    if (!skybox)
    {
        for (int i = 0; i < numCubeFaces * 9; i+=3)//-1 because obj is not 0 based arrays
        {
            positions[vertIdx] = cubeVerts[cubeFaceIndices[i]-1];
            texCoords[vertIdx] = cubeTexCoords[cubeFaceIndices[i+1]-1];
            normals[vertIdx]   = cubeNormals[cubeFaceIndices[i+2]-1];
            vertIdx++;
        }
    }
    else
    {
        for (int i = (numCubeFaces ) * 9 - 3; i >= 0; i-=3) //load it in reverse order (facing in)
        {
            positions[vertIdx] = cubeVerts[cubeFaceIndices[i]-1];
            vertIdx++;
        }
    }

        //assign static buffers to be shared by all!
        glGenBuffers(1, &vertPositions);
        if (!skybox)
        {
            glGenBuffers(1, &vertNormals);
            glGenBuffers(1, &vertTexCoords);
        }

        //TODO  seems like we need to ensure all attribute locations are as follows...
        //or some global in renderer that holds thier locations, and is set in Init?
        //remember they need to be set before shader is linked
        glBindBuffer(GL_ARRAY_BUFFER, vertPositions);
        glEnableVertexAttribArray(0); //vertpos location
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), positions, GL_STATIC_DRAW);

        if (!skybox)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
            glEnableVertexAttribArray(1);//normal location
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
            glEnableVertexAttribArray(2);//texcoords
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector2), texCoords, GL_STATIC_DRAW);
        }


    return true;
}

bool ModelResource::LoadCubeTriStrip()
{
    if (positions)
    {
        LogError("Trying to load cube into an already existing model?");
        return false; //something is already loaded here...
    }
    numVerts  =     numTSCubeIndices;
    positions =     new Vector3[numVerts];
    normals   =     new Vector3[numVerts];
    texCoords =     new Vector2[numVerts];
    int vertIdx = 0;
    //now that all the data is loaded, we need to construct something openGL can use, from the indices

    for (int i = 0; i < numTSCubeIndices;i++)
    {
        positions[i] = tsCubeVerts[tsCubeIndices[i]];
        texCoords[i] = Vector2(0.0, 0.0);
        normals[i]   = Vector3(0.0, 1.0, 0.0);
       // vertIdx++;
    }
    
   

        //assign static buffers to be shared by all!
        glGenBuffers(1, &vertPositions);
        glGenBuffers(1, &vertNormals);
        glGenBuffers(1, &vertTexCoords);
        

        //TODO  seems like we need to ensure all attribute locations are as follows...
        //or some global in renderer that holds thier locations, and is set in Init?
        //remember they need to be set before shader is linked
        glBindBuffer(GL_ARRAY_BUFFER, vertPositions);
        glEnableVertexAttribArray(0); //vertpos location
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), positions, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
        glEnableVertexAttribArray(1);//normal location
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
        glEnableVertexAttribArray(2);//texcoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector2), texCoords, GL_STATIC_DRAW);
    


    return true;
}


bool ModelResource::LoadObj(const char *filename, bool _collisionMesh, ResourceLoadMemInfo *mem)
{

    LOGOUTPUT << "Loading OBJ File: " << filename;
    LogInfo();

    minmaxX.x = 0.0f;
    minmaxX.y = 0.0f;
    minmaxY.x = 0.0f;
    minmaxY.y = 0.0f;
    minmaxZ.x = 0.0f;
    minmaxZ.y = 0.0f;

    std::vector< Vector3> temp_vertices;
    std::vector< Vector2 > temp_uvs;
    std::vector< Vector3 > temp_normals;
#if defined(__ANDROID__)
    if (!mem) return 0;
    FILE * file = fmemopen(mem->mem, mem->size, "r");
#else
    FILE * file = fopen(filename, "r");
#endif
    if( file == NULL ){
        printf("Impossible to open the file !\n");
        return false;
    }

    char readBuffer[512];
    char lineHeader[512];
    while(!feof(file))
    {

        fgets(readBuffer, sizeof(readBuffer), file);

        // read the first word of the line
        int res = sscanf(readBuffer, "%s", lineHeader);
        
        if (res <= 0) //NOTE emscripten EOF is incorrect? or fscanf return
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            Vector3 vertex;
            sscanf(readBuffer, "%s %f %f %f\n", lineHeader, &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
        else if ( strcmp( lineHeader, "vt" ) == 0 )
        {
            Vector2 uv;
            sscanf(readBuffer, "%s %f %f\n", lineHeader, &uv.x, &uv.y );
            temp_uvs.push_back(uv);
        }
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            Vector3 normal;
            sscanf(readBuffer, "%s %f %f %f\n", lineHeader, &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);

        }
        else if ( strcmp( lineHeader, "f" ) == 0 )
        {
           // std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = sscanf(readBuffer, "%s %d/%d/%d %d/%d/%d %d/%d/%d\n", lineHeader, &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            //if (matches != 10){
            //    printf("File can't be read by our simple parser : ( Try exporting with other options\n");
              //  return false;
           // }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }

    numVerts = (unsigned int)vertexIndices.size();
    positions =     new Vector3[numVerts];
    normals   =     new Vector3[numVerts];
    texCoords =     new Vector2[numVerts];
   // int vertIdx = 0;

    Vector3 tmp;
    //now that all the data is loaded, we need to construct something openGL can use, from the indices
    for (int i = 0; i < numVerts; i++)
    {
        positions[i] = temp_vertices[vertexIndices[i]-1];
        normals[i]   = temp_normals[normalIndices[i]-1];
        texCoords[i] = temp_uvs[uvIndices[i]-1];

        //also load the min's and max's    x= minmaxX
        tmp = positions[i];
        if (tmp.x < minmaxX.x)
            minmaxX.x = tmp.x;
        if (tmp.x > minmaxX.y)
            minmaxX.y = tmp.x;

        if (tmp.y < minmaxY.x)
            minmaxY.x = tmp.y;
        if (tmp.y > minmaxY.y)
            minmaxY.y = tmp.y;

        if (tmp.z < minmaxZ.x)
            minmaxZ.x = tmp.z;
        if (tmp.z > minmaxZ.y)
            minmaxZ.y = tmp.z;
    }

    float xLen = minmaxX.y - minmaxX.x;
    float yLen = minmaxY.y - minmaxY.x;

    widthToHeightAspect = xLen / yLen;

    //assign static buffers to be shared by all!
    glGenBuffers(1, &vertPositions);
    glGenBuffers(1, &vertNormals);
    glGenBuffers(1, &vertTexCoords);

    glBindBuffer(GL_ARRAY_BUFFER, vertPositions);
    glEnableVertexAttribArray(0); //vertpos location
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), positions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
    glEnableVertexAttribArray(1);//normal location
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
    glEnableVertexAttribArray(2);//texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector2), texCoords, GL_STATIC_DRAW);


    //generate collision data
    if (_collisionMesh)
    {
        collisionMesh = new btTriangleMesh();
        for (int i = 0; i < numVerts; i += 3)
        {
            collisionMesh->addTriangle(btVector3(positions[i].x, positions[i].y, positions[i].z),
                            btVector3(positions[i+1].x, positions[i+1].y, positions[i+1].z),
                            btVector3(positions[i+2].x, positions[i+2].y, positions[i+2].z),
                            true); // removes duplicates
        }
    }
    fclose(file);
    FileName = filename;
    return true;

}

//file pointer to the already open file,  and name of the file(anything unique i guess)
bool ModelResource::LoadXFWStream(FILE* file, const char *filename, bool _collisionMesh)
{
    minmaxX.x = 0.0f;
    minmaxX.y = 0.0f;
    minmaxY.x = 0.0f;
    minmaxY.y = 0.0f;
    minmaxZ.x = 0.0f;
    minmaxZ.y = 0.0f;

    std::vector< Vector3>  temp_vertices;
    std::vector< Vector2 > temp_uvs;
    std::vector< Vector3 > temp_normals;

    //FILE * file = fopen(filename, "r");
    if( file == NULL ){
        LogError("File porter is null!!!!!!!!!!");
        return false;
    }

    unsigned int linecounter = 0;
    //NOTE putting a line cap incase the file is corrupt and infinite loop occurs, 100,000 million lines,
    unsigned int MaxLines = 100000;
    bool reading = true;
    char readBuffer[512];
    char lineHeader[512];
    while(!feof(file) && reading)
    {

        fgets(readBuffer, sizeof(readBuffer), file);

        // read the first word of the line
        int res = sscanf(readBuffer, "%s", lineHeader);

        if (res <= 0) //NOTE emscripten EOF is incorrect? or fscanf return
            break; // EOF = End Of File. Quit the loop.
            
        if (res == EOF || feof(file) )
        {
            if (strcmp(lineHeader, "endmesh") != 0)
                LogError("LoadXFWStream() hit an unexpected EOF, check file structure");
            break; // EOF = End Of File. Quit the loop.
        }
        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            Vector3 vertex;
            sscanf(readBuffer, "%s %f %f %f\n", lineHeader, &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if ( strcmp( lineHeader, "vt" ) == 0 )
        {
            Vector2 uv;
            sscanf(readBuffer, "%s %f %f\n", lineHeader, &uv.x, &uv.y );
            temp_uvs.push_back(uv);
        }
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            Vector3 normal;
            sscanf(readBuffer, "%s %f %f %f\n", lineHeader, &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);

        }
        else if ( strcmp( lineHeader, "f" ) == 0 )
        {
           // std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = sscanf(readBuffer, "%s %d/%d/%d %d/%d/%d %d/%d/%d\n", lineHeader, &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 10){
                printf("File can't be read by our simple parser : (check texcoords, and normals)Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else if ( strcmp( lineHeader, "endmesh") == 0)
        {
            reading = false;
        }
      //  linecounter++;
    }

    //if (linecounter == MaxLines)
     //   LogError("Line counter hit max lines on mesh CFW load");

    numVerts = (unsigned int)vertexIndices.size();
    positions =     new Vector3[numVerts];
    normals   =     new Vector3[numVerts];
    texCoords =     new Vector2[numVerts];
   // int vertIdx = 0;

    Vector3 tmp;
    //now that all the data is loaded, we need to construct something openGL can use, from the indices
    for (int i = 0; i < numVerts; i++)
    {
        positions[i] = temp_vertices[vertexIndices[i]-1];
        normals[i]   = temp_normals[normalIndices[i]-1];
        texCoords[i] = temp_uvs[uvIndices[i]-1];

        //also load the min's and max's    x= minmaxX
        tmp = positions[i];
        if (tmp.x < minmaxX.x)
            minmaxX.x = tmp.x;
        if (tmp.x > minmaxX.y)
            minmaxX.y = tmp.x;

        if (tmp.y < minmaxY.x)
            minmaxY.x = tmp.y;
        if (tmp.y > minmaxY.y)
            minmaxY.y = tmp.y;

        if (tmp.z < minmaxX.x)
            minmaxX.x = tmp.z;
        if (tmp.z > minmaxX.y)
            minmaxX.y = tmp.z;
    }

    float xLen = minmaxX.y - minmaxX.x;
    float yLen = minmaxY.y - minmaxY.x;

    widthToHeightAspect = xLen / yLen;
    //assign static buffers to be shared by all!
    glGenBuffers(1, &vertPositions);
    glGenBuffers(1, &vertNormals);
    glGenBuffers(1, &vertTexCoords);

    glBindBuffer(GL_ARRAY_BUFFER, vertPositions);
    glEnableVertexAttribArray(0); //vertpos location
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), positions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertNormals);
    glEnableVertexAttribArray(1);//normal location
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector3), normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertTexCoords);
    glEnableVertexAttribArray(2);//texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vector2), texCoords, GL_STATIC_DRAW);


    //generate collision data
    if (_collisionMesh)
    {
        collisionMesh = new btTriangleMesh();
        for (int i = 0; i < numVerts; i += 3)
        {
            collisionMesh->addTriangle(btVector3(positions[i].x, positions[i].y, positions[i].z),
                            btVector3(positions[i+1].x, positions[i+1].y, positions[i+1].z),
                            btVector3(positions[i+2].x, positions[i+2].y, positions[i+2].z),
                            true); // removes duplicates
        }
    }

    FileName = filename;
    return true;
}

void UnloadModelResource(const char *filename)
{
    std::map<std::string, ModelResource *, StringCompareForMap>::iterator iter;
    iter = Models.find(string(filename));
    if (iter == Models.end())
    {
        LOGOUTPUT << "UnloadModelResource() - filename not found";
        LogError();
        return;
    }
    if (iter->second->getReferences() <= 0)
    {
        Models.erase(iter);
    }
    else
    {
        LOGOUTPUT << "UnloadModelResource() - attempted to unload resource with ("
        << iter->second->getReferences() << ") active references";
        LogError();
    }

}
