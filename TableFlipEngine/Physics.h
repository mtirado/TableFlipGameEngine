/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef PHYSICS_H__
#define PHYSICS_H__

#include "btBulletDynamicsCommon.h"
#include <BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.h>
#include <BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.h>
#include <BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.h>
#include <map>
#include <vector>
#include <string>
#include "Resources.h"

class CharacterController;
class RigidBody;
class CollisionShape;
class Entity;
class Transform3d;

struct StaticMeshData
{
    btBvhTriangleMeshShape *shape;
    btRigidBody *rb;
};

class ContactInfo
{
private:
    unsigned int totalFramesInContact;
    unsigned int lastFrameInContact;
public:
    ContactInfo() { totalFramesInContact = 0; lastFrameInContact = 0; }
    void Update(unsigned int currentFrame) { lastFrameInContact = currentFrame; totalFramesInContact++; }
    bool isNewContact() { return (totalFramesInContact == 1); }
    bool isOldContact(unsigned int currentFrame) { return (lastFrameInContact != currentFrame); }
};

struct RayHitInfo
{
    Entity *entity;
    Vector3 worldPoint;
    Vector3 worldNormal;
};

class Physics
{
    friend class ObjectManager;
private:
    //disable functions for singleton use
    Physics();
    ~Physics();
    Physics(const Physics &) {}
    Physics &operator = (const Physics &) { return *this;}

    //the singletons instance.
    static Physics *instance;
   
    //3d world stuff
    btBroadphaseInterface *broadphase;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btSequentialImpulseConstraintSolver *solver;
    
    //2d world
    btDefaultCollisionConfiguration         *collisionConfiguration2d;
    btCollisionDispatcher                   *dispatcher2d;
    btVoronoiSimplexSolver                  *simplex;
    btMinkowskiPenetrationDepthSolver       *pdSolver;
    btConvex2dConvex2dAlgorithm::CreateFunc *convexAlgo2d;
    btBroadphaseInterface                   *broadphas2d;
    btSequentialImpulseConstraintSolver     *solver2d;
    btDiscreteDynamicsWorld                 *dynamicsWorld2d;
    
    
    Entity *worldEntity; //doesnt do anything seems a little hacky, TODO figure out world - collision should we just throw entities on everyting like unity?
    void UpdateContacts();
    void RemoveFromContacts(Entity *ent);
            
public:
    //custom dynamics world that allows for special large massive bodies to be added with orbital dynamics
    //btOrbitalDynamicsWorld *dynamicsWorld;
    btDiscreteDynamicsWorld *dynamicsWorld;
    btCollisionDispatcher *dispatcher;
    //test vars
    btCollisionShape *groundShape;
    btCollisionShape *sphereShape;
    btCollisionShape *cubeShape;
    
    //motion states (for updating position and whatnot)
    btDefaultMotionState *groundMotionState;
   // btDefaultMotionState *fallMotionState;
    
    btRigidBody *groundRigidBody;

    //list of rigid body components
    std::vector <RigidBody *> rigidBodies;
    std::vector <CollisionShape *> collisionShapes;
    std::vector <CharacterController *> characters;
    //things that do not move!
    std::map <std::string, StaticMeshData *, StringCompareForMap> staticWorld;
    
    //2d objects
    std::vector <RigidBody *> rigidBodies2d;
    std::vector <CollisionShape *> collisionShapes2d;
    
    //active contacts
   // std::map<
    
    //inlines
    inline static Physics *GetInstance() { return instance; }

    //init the singleton instance
    static void InitInstance();

    //delete the singelton instance
    static void DeleteInstance();
    
    void AddRigidBody(RigidBody *rb);
    void AddStaticGeometry(string name, Vector3 position, ModelResource *mesh); //static world
    void AddStaticTriangleMesh(string name, Vector3 position, btTriangleMesh *triMesh);
    bool RemoveStaticGeometry(string name); //static world
    void AddCollisionShape(CollisionShape *cs);
    void AddCharacter(CharacterController *cc);
    bool RemoveCharacter(CharacterController *cc);
    bool RemoveRigidBody(RigidBody *rb);
    bool RemoveCollisionShape(CollisionShape *cs);
    
   
    RigidBody *CreateNewRigidBody(Entity *ent, Transform3d *trans, btCollisionShape *shape, float mass, float linDrag = 0.0f,
                float angularDrag = 0.0f, float friction = 0.5f, float rollingFriction = 1.0f, float restitution = 0.0f);
     

    RayHitInfo Raycast(Vector3 start, Vector3 end);
    RayHitInfo ScreenRaycast(Vector2 screenCoords);
    //initialize the physics system
    bool Init();   
    void Update(float dt);  
    void Shutdown();
    void DrawDebugWireframe() { dynamicsWorld->debugDrawWorld(); dynamicsWorld2d->debugDrawWorld(); }
    
   // btBroadphaseInterface *GetBroadphase() { return broadphase; }
};


#endif
