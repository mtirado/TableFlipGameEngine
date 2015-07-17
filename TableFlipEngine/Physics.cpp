/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

//NOTE TODO INVESTIGATE an optimization may be to share collision shapes?? cache performance perhaps?
#include "Physics.h"
#include "Components.h"
#include "Entity.h"
#include "GLDebugDrawer.h"
#include <BulletCollision/Gimpact/btGImpactShape.h>
//#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
//#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include "CharacterController.h"
#include "Renderer.h"
#include <unistd.h>
Physics *Physics::instance = 0;

#if defined(PC_PLATFORM)
GLDebugDrawer G_DebugDrawer;
#endif
Physics::Physics()
{    
    worldEntity = new Entity;
    dynamicsWorld = 0;
}
Physics::~Physics()
{
    delete worldEntity;
}

void Physics::InitInstance()
{
    if (!instance)
        instance = new Physics;
}

void Physics::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}





//holds 2 objects that have made contacts, for insertion into map
struct ContactPair
{
    Entity *a, *b;
    ContactPair(Entity *_a, Entity *_b)
    {
        a = _a;
        b = _b;
    }
    bool operator < (const ContactPair &pair) const
    {
        //just going to compare pointers...
        size_t ourLow = std::min((size_t)a, (size_t)b);
        size_t ourHigh = std::max((size_t)a, (size_t)b);
        size_t theirLow = std::min((size_t)pair.a, (size_t)pair.b);
        size_t theirHigh = std::max((size_t)pair.a, (size_t)pair.b);
        
        if (ourLow < theirLow) return true;
        if (ourLow == theirLow) return (ourHigh < theirHigh);
        
        return false;
    }
};

//HACK its getting bad out here...  so we dont call collision leave for stuff destroyed this frame
vector<Entity *> entitiesRemovedThisFrame;
//hidden contact map
std::map<ContactPair, ContactInfo > contacts;
//remove entity from contacts and  end contact
void Physics::RemoveFromContacts ( Entity* ent )
{
    std::map<ContactPair, ContactInfo, std::less<ContactPair> >::iterator iter = contacts.begin();
    bool bValid = true;
    bool aValid = true;
    while (iter != contacts.end())
    {
        ContactPair cp = iter->first;
        if (cp.a == ent || cp.b == ent)
        {
            //TODO this is bad, no time to redesign right now though
            for (int i = 0; i < entitiesRemovedThisFrame.size(); i++)
            {
                if (entitiesRemovedThisFrame[i] == cp.a)
                    aValid = false;
                if (entitiesRemovedThisFrame[i] == cp.b)
                    bValid = false;
            }
            if (aValid)
                cp.a->OnCollisionLeave(cp.b);
            if (bValid)
                cp.b->OnCollisionLeave(cp.a);
            std::map<ContactPair, ContactInfo, std::less<ContactPair> >::iterator deleteMe = iter;
#if defined(__llvm__)  //yeah, this just happened...
           iter = contacts.erase(deleteMe);
            
        }
    else
        iter++;
#else
           contacts.erase(deleteMe);
          
        }
        iter++;
#endif
        entitiesRemovedThisFrame.push_back(ent);
    }
}




///User can override this material combiner by implementing gContactAddedCallback and setting body0->m_collisionFlags |= btCollisionObject::customMaterialCallback;
inline btScalar calculateCombinedFriction(float friction0,float friction1)
{
    //return 0.f;
    btScalar friction = friction0 * friction1;

    const btScalar MAX_FRICTION  = 10.f;
    if (friction < -MAX_FRICTION)
        friction = -MAX_FRICTION;
    if (friction > MAX_FRICTION)
        friction = MAX_FRICTION;
    return friction;

}

inline btScalar calculateCombinedRestitution(float restitution0,float restitution1)
{
    return restitution0 * restitution1;
}

