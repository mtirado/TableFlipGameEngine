/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Utilities.h"
#include "Entity.h"
#include "CharacterController.h"
#include "Renderer.h"


#include "Physics.h"
#include "ObjectManager.h"
#include "AnimationResource.h"
#include "Builder.h"
#include "Scripting.h"
#include <Sprite.h>
#include <Transform3d.h>

Entity::Entity()
{
    parent = 0;
    _is3d = true;
    refCount = 1; //yep, theres one ref already
    blueprint = 0;
#if defined(PC_PLATFORM) && !defined(__APPLE__)
    //scriptObject = 0;
#endif
   // drag = 1.0f;
   //Scripting::CreateScriptObject();
    //no components attached.
    for (int i = 0; i < NUM_COMPONENTS; i++)
        components[i] = 0;
    
    components[CMP_TRANSFORM_3D] = new Transform3d();
    GetTransform()->entity = this;
    setType(ETYPE_DEFAULT);
    octreeNode = 0;    
}
Entity::~Entity()
{
    bool hadRb = false;
    //axe all these components
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        if (components[i])
        {
            switch(i)
            {
                case CMP_TRANSFORM_3D:
                    delete (Transform3d *)components[i];
                break;
                case CMP_MESH:
                    Renderer::GetInstance()->RemoveMesh(GetMesh());
                    delete (Mesh *)components[i];
                break;
                case CMP_SPRITE:
                    Renderer::GetInstance()->RemoveSprite(GetSprite());
                    delete GetSprite();
                    break;
                case CMP_LOGIC:
                    //ObjectManager::GetInstance()->Remove((Logic *)components[i]);
                    GetLogic()->Destroy();
                    delete (Logic *)components[i];
                break;
                case CMP_RIGIDBODY:
                    hadRb = true;
                    
                    if (Physics::GetInstance()->RemoveRigidBody((RigidBody *)components[i]))
                    delete (RigidBody *)components[i];
                    
                break;
                case CMP_SKELETAL_MESH:
                    delete (SkeletalMesh *)components[i];
                break;
                case CMP_ANIMATION_CONTROLLER:
                    delete (AnimationController *)components[i];
                break;
                case CMP_COLLISION_SHAPE:
                    if (!hadRb)
                    {
                        // no rigid body, so it was just a simple shape
                        Physics::GetInstance()->RemoveCollisionShape((CollisionShape *)components[i]);
                        delete (CollisionShape *)components[i];
                    }
                break;
                case CMP_CHARACTER_CONTROLLER:
                    delete (CharacterController *)components[i];
                break;
                case CMP_PARTICLE_SYSTEM:
                    Renderer::GetInstance()->RemoveParticleSystem((ParticleSystem *)components[i]);
                    delete (ParticleSystem *)components[i];
                break;
                default:
                    LogError("Deleting unknown component, skipped");
                break;  
            }
        }
    }
    
    if (blueprint)
        blueprint->RemoveRef();
    
}


