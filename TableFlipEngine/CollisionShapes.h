/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Components.h"
#include <BulletCollision/CollisionShapes/btBox2dShape.h>
#include <BulletCollision/CollisionShapes/btConvex2dShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

class BoxShape : public CollisionShape
{
private:
    BoxShape();
public:         
    BoxShape(Vector3 halfWidths, bool _isTrigger = false, bool hasRigidBody = false)
    {
        //half extents
        shape = new btBoxShape(btVector3(halfWidths.x, halfWidths.y, halfWidths.z));
        Init(_isTrigger, hasRigidBody);
        type = SHAPE_BOX;
    }
};

class Box2dShape : public CollisionShape
{
private:
    Box2dShape();
    Vector2 _halfWidths;
public:         
    Box2dShape(Vector3 halfWidths, bool _isTrigger = false, bool hasRigidBody = false)
    {
        //half extents
        _halfWidths.x = halfWidths.x;
        _halfWidths.y = halfWidths.y;
        
        Init(_isTrigger, hasRigidBody);
        type = SHAPE_BOX2D;
        _is3d = false;
        shape2dProxy = new btBox2dShape(btVector3(halfWidths.x, halfWidths.y, halfWidths.z));
        shape = new btConvex2dShape((btConvexShape*)shape2dProxy);
        shape->setUserPointer(this);
    }
    
    const Vector2 &getHalfWidths() { return _halfWidths; }
};

class SphereShape : public CollisionShape
{
private:
    SphereShape();
public:
    SphereShape(float radius, bool _isTrigger = false, bool hasRigidBody = false)
    {  
        Init(_isTrigger, hasRigidBody);
        shape = new btSphereShape(radius);
        type = SHAPE_SPHERE;
    }
};

//TODO implement
class CapsuleShape : public CollisionShape
{
private:
    CapsuleShape();
public:
};

class MeshShape : public CollisionShape
{
private:
    MeshShape();
public:
    //i think this is the right idea, so we can have independent scaling
    MeshShape(ModelResource *model, bool convex = true, bool _isTrigger = false, bool hasRigidBody = false)
    {
        if (convex)
        {
            
            btConvexHullShape *ch = new btConvexHullShape((btScalar *)model->positions, model->numVerts, sizeof(Vector3));
            btShapeHull *sh = new btShapeHull(ch);
            if (!sh->buildHull(1.0))
                LogError("COULDNT BUILD CONVEX HULL!?!?!?");
            shape = new btConvexHullShape();
            const unsigned int * indices = sh->getIndexPointer();
            const btVector3 *points = sh->getVertexPointer();
            for (int i = 0; i < sh->numIndices(); i++)
            {
                ((btConvexHullShape *)shape)->addPoint(points[indices[i]]);
            }
            
        }
        else //concave meshes are probably VERY slow, do not use them unless you must, or they are static.
        {
            if (model->collisionMesh)
            {
                shape = new btBvhTriangleMeshShape(model->collisionMesh, true);
               // ((btBvhTriangleMeshShape *)shape)->buildOptimizedBvh(); ???
            }
            else
            {
                LogError("trying to attach concave triangle mesh with no trimesh data, use LoadObj() with proper flag set");
                LogError("Defaulting to convex hull");
                shape = new btConvexHullShape((btScalar *)model->positions, model->numVerts, sizeof(Vector3));  
                //((btConvexHullShape *)shape)
                
            }
        }
        type = SHAPE_MESH;
        Init(_isTrigger, hasRigidBody);
    }
};