static bool CustomMaterialCombinerCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
{

   // if (enable)
    {
        btAdjustInternalEdgeContacts(cp,colObj1Wrap,colObj0Wrap, partId1,index1);
        //btAdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, BT_TRIANGLE_CONVEX_BACKFACE_MODE);
        //btAdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, BT_TRIANGLE_CONVEX_DOUBLE_SIDED+BT_TRIANGLE_CONCAVE_DOUBLE_SIDED);
    }

    float friction0 = colObj0Wrap->getCollisionObject()->getFriction();
    float friction1 = colObj1Wrap->getCollisionObject()->getFriction();
    float restitution0 = colObj0Wrap->getCollisionObject()->getRestitution();
    float restitution1 = colObj1Wrap->getCollisionObject()->getRestitution();

//     if (colObj0Wrap->getCollisionObject()->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
//     {
//         friction0 = 1.0;//partId0,index0
//         restitution0 = 0.f;
//     }
//     if (colObj1Wrap->getCollisionObject()->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
//     {
//         if (index1&1)
//         {
//             friction1 = 1.0f;//partId1,index1
//         } else
//         {
//             friction1 = 0.f;
//         }
//         restitution1 = 0.f;
//     }

    cp.m_combinedFriction = calculateCombinedFriction(friction0,friction1);
    cp.m_combinedRestitution = calculateCombinedRestitution(restitution0,restitution1);

    //this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
    return true;
}

extern ContactAddedCallback     gContactAddedCallback;



btGhostPairCallback *G_GhostPairCallback = 0;
btBox2dBox2dCollisionAlgorithm::CreateFunc *G_2dCreateFunc = 0;
//i'm thinking there needs to be an OS Data layer, for passing platform specific data.
bool Physics::Init()
{
 // LogInfo("physics::init");
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;//new btParallelConstraintSolver();  -- threaded solver
    dynamicsWorld = (btDiscreteDynamicsWorld *)new btOrbitalDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    //((btOrbitalDynamicsWorld *)dynamicsWorld)->setMassiveThreshold(1000000);
    //dynamicsWorld->setGravity(btVector3(0,/*MtoKM*/(-18.8),0));
    //moon gravity 1.622 m/s²
    dynamicsWorld->setGravity(btVector3(0,/*MtoKM*/(-2.2613332211),0));
    dynamicsWorld->getDispatchInfo().m_enableSatConvex = true;

#if defined(PC_PLATFORM)
    dynamicsWorld->setDebugDrawer(&G_DebugDrawer);
    dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
#endif
    
    
    //callback for ghost objects
    G_GhostPairCallback = new btGhostPairCallback();
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(G_GhostPairCallback);
   
    G_2dCreateFunc = new btBox2dBox2dCollisionAlgorithm::CreateFunc();
    //setup 2d world
    collisionConfiguration2d = new btDefaultCollisionConfiguration();
    //m_collisionConfiguration->setConvexConvexMultipointIterations();

    //use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    dispatcher2d = new btCollisionDispatcher(collisionConfiguration2d);
    simplex = new btVoronoiSimplexSolver();
    pdSolver = new btMinkowskiPenetrationDepthSolver();
    convexAlgo2d = new btConvex2dConvex2dAlgorithm::CreateFunc(simplex,pdSolver);
    
    dispatcher2d->registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE,CONVEX_2D_SHAPE_PROXYTYPE,convexAlgo2d);
    dispatcher2d->registerCollisionCreateFunc(BOX_2D_SHAPE_PROXYTYPE,CONVEX_2D_SHAPE_PROXYTYPE,convexAlgo2d);
    dispatcher2d->registerCollisionCreateFunc(CONVEX_2D_SHAPE_PROXYTYPE,BOX_2D_SHAPE_PROXYTYPE,convexAlgo2d);
    //dispatcher2d->registerCollisionCreateFunc(BOX_2D_SHAPE_PROXYTYPE,BOX_2D_SHAPE_PROXYTYPE,convexAlgo2d);
    dispatcher2d->registerCollisionCreateFunc(BOX_2D_SHAPE_PROXYTYPE,BOX_2D_SHAPE_PROXYTYPE, G_2dCreateFunc);

    broadphas2d = new btDbvtBroadphase();
    //m_broadphase = new btSimpleBroadphase();

    //the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
    solver2d = sol;

    dynamicsWorld2d = new btDiscreteDynamicsWorld(dispatcher2d,broadphas2d,solver2d,collisionConfiguration2d);
    dynamicsWorld2d->setGravity(btVector3(0,/*MtoKM*/(-9.8),0));
