/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "BoundingVolumes.h"
#include "Renderer.h"



void DrawEntity(Entity *ent)
{
#if defined(PC_PLATFORM)

    Vector3 c = ent->GetTransform()->position;
    float halfWidth = ent->GetTransform()->radius;
    glColor3f(0.8, 0.0, 1.0);

    //left side
    float xLoc = c.x - halfWidth;
    glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    glVertex3f(xLoc, c.y + halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z + halfWidth);
    // glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    //right side
    xLoc = c.x + halfWidth;
    glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    glVertex3f(xLoc, c.y + halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z + halfWidth);
    // glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    //bottom side
    float yLoc = c.y - halfWidth;
    glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    glVertex3f(c.x + halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z + halfWidth);
    // glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    //top side
    yLoc = c.y + halfWidth;
    glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    glVertex3f(c.x + halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z + halfWidth);
    //  glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    //back side
    float zLoc = c.z + halfWidth;
    glVertex3f(c.x + halfWidth, c.y + halfWidth, zLoc);
    glVertex3f(c.x + halfWidth, c.y - halfWidth, zLoc);
    glVertex3f(c.x - halfWidth, c.y - halfWidth, zLoc);
    glVertex3f(c.x - halfWidth, c.y + halfWidth, zLoc);
    // glVertex3f(c.x + halfWidth, c.y + halfWidth, zLoc);

    glColor3f(0.0, 0.0, 0.0);


#endif
}


//recursively build an octree
BVHOctreeNode *OctreeBuild(BVHOctreeNode *parent, Vector3 center, float halfWidth, int stopDepth)
{
    if (stopDepth == 0)
        return 0; // back out man, back out!!

    BVHOctreeNode *node = new BVHOctreeNode;
    node->parent = parent;
    node->level = (char)stopDepth;
    node->center = center;
    node->halfWidth = halfWidth;

    Vector3 offset;
    float step = halfWidth * 0.5f;
    for (int i = 0; i < 8; i++)
    {
        offset.x = ((i & 1) ? step : - step);
        offset.y = ((i & 2) ? step : - step);
        offset.z = ((i & 4) ? step : - step);
        node->children[i] = OctreeBuild(node, center + offset, step, stopDepth - 1);
    }
    return node;  
}

void OctreeInsert(BVHOctreeNode *node, Entity *obj)
{
    int index = 0;
    int stradle = 0;
    float delta;
    
    float radius = obj->GetTransform()->radius;
    Vector3 pos = obj->GetTransform()->position;
    Vector3 dist = pos - node->center;
    //TODO fix this, when objects are outside of octree
    if (dist.Magnitude() - radius > node->halfWidth*2)
    {
        obj->octreeNode = node;
        return; //outside of root AABB
    }

    //compute octant number  0 - 7  if straddling a dividor, break
    //in da book this was a crazy for loop with array accessor on vector.
    delta = pos.x - node->center.x;
    if (fabs(delta) <= radius){stradle = 1;goto BVCHECK;}
    if (delta > 0.0f) index |= 1;

    delta = pos.y - node->center.y;
    if (fabs(delta) <= radius){stradle = 1;goto BVCHECK;}
    if (delta > 0.0f) index |= (1 << 1);

    delta = pos.z - node->center.z;
    if (fabs(delta) <= radius){stradle = 1;goto BVCHECK;}
    if (delta > 0.0f) index |= (1 << 2);

BVCHECK:

    if (!stradle && node->children[index])
    {
        //not straddling, and child is not null, dig deeper
        OctreeInsert(node->children[index], obj);
    }
    else
    {
        //straddling, or we are as deep as we can go. insert here cap'n
        //i'm assuming anything outside of the root node gets inserted at root level?
        //node->objects.push_back(obj);
        node->objects.push_front(obj);
        obj->octreeNode = node;
    }
}

// * note * does NOT delete node that was passed in
void OctreeDeleteChildren(BVHOctreeNode *node)
{
    if (node->children == 0)
        return;

    for (int i = 0; i < 8; i++)
    {
        OctreeDeleteChildren(node->children[i]);
        delete node->children[i];  
    }

}

float thickness = 0.0f;
int lvl = 0;
float colors[6][3] =   { 
    { 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f }

};

void DrawNode(BVHOctreeNode *node, int depth)
{
#if defined(PC_PLATFORM)
    //draw self
    Vector3 c = node->center;
    float halfWidth = node->halfWidth;
    float xLoc, yLoc, zLoc;


    if (node->children == 0)
        return;

    //draw children
    for (int i = 0; i < 8; i++)
    {
        if (node->children[i])
            DrawNode(node->children[i], depth+1);
    }

    SLList<Entity *>::iterator iter = node->objects.getIterator();
    while (iter.isGood())
    {
        DrawEntity(iter.get());
        iter.next();
    }
    if (depth == 0)
        return;

    if (node->objects.size() == 0)
        return;

    if (!node->children[0])
        if (node->objects.size())  
            glColor3f(0.0, 1.0, 0.2);  
   

    //left side
    xLoc = c.x - halfWidth;
    glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    glVertex3f(xLoc, c.y + halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z + halfWidth);
    // glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    //right side
    xLoc = c.x + halfWidth;
    glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    glVertex3f(xLoc, c.y + halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z - halfWidth);
    glVertex3f(xLoc, c.y - halfWidth, c.z + halfWidth);
    // glVertex3f(xLoc, c.y + halfWidth, c.z + halfWidth);
    //bottom side
    yLoc = c.y - halfWidth;
    glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    glVertex3f(c.x + halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z + halfWidth);
    // glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    //top side
    yLoc = c.y + halfWidth;
    glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    glVertex3f(c.x + halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z - halfWidth);
    glVertex3f(c.x - halfWidth, yLoc, c.z + halfWidth);
    //  glVertex3f(c.x + halfWidth, yLoc, c.z + halfWidth);
    //back side
    zLoc = c.z + halfWidth;
    glVertex3f(c.x + halfWidth, c.y + halfWidth, zLoc);
    glVertex3f(c.x + halfWidth, c.y - halfWidth, zLoc);
    glVertex3f(c.x - halfWidth, c.y - halfWidth, zLoc);
    glVertex3f(c.x - halfWidth, c.y + halfWidth, zLoc);
    // glVertex3f(c.x + halfWidth, c.y + halfWidth, zLoc);
    
    glColor3f(0.0, 0.0, 0.0);

#endif

}

void OctreeDraw(BVHOctreeNode *node)
{

#if defined(PC_PLATFORM)
    glLineWidth(2.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(gRenderer->camera->GetProjection());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(gRenderer->camera->inverseView);

    glColor3f(0.0, 0.0, 0.0);
    thickness = 2.2f;
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(thickness);
    glBegin(GL_QUADS);

    int depth = 1;
    DrawNode(node, depth);

    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glPolygonMode( GL_FRONT, GL_FILL );

    glColor3f(1.0f, 1.0f, 1.0f);
#endif
}

void OctreeRemove(BVHOctreeNode *node, Entity *obj)
{
    node->objects.remove(obj);
    obj->octreeNode = 0;
}





