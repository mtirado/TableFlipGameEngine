/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __OCTREE_ORB_HPP__
#define __OCTREE_ORB_HPP__

#include "Logic.h"
class OctreeOrb : public Logic
{
public:
    BVHOctreeNode *root;
    Vector3 velocity;
    
    void Init()
    {
        activeUpdate = true;
        velocity = Vector3(0.0);
    }

    void Update(float dt)
    {
        //find neighboring orbs and apply force
        int level = entity->octreeNode->level; 
        BVHOctreeNode *start;
        if (level == 1)
            start = entity->octreeNode->parent;
        else
            start = entity->octreeNode;

        //go through the start node and apply forces from every object
        Vector3 force(0.0);
        Vector3 tmp;
        Vector3 cur = transform->position;
        for (int i = 0; i < 8; i++)
        {
            BVHOctreeNode *node = start->children[i];
            SLList<Entity *>::iterator iter = node->objects.getIterator();
            while (iter.isGood())
            {
                tmp = cur - iter.get()->GetTransform()->position;
                force += tmp;
                iter.next();
            }
        }
        OctreeRemove(entity->octreeNode, entity);
        force.Normalize();
        force *= -0.67;
        velocity += force * dt;
        transform->position +=  velocity;
        velocity *= 0.9992;
        transform->UpdatePosition();   
        OctreeInsert(root, entity);
    }
};

#endif