bool Entity::AttachComponent (ComponentTypes type, void *data)
{
    //TODO obviously this needs to be changed to a memory managed allocator
    //to ensure contiguous memory and maximize cache hits
    switch(type)
    {
        case CMP_TRANSFORM_3D:
        {
            //they all gottem now
            return true;
        }    
        break;
        
        case CMP_SPRITE:
        {
            SpriteConstructor *ctr = static_cast<SpriteConstructor *>(data);
            Sprite *sprite = new Sprite(this, ctr->spriteResource, ctr->spriteType);
            components[CMP_SPRITE] = sprite;
            Renderer::GetInstance()->AddSprite(sprite);
            return true;
            break;
        }
        
        //void * is a mesh
        case CMP_MESH:
        {
            //try to load the model
           
            Mesh *newMesh = static_cast<Mesh *>(data);
            components[CMP_MESH] = newMesh;
            if (!newMesh)
            {
                //failure
                LogError("Mesh is null, nothing attached.");
                return false;
            }
            else
            {
                Renderer::GetInstance()->AddMesh(newMesh);
                newMesh->entity = this;
                newMesh->transform = this->GetTransform();

                Vector3 min, max;
                min.x = newMesh->GetMinX();
                min.y = newMesh->GetMinY();
                min.z = newMesh->GetMinZ();
                max.x = newMesh->GetMaxX();
                max.y = newMesh->GetMaxY();
                max.z = newMesh->GetMaxZ();
                //overrides radius to match mesh
                GetTransform()->radius = radiusFromAABB(min, max);
                
                return true;
            }
        }   
        break;
        
        //TODO  should probably do some error checking
        case CMP_SKELETAL_MESH:
        {
           // SkinnedMesh *newMesh = new SkinnedMesh(this, GetTransform(),  (AnimatedModelResource *)data );
       
            SkinnedMeshConstructor *meshConstruct = static_cast<SkinnedMeshConstructor *>(data);
            SkinnedMesh *newMesh = new SkinnedMesh(this, GetTransform(), meshConstruct->animatedModel, meshConstruct->mat);
           // SkinnedMesh *newMesh = new SkinnedMesh(this, GetTransform(), static_cast<AnimatedModelResource *>(data));
           // newMesh->material = new Material( TODO figure the materials fiasco out for these guys
            //right now its hardcoded in the resource load
           
            components[CMP_SKELETAL_MESH] = newMesh;
            if (!newMesh)
            {
                LogError("Error attaching skeletal mesh");
               // return false;
            }     
            else
            {
                newMesh->UpdatePose();//loads bind pose
                newMesh->entity = this;
                newMesh->transform = this->GetTransform();
                //add all the meshes
                for (int i = 0; i < newMesh->numMeshes; i++)
                {
                    //link all the child transforms and materials for renderer
                    newMesh->meshData[i].transform = newMesh->transform;
                    newMesh->meshData[i].entity = this;
                    newMesh->meshData[i].material = newMesh->material;
                    
                    Renderer::GetInstance()->AddMesh(&newMesh->meshData[i]);
                }
                
                return true;
            }
        }
        break;
        
        
        case CMP_ANIMATION_CONTROLLER:
        {
            SkeletalMesh * skeleMesh = GetSkeletalMesh();
            if (!skeleMesh)
            {
                LogError("Entity requires skeletal mesh for animation controller!");
                return false;
            }
            AnimationControllerConstructor *ac = static_cast<AnimationControllerConstructor *>(data);
            AnimationController *newController = new AnimationController(this, GetTransform(), skeleMesh);
            components[CMP_ANIMATION_CONTROLLER] = newController;
            newController->entity = this;
            newController->transform = this->GetTransform();
            
            //load the animations, if there are any...
            if (ac)
            {
                vector<AnimationResource*>::iterator iter = ac->animations.begin();
                while (iter != ac->animations.end())
                {
                    newController->AddAnimation((*iter));
                    iter++;
                }
            }
	    return true;
        }
        break;
        
        //data = rb constructor!
        case CMP_RIGIDBODY:
        {
            RigidBodyConstructor *rc = static_cast<RigidBodyConstructor *>(data);
            _is3d = rc->shape->is3d();
            RigidBody *rbComponent = Physics::GetInstance()->CreateNewRigidBody(this, GetTransform(), rc->shape->shape, rc->mass,
                                    rc->linearDrag, rc->angularDrag, rc->friction, rc->rollingFriction, rc->restitution);
            //HACK because of how builder constructs rigidbodies that depend on shapes that havent been attached yet
            rbComponent->is3d = rc->shape->is3d();
            //the shape component expects to know about the collision object
            rbComponent->angularDrag = rc->angularDrag;
            rbComponent->linearDrag = rc->linearDrag;
            rc->shape->collisionObject = rbComponent->rigidBody;   
            components[CMP_RIGIDBODY] = rbComponent;            
            Physics::GetInstance()->AddRigidBody(rbComponent);
            rbComponent->rigidBody->setUserPointer(this);
            rbComponent->entity = this;
            rbComponent->transform = this->GetTransform();
            return true;
        }
        break;
        
        //NOTE this one just takes a shape, pretty f'n simple
        case CMP_COLLISION_SHAPE:
        {
            CollisionShape *shapeComponent = static_cast<CollisionShape *>(data);
            components[CMP_COLLISION_SHAPE] = shapeComponent;
            //Physics::GetInstance()->
            shapeComponent->collisionObject->setCollisionShape(shapeComponent->shape);
            shapeComponent->collisionObject->setUserPointer(this);
            shapeComponent->collisionObject->getWorldTransform().setOrigin(btVector3(GetTransform()->position.x, 
                                                                            GetTransform()->position.y, GetTransform()->position.z));
            shapeComponent->entity = this;
            shapeComponent->transform = this->GetTransform();
            return true;
        }
        break;
       
        case CMP_CHARACTER_CONTROLLER:
        {
            CharacterController *controller = static_cast<CharacterController *>(data);
            components[CMP_CHARACTER_CONTROLLER] = controller;

            controller->GetCollisionObject()->setUserPointer(this);
            controller->entity = this;
            controller->transform = this->GetTransform();
                Physics::GetInstance()->AddCharacter(controller);
            return true;
        }
        break;
        
        case CMP_LOGIC:
        {
             //data is the component we're attaching
            Logic *logic = static_cast<Logic *>(data);
            components[CMP_LOGIC] = logic;
            logic->entity = this;
            logic->transform = this->GetTransform();
             
            logic->Init();
            if (logic->needsUpdate())
                ObjectManager::GetInstance()->Add(logic);
#if defined(PC_PLATFORM) && !defined(__APPLE__)
             //if (logic->scriptName.length())
               // scriptObject = Scripting::GetInstance()->CreateScriptObject(logic->scriptName, this);
#endif             
             return true;
        }
        break;

        case CMP_PARTICLE_SYSTEM:
        {
            ParticleSystem *p = new ParticleSystem((ShaderResource *)data, this);
            components[CMP_PARTICLE_SYSTEM] = p;
            Renderer::GetInstance()->AddParticleSystem(p);
            return true;
        }
        break;
        
        default:
            return false;
        break;
    }
    
    return false;
}

void Entity::UpdateChildTransforms()
{
    if (children.size() == 0)
        return;

    SLList<Entity *>::iterator iter = children.getIterator();
    Vector3 pos = GetTransform()->ExtractPosition();
    Quaternion rot = GetTransform()->rotation;
    while(iter.isGood())
    {
        iter.get()->GetTransform()->ParentUpdate(pos, rot);
        iter.next();
    }
}


void Entity::AttachEntity( Entity* child, Vector3 offset)
{
    children.push_front(child);
    child->GetTransform()->position = offset;
    
    GetTransform()->hasChildren = true;
    child->GetTransform()->motionState->disabled = true;
}