#if defined(PC_PLATFORM)
    dynamicsWorld2d->setDebugDrawer(&G_DebugDrawer);

    dynamicsWorld2d->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb);
    #endif
    //m_dynamicsWorld->getSolverInfo().m_erp = 1.f;
    //m_dynamicsWorld->getSolverInfo().m_numIterations = 4;  


    gContactAddedCallback = CustomMaterialCombinerCallback;
    
    return true;
}


void Physics::Update (float dt)
{
    if (dt > 1.0)
        return;
   //physics updates first, so this should be ok...
   entitiesRemovedThisFrame.clear();
    //cout << "timestep: " << dt << endl;
    //dynamicsWorld->stepSimulation(dt, 3);
    dynamicsWorld->setLatencyMotionStateInterpolation(false);
    
    dynamicsWorld->stepSimulation(1.0/60.0, 25, (1.0/60.0)/3.0);
    //dynamicsWorld->stepSimulation(1.0/120.0, 0);
    dynamicsWorld2d->stepSimulation(dt,2); //TODO special 2d motion state?? (transform2d)
    Transform3d *objTrans;
    
    //update characters  TODO this should probably use motion states
    for (int i = 0; i < characters.size(); i++)
    {
        btQuaternion q;
        objTrans = characters[i]->transform;
        btTransform &t = characters[i]->getGhostObject()->getWorldTransform();
       // characters[i]->getMotionState()->getWorldTransform(t);
        q = t.getRotation();
        btVector3 pos = t.getOrigin();
       
        objTrans->position.x = pos.getX();
        objTrans->position.y = pos.getY() + characters[i]->yOffset;
        objTrans->position.z = pos.getZ();  //nbackwards sonny dun dunny
       
        objTrans->rotation.x = -q.getX();
        objTrans->rotation.y = q.getY();
        objTrans->rotation.z = -q.getZ();        
        //not acutally sure whats going on here, but the bullet quaternion seems ...  different?
        objTrans->rotation.w = q.getW();
        
    
        objTrans->Update();
        
        //a little redundant, updates position again
        objTrans->position = objTrans->position - (objTrans->GetBack()) * characters[i]->zOffset;
        objTrans->UpdatePosition();
    }
    
    
    //handle contact messaging
    UpdateContacts();
}

void Physics::AddRigidBody (RigidBody* rb)
{
    //end of the line for this HACK
    if (rb->is3d)
    {
        rigidBodies.push_back(rb);
        dynamicsWorld->addRigidBody(rb->rigidBody);
    }
    else
    {
        rigidBodies2d.push_back(rb);
        dynamicsWorld2d->addRigidBody(rb->rigidBody);
    }
}

void Physics::AddCollisionShape ( CollisionShape* cs )
{
    if (cs->is3d())
    {
        collisionShapes.push_back(cs);
        dynamicsWorld->addCollisionObject(cs->collisionObject);
    }
    else
    {
        collisionShapes2d.push_back(cs);
        dynamicsWorld2d->addCollisionObject(cs->collisionObject);
    }
    
}

