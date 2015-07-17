/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef BOUNDINGVOLUMES_H__
#define BOUNDINGVOLUMES_H__

#include "EngineMath.h"
#include "DataStructures/SLList.h"
//#include "Entity.h"

class Entity;

#include <vector>
using std::vector;
//abstract base class, cannot instantiate
class BoundingVolume
{
   
    
};

//aabb, fastest, and easiest, never rotates. axis' aligned to world axis
class BVAABB : public BoundingVolume
{
public:
   
    //two opposing corner points
    Vector3 min;
    Vector3 max;
    
    //this is probably going to get called a shitload
    //just make sure the AABBs are generated correctly, opposing corners..
    inline bool TestAgainst(const BVAABB &other) const
    {
        if (max.x < other.min.x || min.x > other.max.x) return false;
        if (max.y < other.min.y || min.y > other.max.y) return false;
        if (max.z < other.min.z || min.z > other.max.z) return false;
        
        //hit!!
        return true;
    }
};

//bounding box, can be rotated
class BVBB : public BoundingVolume
{
};

//pretty darn obvious what this is..
class BVSphere : public BoundingVolume
{
public:
    BVSphere() { radius = 0.0f; }
    BVSphere(Vector3 c, float r) { center = c; radius = r; }
    Vector3 center;
    float   radius;

    
    inline bool TestAgainst(const BVSphere &other) const
    {
        Vector3 vecTo;
        vecTo = other.center - center;
        if (vecTo.Magnitude() <= radius + other.radius)
            return true;
        else
            return false;
    }
    
    void Draw();
    
};


struct BVHOctreeNode
{
    char level;  //subdivision level,  0 means leaf node, higher means closer to root
    Vector3 center;
    float halfWidth;
    BVHOctreeNode *children[8];
    BVHOctreeNode *parent;
    //vector<Entity *> objects; 
    SLList<Entity *> objects;
};

BVHOctreeNode *OctreeBuild(BVHOctreeNode *parent, Vector3 center, float halfWidth, int stopDepth);

void OctreeInsert(BVHOctreeNode *node, Entity *obj);
void OctreeRemove(BVHOctreeNode *node, Entity *obj);
void OctreeDeleteChildren(BVHOctreeNode *node);
void OctreeDraw(BVHOctreeNode *node);

#endif