void Physics::AddCharacter(CharacterController* cc)
{
  characters.push_back(cc);
    dynamicsWorld->addCollisionObject(cc->ghostObject,btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::KinematicFilter,
                btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
    dynamicsWorld->addAction((btKinematicCharacterController *)cc);
}

bool Physics::RemoveCharacter(CharacterController* cc)
{
    vector<CharacterController *>::iterator iter;
    iter = characters.begin();
    while (iter != characters.end())
    {
        if (*iter == cc)
        {
            dynamicsWorld->removeCollisionObject((*iter)->ghostObject);
            dynamicsWorld->removeAction((btKinematicCharacterController *)(*iter));
            characters.erase(iter);
            RemoveFromContacts(cc->entity);
            return true;
        }
        iter++;
    }
    return false;
  
}


RigidBody *Physics::CreateNewRigidBody(Entity *ent, Transform3d *trans, btCollisionShape *shape, float mass, float linDrag,
                float angularDrag, float friction, float rollingFriction, float restitution )
{
    btVector3 fallInertia(0,0,0);
    if (mass != 0.0f) //0 mass means its a static object!
        shape->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,ent->GetTransform()->motionState,shape,fallInertia);
    btRigidBody *rb = new btRigidBody(fallRigidBodyCI);
    if (!ent->is3d()) //constrain to xy axis
    {
        //rb->setActivationState(ISLAND_SLEEPING);
        rb->setLinearFactor(btVector3(1,1,0));
        rb->setAngularFactor(btVector3(0,0,1));
       // rb->setActivationState(DISABLE_DEACTIVATION);
        rb->setSleepingThresholds(1.11, 0.99);
        //rb->setContactProcessingThreshold(0.0511);
       
    }
    else
        //rb->setContactProcessingThreshold(0);
    
    //apply properties
    
    rb->setDamping(linDrag, angularDrag);
    rb->setFriction(friction);
    rb->setRollingFriction(rollingFriction);
   // if (rollingFriction != 0.0f)
     //   rb->setAnisotropicFriction(shape->getAnisotropicRollingFrictionDirection(),btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);

    rb->setRestitution(restitution);
    
    RigidBody *newRigidody = new RigidBody(ent, trans);
   
    newRigidody->rigidBody = rb;
    
    return newRigidody;
    
}

void Physics::AddStaticGeometry(string name, Vector3 position, ModelResource *model)
{
    //for now, no index support. just straight verts
    if (model->isIndexed)
    {
        LogError("AddStaticGeometry() : does not support indexed meshes at the moment");
        return;
    }

    map<string, StaticMeshData *, StringCompareForMap>::iterator iter = staticWorld.find(name);
    if (iter != staticWorld.end())
    {
        LogError("AddStaticGeometry() : Static Geometry with that name already exists in world!");
        return;
    }

    if (model->numVerts == 0)
    {
        LogError("AddStaticGeometry() : mesh has no verts!");
        return;
    }

    //TODO NOTE memory may be leaking here
    btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(model->collisionMesh, false,true); //build quantized?
    StaticMeshData *meshData = new StaticMeshData;
    //create rigid body
    btDefaultMotionState *fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1.0f),
                                         btVector3(position.x, position.y, position.z)));
    btVector3 fallInertia(1.0f,0.0f,0.0f);
    shape->calculateLocalInertia(0.0f,fallInertia); //0 mass means fixed (no motion)
    //TODO  this static body currently will not be removed untill engine shutdown
    //due to how the entity doesnt know about it.
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(0,fallMotionState,shape,fallInertia);
    btRigidBody *rb = new btRigidBody(fallRigidBodyCI);

    rb->setFriction(1.0f);

    meshData->rb = rb;
    meshData->shape = shape;
    staticWorld[name] = meshData; //TODO check if it already exists...
    dynamicsWorld->addRigidBody(rb);

    //attach an entity so our collision notification system works with this mesh
    rb->setUserPointer(worldEntity);

}



//yeah redundant much???
void Physics::AddStaticTriangleMesh(string name, Vector3 position, btTriangleMesh *triMesh)
{
    //for now, no index support. just straight verts

    map<string, StaticMeshData *, StringCompareForMap>::iterator iter = staticWorld.find(name);
    if (iter != staticWorld.end())
    {
        LogError("AddStaticTriangleMesh() : Static Geometry with that name already exists in world!");
        return;
    }

    if (triMesh)
    {
        if (triMesh->getNumTriangles() == 0)
        {
            LogError("AddStaticTriangleMesh() : mesh has no tris!");
            return;
        }
    }
    else {LogError("AddStaticTriangleMesh() -- no tri mesh..."); return; }

    //TODO NOTE memory may be leaking here
    btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triMesh, true,true); //build quantized?

    //INTERNAL EDGE SAVIOR
    btTriangleInfoMap *tInfoMap = new btTriangleInfoMap;
   // btGenerateInternalEdgeInfo(shape, tInfoMap);
    //btGImpactMeshShape * shape = new btGImpactMeshShape(triMesh);
    //shape->updateBound();

    StaticMeshData *meshData = new StaticMeshData;
    shape->setMargin(0.13);
    //create rigid body
    btDefaultMotionState *fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1.0f),
                                         btVector3(position.x, position.y, position.z)));
    btVector3 fallInertia(1.0f,0.0f,0.0f);
    shape->calculateLocalInertia(0.0f,fallInertia); //0 mass means fixed (no motion)
    //TODO  this static body currently will not be removed untill engine shutdown
    //due to how the entity doesnt know about it.
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(0,fallMotionState,shape,fallInertia);
    btRigidBody *rb = new btRigidBody(fallRigidBodyCI);
    //rb->setCollisionFlags(rb->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
    rb->setFriction(1.0f);

    meshData->rb = rb;
    //meshData->shape = shape; GOT HACK S?
    staticWorld[name] = meshData; //TODO check if it already exists...
    dynamicsWorld->addRigidBody(rb);

    //attach an entity so our collision notification system works with this mesh
    rb->setUserPointer(worldEntity);

}

bool Physics::RemoveStaticGeometry ( string name )
{
    //TODO prolly need to delete the shape/rigidbody?
     std::map <std::string, StaticMeshData *, StringCompareForMap>::iterator iter;
     iter = staticWorld.find(name);
     if (iter == staticWorld.end())
         return false;
     LOGOUTPUT << "Removing static geometry from world: " << name;
     LogInfo();
     dynamicsWorld->removeRigidBody(iter->second->rb);
     RemoveFromContacts((Entity *)iter->second->rb->getUserPointer());
     staticWorld.erase(iter);
     return true;
     
}


bool Physics::RemoveRigidBody ( RigidBody* rb )
{
    vector<RigidBody *>::iterator iter;
     
    if (rb->is3d)
    {
        iter = rigidBodies.begin();
        while (iter != rigidBodies.end())
        {
            if (*iter == rb)
            {
                
                if (rb->is3d)
                    dynamicsWorld->removeRigidBody((*iter)->rigidBody);
                else
                    dynamicsWorld2d->removeRigidBody((*iter)->rigidBody);
                rigidBodies.erase(iter);
               RemoveFromContacts(rb->entity);
               // LOGOUTPUT << "Removing rigidbody shape from world: " << rb->entity->name;
               // LogInfo();
                return true;
            }
            iter++;
        }
        return false;
    }
    else
    {
        iter = rigidBodies2d.begin();
        while (iter != rigidBodies2d.end())
        {
            if (*iter == rb)
            {
                
                if (rb->is3d)
                    dynamicsWorld->removeRigidBody((*iter)->rigidBody);
                else
                    dynamicsWorld2d->removeRigidBody((*iter)->rigidBody);
                rigidBodies2d.erase(iter);
               RemoveFromContacts(rb->entity);
               // LOGOUTPUT << "Removing 2d rigidbody shape from world: " << rb->entity->name;
               // LogInfo();
                return true;
            }
            iter++;
        }
        return false;
    }
    return false;
    
}

bool Physics::RemoveCollisionShape ( CollisionShape* cs )
{
    vector<CollisionShape *>::iterator iter;
    if (cs->is3d())
    {
        iter = collisionShapes.begin();
        while (iter != collisionShapes.end())
        {
            if (*iter == cs)
            {
                LOGOUTPUT << "Removing collision shape from world: " << cs->entity->name;
                LogInfo();
                dynamicsWorld->removeCollisionObject((*iter)->collisionObject);
                collisionShapes.erase(iter);
                RemoveFromContacts(cs->entity);
                return true;
            }
            iter++;
        }

        return false;
    }
    else
    {
        iter = collisionShapes2d.begin();
        while (iter != collisionShapes2d.end())
        {
            if (*iter == cs)
            {
                LOGOUTPUT << "Removing 2d collision shape from world: " << cs->entity->name;
                LogInfo();
                dynamicsWorld2d->removeCollisionObject((*iter)->collisionObject);
                collisionShapes2d.erase(iter);
                RemoveFromContacts(cs->entity);
                return true;
            }
            iter++;
        }

        return false;
    }
}


//update contact list, theres a better way without a redundant list of contacts
//but aparently it doesnt work with multithreaded solvers, and we might want that
//if we dont want a multithreaded solver,  look into gContactAddedCallback
//so we dont need to iterate all these damn contact points every frame....
//an optimization may be to only update some contacts per frame, not all. 
static int frameCounter = 0;
void Physics::UpdateContacts()
{
    frameCounter++;
    Entity *a, *b;
    //manifolds hold all the intersecting points between 2 objects colliding
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject *obA = contactManifold->getBody0();
        const btCollisionObject *obB = contactManifold->getBody1();
        
        int numContacts = contactManifold->getNumContacts();
        
        if (numContacts > 0)
        {
            a = (Entity *)(obA->getUserPointer());
            b = (Entity *)(obB->getUserPointer());
            if (a && b)
                contacts[ ContactPair(a,b) ].Update(frameCounter); //inserts, and updates
        }
    }
    
    //TODO  seperate 2d contact list
    numManifolds = dynamicsWorld2d->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold *contactManifold = dynamicsWorld2d->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject *obA = contactManifold->getBody0();
        const btCollisionObject *obB = contactManifold->getBody1();
        
        int numContacts = contactManifold->getNumContacts();
        
        if (numContacts > 0)
        {
            a = (Entity *)(obA->getUserPointer());
            b = (Entity *)(obB->getUserPointer());
            if (a && b)
                contacts[ ContactPair(a,b) ].Update(frameCounter); //inserts, and updates
        }
    }
    
    std::map<ContactPair, ContactInfo>::iterator iter = contacts.begin();
   
    while (iter != contacts.end())
    {
        a = iter->first.a;
        b = iter->first.b;
        if (iter->second.isNewContact())
        { 
            a->OnCollisionEnter(b);
            b->OnCollisionEnter(a);
            //there seems to be a bug with contacts only existing for one frame, hack?
            iter->second.Update(frameCounter+1);
            iter++;
            
            
        }
        else if (iter->second.isOldContact(frameCounter))
        {
            //TODO this is fucking broken and causing a crash,  FFS
	    //XXX   wait i thought i resolved this, maybe i forgot to remove
	    //the TODO,  welllp,  TODO:  verify this alleged bug.
            a->OnCollisionLeave(b);
            b->OnCollisionLeave(a);
            std::map<ContactPair, ContactInfo>::iterator deleteMe = iter;
            iter++;
            contacts.erase(deleteMe);
        }
        else
        {
            //TODO fixxxxxx this please!  -- seems like a bullet bug to me, change oncollision notifications
            a->OnCollisionStay(b);
            b->OnCollisionStay(a);
            iter++;
        }
    }
    
}


RayHitInfo Physics::Raycast(Vector3 start, Vector3 end)
{
    // Start and End are vectors
    //TODO reallyneed to do some conversion between types, they're practically the same thing
    btVector3 Start, End;
    Start = btVector3(start.x, start.y, start.z);
    End = btVector3(end.x, end.y, end.z);
    
    btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);

    // Perform the raycast
    dynamicsWorld->rayTest(Start, End, RayCallback);

    
    RayHitInfo ret;
    ret.entity = 0;
    if(RayCallback.hasHit())
    {
       
        ret.entity = ((Entity *)RayCallback.m_collisionObject->getUserPointer());
        ret.worldPoint = RayCallback.m_hitPointWorld;
        ret.worldNormal = RayCallback.m_hitNormalWorld;
    }
   
    return ret;
}

#define _RAY_DIST_ 4000
//yes, relies on an active camera, in renderer
RayHitInfo Physics::ScreenRaycast(Vector2 screenCoords)
{
    LogError("WTF HAPPENED TO Physics::ScreenRaycast()?? fixit....");
    RayHitInfo rh;
    rh.entity = 0;
    return rh;
//     //http://www.antongerdelan.net/opengl/raycasting.html
//     Vector4 rayClip, rayEye, rayWorld;
//     rayClip = Vector4(  (2.0 * screenCoords.x) / Renderer::GetInstance()->width - 1.0f,
//                         1.0f - (2.0f * screenCoords.y) / Renderer::GetInstance()->height,
//                         -1.0, /*forward in opengl*/
//                         1.0);
//     Matrix4 inv;
//     MatrixInvert(inv,Renderer::GetInstance()->GetCamera()->GetProjection());
//     Matrix4Multiply(rayEye, inv, rayClip);
//     rayEye.z = -1.0f;
//     rayEye. = 0.0f;
//     Matrix4MultiplyVector4(rayWorld, Renderer::GetInstance()->GetCamera()->GetInverseView(), rayEye);
//     Vector3 mouseRay = Vector3(rayWorld[0], rayWorld[1], rayWorld[2]);
//     mouseRay.Normalize();
// 
//     Vector3 start = Renderer::GetInstance()->GetCamera()->GetTransform()->position;
//     Vector3 end = start + mouseRay * _RAY_DIST_; //or more??
// 
//    
//     return Physics::GetInstance()->Raycast(start, end);
}


void Physics::Shutdown()
{
    if (!dynamicsWorld)
        return;
 
    //clear static meshes
    int i = 0;
    for (i = 0; i < rigidBodies.size(); i++)
    {
        dynamicsWorld->removeRigidBody(rigidBodies[i]->rigidBody);
        delete rigidBodies[i];
    }
    LOGOUTPUT << i << " 3d rigid bodies removed";
    LogInfo();
    for (i = 0; i < rigidBodies2d.size(); i++)
    {
        dynamicsWorld2d->removeRigidBody(rigidBodies2d[i]->rigidBody);
        delete rigidBodies2d[i];
    }
    LOGOUTPUT << i << " 2d rigid bodies removed";
    LogInfo();
    for (i = 0; i < collisionShapes.size(); i++)
    {
        dynamicsWorld->removeCollisionObject(collisionShapes[i]->collisionObject);
        //delete collisionShapes[i];
    }
    LOGOUTPUT << i << " 3d collision shapes removed";
    LogInfo();
    for (i = 0; i < collisionShapes2d.size(); i++)
    {
        dynamicsWorld2d->removeCollisionObject(collisionShapes2d[i]->collisionObject);
        //delete collisionShapes[i];
    }
    LOGOUTPUT << i << " 2d collision shapes removed";
    LogInfo();
    
    i = 0;
    map<string, StaticMeshData *, StringCompareForMap>::iterator staticIter = staticWorld.begin();
    while (staticIter != staticWorld.end())
    {
        StaticMeshData *meshData = staticIter->second;
        dynamicsWorld->removeRigidBody(meshData->rb);
        delete meshData->rb;
        delete meshData->shape;
        delete meshData;
        staticIter++;
        i++;
    }
    LOGOUTPUT << i << " static collision meshes removed";
    LogInfo();
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
  
    
    delete broadphase;
    delete G_GhostPairCallback;
    
    delete collisionConfiguration2d;
    delete dispatcher2d;
    delete simplex;
    delete pdSolver;
    delete convexAlgo2d;
    delete broadphas2d;
    delete solver2d;

    delete dynamicsWorld2d;
    delete G_2dCreateFunc;
   
}
