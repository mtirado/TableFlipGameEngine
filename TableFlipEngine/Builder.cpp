/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Builder.h"
#include "Entity.h"
#include "Components.h"
#include "Renderer.h"
//#include "Lander.h"
#include <stdio.h>
//#include <MiscLogic.hpp>
//#include <EnemySpawn.hpp>
Builder *Builder::instance = 0;

Builder::Builder()
{
 
}
Builder::~Builder()
{}

void Builder::InitInstance()
{
    if (!instance)
        instance = new Builder;
}

void Builder::DeleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

Entity* Builder::ConstructEntity(BlueprintResource *bp, Vector3 position)
{
    //i suppose i'm going to have the blueprint be a proxy reference counter for resources
    //meaning every instantiated object will add ref to the blueprint, which indirectly counts as
    //a ref towards each resource the blueprint contains.  bad idea? too bad its quick to implement that way.

    Entity *entity = new Entity;
    entity->setType(ETYPE_GENERIC);
    Material *mat = &Renderer::GetInstance()->defaultMaterial;
    CollisionShape *shape = 0;
    
    entity->GetTransform()->position = position;
    entity->GetTransform()->Update();
    
    //mesh requires a material, could be a default
    if (bp->materialConstructor)
    {
        MaterialConstructor *mc = bp->materialConstructor;
        mat = new Material;
        mat->LoadShader(mc->shader);  //this will add a reference to the shader for us
        
        //NOTE i guess these names are getting set in stone?
        //could still add them to the blueprint if desired
        mat->SetEntity(entity);
        //TODO this is getting extremely bad, this whole loading files thing...
        if (bp->spriteConstructor)
            mat->RegisterUniform("WorldMatrix", SV_ENTITY_TRANSFORM, (void *)1);
        else
            mat->RegisterUniform("WorldMatrix", SV_ENTITY_TRANSFORM, 0);
        mat->RegisterViewMatrix("ViewMatrix");
        mat->RegisterProjectionMatrix("ProjectionMatrix");
        //this is HACK 'd, only 1 light right now - the sun.
       // mat->RegisterUniform("lightPos", SV_FLOAT3, &Renderer::GetInstance()->ptLight.position);
        
        //register the uniforms
        for (int i = 0; i < mc->numLinks; i++)
        { 
            //TODO  it appears i didnt think about individual data locations, right now they are all shared
            //in the material constructor obj, one solution may be to have a vector of pointers in the material
            //instance but i am far too tired to implement that right now, and possibly a better solution lurks.
            //TODO one things for sure, this must be fixed!!!!!1!!111!
            mat->RegisterUniform(mc->uniformLinks[i].name, mc->uniformLinks[i].type, mc->uniformLinks[i].data);
        }
    }
    if (bp->meshConstructor)
    {
        Mesh *mesh = new Mesh(entity, entity->GetTransform(), bp->meshConstructor->mdl, mat);
        entity->AttachComponent(CMP_MESH, mesh);
    }
    if (bp->skMeshConstructor)
    {
        //TODO make sure these materials arent leaking memory, im not sure anymore....
        //SkeletalMesh *mesh = new SkinnedMesh(entity, entity->GetTransform(), bp->skMeshConstructor->animatedModel);
        bp->skMeshConstructor->mat = mat;
        entity->AttachComponent(CMP_SKELETAL_MESH, bp->skMeshConstructor);
    }
    if (bp->animControllerConstructor)
    {
        entity->AttachComponent(CMP_ANIMATION_CONTROLLER, bp->animControllerConstructor);
    }
    if (bp->shapeConstructor)
    {
        
        if (bp->shapeConstructor->type == SHAPE_BOX)
            shape = new BoxShape(bp->shapeConstructor->boxHalfWidths, bp->shapeConstructor->isTrigger, !bp->rbConstructor);
        else if (bp->shapeConstructor->type == SHAPE_SPHERE) //TODO sphere collider on a trigger CRASHES!!
            shape = new SphereShape(bp->shapeConstructor->sphereRadius, bp->shapeConstructor->isTrigger, !bp->rbConstructor);
        else if (bp->shapeConstructor->type == SHAPE_MESH) //TODO mesh collider will not work on animated / dynamic meshes
            shape = new MeshShape(bp->meshConstructor->mdl, !bp->meshConstructor->staticCollisionMesh, bp->shapeConstructor->isTrigger,  !bp->rbConstructor);
        else if (bp->shapeConstructor->type == SHAPE_BOX2D)
            shape = new Box2dShape(bp->shapeConstructor->boxHalfWidths, bp->shapeConstructor->isTrigger, !bp->rbConstructor);
        else
        {
            LogError("UNKNOWN COLLISION SHAPE!?");
            shape = 0;
        }
        
    }
    if (bp->rbConstructor) //NOTE no idea whats going ot happen if we have a RB and a char controller...
    {
        if (shape == 0)
            LogError("no collision shape, cannot attach rigid body.");
        else
        {
            bp->rbConstructor->shape = shape; //need to attach previously created shape!
            
            entity->AttachComponent(CMP_RIGIDBODY, bp->rbConstructor);
        }
    }
    if (bp->charConstructor)
    {
        //TODO put this at the transform location
        btTransform startTransform;
        startTransform.setIdentity ();
        startTransform.setOrigin (btVector3(position.x, position.y, position.z));
        btPairCachingGhostObject *ghostObject = new btPairCachingGhostObject();
        ghostObject->setWorldTransform(startTransform);
        // sweepBP->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
        btScalar characterHeight = bp->charConstructor->height;
        btScalar characterWidth = bp->charConstructor->width;
        btScalar stepHeight = bp->charConstructor->stepHeight;
        btConvexShape *capsule = new btCapsuleShape(characterWidth,characterHeight);
        ghostObject->setCollisionShape (capsule);
        ghostObject->setCollisionFlags (ghostObject->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT |
                                                                        btCollisionObject::CF_KINEMATIC_OBJECT );
        //ghostObject->setCollisionFlags();
        
        CharacterController *controller = new CharacterController(ghostObject, capsule, stepHeight);
        controller->s_yOffset = bp->charConstructor->yOffset;
        controller->s_zOffset = bp->charConstructor->zOffset;
        entity->AttachComponent(CMP_CHARACTER_CONTROLLER, controller);
    }
    if (bp->logicScript != "")
    {
        //TODO this is HACK-ey McHacktown just to get the game running
        //without scripts, NEEDS A GOOD FLEXIBLE SOLUTION
        Logic *logic = 0; 
        //if (bp->logicScript == "Worker")
        //    logic = new Worker();
        //else if (bp->logicScript == "Lander")
        //   logic = new Lander();
        //else if (bp->logicScript == "EnemySpawn")
        //    logic = new EnemySpawn();
        
        if (logic)
            entity->AttachComponent(CMP_LOGIC, logic);
    }
    //late shape attach, this logic is FUCKED!!!!!
    if (bp->shapeConstructor)
        entity->AttachComponent(CMP_COLLISION_SHAPE, shape);
    //we have a shape, but no RB. this is a regular collision object.
    if (entity->GetCollisionShape() && !entity->GetRigidBody())
        Physics::GetInstance()->AddCollisionShape(shape);
    
    //sprite requires a shape and material
    if (bp->spriteConstructor)
    {
        //NOTE at this time, sprite names shadow the blueprint name
        
        //HACK, this is a lazy way to check if sprite resource is already loaded
        //untill i redesign this whole mess and add working shared materials
        bp->spriteConstructor->spriteResource = LoadSpriteResource(bp->name, 0, bp->spriteConstructor);
        if (bp->spriteConstructor->spriteResource)
        {
            delete mat; // the above material load was useless and wasted cpu time
            entity->AttachComponent(CMP_SPRITE, bp->spriteConstructor);
        }
        else
        {
            CheckGLErrors();
            //didnt exist, use the new material
            bp->spriteConstructor->spriteResource = LoadSpriteResource(bp->name, mat, bp->spriteConstructor );
         
            entity->AttachComponent(CMP_SPRITE, bp->spriteConstructor);
          
        }
    }
    
    //animation controller requires skeletal mesh
    
    //rigidbody requires collision shape
    
    //if shape and no rigidbody, set proper bool in collision shape creation? i think
   
    entity->SetBlueprint(bp);
    bp->AddRef(); //entity will remove ref if it has a bleprint
    bp->instances++;
    
    std::ostringstream buff;
    if (bp->instances != 1) //first one has no number, just the name
    {
        buff << bp->name.c_str() << bp->instances;
        entity->name = buff.str();
    }
    else
        entity->name = bp->name;

    return entity;
}



/*      Blueprint File Format
  CMP_MESH, CMP_SKELETAL_MESH, CMP_ANIMATION_CONTROLLER, 
CMP_RIGIDBODY, CMP_COLLISION_SHAPE, CMP_LOGIC, CMP_CHARACTER_CONTROLLER,
  
    Entity     - the root entity TODO add sub entities!
    {
        Name "name"  
                        ///0 means no static collision mesh, 1 means static collision!
        Mesh "filename" 0
        SkeletalMesh "filename"
        NOTE: i think logic will be dev feature only
        Logic "scriptName"
        Tilemap "filename"
        Sprite 1 //num frames, must be at least one
        [
            origin x y //in pixels
            dimensions x y //in screen % 0-100
            framesize x y  //in pixels
            rows 1
            cols 1
            texturesize 1024 //size of the atlas
        ]
        Material
        [
            "shaderfile.shader"
            UniformLinks 2
            "name" SV_SAMPLER2D 
            "textureFile.png" texStage (1/0 - repeat/clamp)
            "name" SV_FLOAT4 
            1.0 1.0 1.5 7.4
        ]
        AnimationController
        [
            numanims 2
            "animName" "animFile"
            "animName" "animFile"
        ]
        RigidBody
        [
            mass 5.0
            lineardrag 0.0
            angulardrag 0.0
            friction 0.5
            rollingfriction 0.0
            restitution 0.0
        ]
        CollisionShape
        [
            type "sphere" "box" or "mesh"  TODO: add others
            trigger 0/1   yay or nay
            data radius   vector3   null
            
            NOTE: only provide data for the type
        ]      
        CharacterController
        [
            width 1.0
            height 2.0
            stepheight 0.3
            yoffset 0.1f
            zoffset 0.1f
        ] 
    }
 
 
 */

static char buff[512];
static char typeBuff[64];
static char nameBuff[256];
static int intRead[4];
static float floatRead[8]; //TODO matrices for parented transforms will be needed..
//static int priority;

MaterialConstructor *ReadMaterial(FILE *file, int priority)
{
    std::string shaderFile;
    
     //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    //read shader file
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    if (sscanf(buff, " %s", nameBuff) != 1)
        return 0;
    else
        shaderFile = nameBuff;
    
    //
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    //read number of links
    if (sscanf(buff, " UniformLinks %i", &intRead[0]) == 1)
    {
        if (intRead[0] < MAX_UNIFORM_LINKS)
        {
            MaterialConstructor *mc = new MaterialConstructor;
            mc->numLinks = intRead[0];
            if (mc->numLinks > 0)
                mc->uniformLinks = new UniformLinkConsructor[mc->numLinks];

            mc->shader = LoadShaderResource(shaderFile.c_str(), priority);

            //read each link
            for (int i = 0; i < mc->numLinks; i++)
            {
                if (!fgets(buff, sizeof(buff), file))
                {
                    delete mc;
                    return 0;
                }
                if (sscanf(buff, " %s %i",nameBuff, &intRead[0]) == 2)
                {
                    if (intRead[0] >= 0 && intRead[0] < NUM_SVARS)
                    {                                
                        mc->uniformLinks[i].name = nameBuff;
                        mc->uniformLinks[i].type = (ShaderVariables)intRead[0];
                        mc->uniformLinks[i].data = 0;
                        
                        //no data, just tells us to send the current time in seconds to shader
                        if (mc->uniformLinks[i].type == SV_TIME_SECONDS)
                            continue;
                        //read data line
                        if (!fgets(buff, sizeof(buff), file))
                        {
                            delete mc;
                            return 0;
                        }
                        switch (mc->uniformLinks[i].type)
                        {
                            case SV_FLOAT:  //TODO change type from int value, to text, so it makes sense to the 
                            { 
                                if (sscanf(buff, " %f", &floatRead[0]) == 1)
                                {
                                    mc->uniformLinks[i].data = new float[1];
                                    ((float *)mc->uniformLinks[i].data)[0] = floatRead[0];
                                }
                                else
                                {
                                    LogError("Error reading uniform float1 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_FLOAT2:
                            {
                                if (sscanf(buff, " %f %f", &floatRead[0], &floatRead[1]) == 2)
                                {
                                    mc->uniformLinks[i].data = new float[2];
                                    ((float *)mc->uniformLinks[i].data)[0] = floatRead[0];
                                    ((float *)mc->uniformLinks[i].data)[1] = floatRead[1];
                                }
                                else
                                {
                                    LogError("Error reading uniform float2 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_FLOAT3:
                            {    
                                if (sscanf(buff, " %f %f %f", &floatRead[0], &floatRead[1], &floatRead[2]) == 3)
                                {
                                    mc->uniformLinks[i].data = new float[3];
                                    ((float *)mc->uniformLinks[i].data)[0] = floatRead[0];
                                    ((float *)mc->uniformLinks[i].data)[1] = floatRead[1];
                                    ((float *)mc->uniformLinks[i].data)[2] = floatRead[2];
                                }
                                else
                                {
                                    LogError("Error reading uniform float 3 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_FLOAT4:
                            {    
                                if (sscanf(buff, " %f %f %f %f", &floatRead[0], &floatRead[1], &floatRead[2], &floatRead[3]) == 4)
                                {
                                    mc->uniformLinks[i].data = new float[4];
                                    ((float *)mc->uniformLinks[i].data)[0] = floatRead[0];
                                    ((float *)mc->uniformLinks[i].data)[1] = floatRead[1];
                                    ((float *)mc->uniformLinks[i].data)[2] = floatRead[2];
                                    ((float *)mc->uniformLinks[i].data)[3] = floatRead[3];
                                }
                                else
                                {
                                    LogError("Error reading uniform float 4 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            /*case SV_MAT2:
                            {
                            }
                            break;
                            case SV_MAT3:
                            {    
                            }
                            break;
                            case SV_MAT4:
                            {    
                            }
                            break;*/
                            case SV_INT:
                            {
                                if (sscanf(buff, " %i", &intRead[0]) == 1)
                                {
                                    mc->uniformLinks[i].data = new int[1];
                                    ((int *)mc->uniformLinks[i].data)[0] = intRead[0];
                                }
                                else
                                {
                                    LogError("Error reading uniform int1 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_INT2:
                            {    
                                if (sscanf(buff, " %i %i", &intRead[0], &intRead[1]) == 2)
                                {
                                    mc->uniformLinks[i].data = new int[2];
                                    ((int *)mc->uniformLinks[i].data)[0] = intRead[0];
                                    ((int *)mc->uniformLinks[i].data)[1] = intRead[1];
                                }
                                else
                                {
                                    LogError("Error reading uniform int2 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_INT3:
                            {
                                if (sscanf(buff, " %i %i %i", &intRead[0], &intRead[1], &intRead[2]) == 3)
                                {
                                    mc->uniformLinks[i].data = new int[3];
                                    ((int *)mc->uniformLinks[i].data)[0] = intRead[0];
                                    ((int *)mc->uniformLinks[i].data)[1] = intRead[1];
                                    ((int *)mc->uniformLinks[i].data)[2] = intRead[2];
                                }
                                else
                                {
                                    LogError("Error reading uniform int3 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_INT4:
                            {    
                                if (sscanf(buff, " %i %i %i %i", &intRead[0], &intRead[1], &intRead[2], &intRead[3]) == 4)
                                {
                                    mc->uniformLinks[i].data = new int[4];
                                    ((int *)mc->uniformLinks[i].data)[0] = intRead[0];
                                    ((int *)mc->uniformLinks[i].data)[1] = intRead[1];
                                    ((int *)mc->uniformLinks[i].data)[2] = intRead[2];
                                    ((int *)mc->uniformLinks[i].data)[3] = intRead[3];
                                }
                                else
                                {
                                    LogError("Error reading uniform int4 data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                            }
                            break;
                            case SV_SAMPLER2D: 
                            {
                                if (sscanf(buff, " %s %i %i", nameBuff, &intRead[1], &intRead[2]) == 3)
                                {
                                    //TODO  options, these options at the moment are for PNG files
                                    unsigned int flagRepeat = 0;
                                    if (intRead[2] == 1)
                                        flagRepeat = SOIL_FLAG_TEXTURE_REPEATS;
                                    mc->uniformLinks[i].data = LoadTexture(nameBuff, /*SOIL_LOAD_AUTO*/SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
                                    SOIL_FLAG_INVERT_Y | /*SOIL_FLAG_MIPMAPS |*/ flagRepeat, priority);
                                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                                    if (mc->uniformLinks[i].data == 0)
                                    {
                                        LogError("Could not load texture in blueprint, check filename");
                                    }
                                }
                                else
                                {
                                    LogError("Error reading uniform sampler2d data, check syntax");
                                    delete mc;
                                    return 0;
                                }
                                                        
                                
                            }
                            break;   
                            case SV_TIME_SECONDS:
                            {
                                mc->uniformLinks[i].data = 0;
                                LogError("shader loader should never reach this line! something is fudged up");
                            }
                            break;
                            
                            default:
                                break;
                        }
                            
                        
                    }
                    else
                        LogError("Invalid UniformLink Type");
                    
                /*Material
                [
                    UniformLinks 2
                    "name" SV_SAMPLER2D 
                    "textureFile.png" texStage
                    "name" SV_FLOAT4 
                    1.0 1.0 1.5 7.4
                ]*/
                }
                else
                    LogError("Could not read uniform link type");
            }
            
            //eat bracket;
            fgets(buff, sizeof(buff), file);
            return mc;
        }
        else
            LogError("Too many uniform links!");
        
    }
    else
        LogError("Error reading material uniform links");   
    
    return 0;
}

AnimationControllerConstructor *ReadAnimController(FILE *file, int priority)
{
     //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
   
    int numAnims;
   
    //read number of anims
    
     if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " numanims %i", &intRead[0]) == 1)
    {
        numAnims = intRead[0];
        if (numAnims > MAX_ANIMS)
        {
            LogError("too many anims!");
            return 0;
        }
        
        AnimationControllerConstructor *ac = new AnimationControllerConstructor;
        for (int i = 0; i < numAnims; i++)
        {
            //read each anim
            if (!fgets(buff, sizeof(buff), file))     
            {
                delete ac;
                return 0;
            }
            
            //TODO will break if name is grater than typebuff  [64] ??
            //typebuff is name, namebuff is file
            if (sscanf(buff, " %s %s", typeBuff, nameBuff) == 2)
            {
                AnimationResource *ar = LoadAnimation(nameBuff, priority);
                ar->name = typeBuff;
                if (ar == 0)
                    LogError("Couldnt load animation from blueprint");
                else
                    ac->animations.push_back(ar);                    
                
            }
            else
                LogError("error reading anim info");               
        }
        
        //eat bracket;
        fgets(buff, sizeof(buff), file);
        return ac;
    }
    else
        LogError("Could not read num anims");
        /*AnimationController
        [
            NumAnims: 2
            "animName" "animFile"
            "animName" "animFile"
        ]*/
    return 0;
}

RigidBodyConstructor *ReadRigidbody(FILE *file)
{
    
    //default values, will return even if the read fails
    RigidBodyConstructor *rbc = new RigidBodyConstructor;
    
    
    int numProperties = 6;
    rbc->mass = 10.0f;
    rbc->linearDrag = 0.0f;
    rbc->angularDrag = 0.0f;
    rbc->friction = 0.5f;
    rbc->rollingFriction = 0.0f;
    rbc->restitution = 0.0f;
    
    //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    for (int i = 0; i < numProperties + 1; i++) // +1 to read end bracket
    {
        //read next line
        if (!fgets(buff, sizeof(buff), file))   
        {
            delete rbc;
            return 0;
        }
        //check for end bracket, if it only read 1, its a bracket 
        if (sscanf(buff, " %s %f", typeBuff, &floatRead[0]) == 1)
        {
            return rbc;
        }
        else if (sscanf(buff, " %s %f", typeBuff, &floatRead[0]) == 2)
        {
            if (!strcmp(typeBuff, "mass")) { rbc->mass = floatRead[0]; }
            else if (!strcmp(typeBuff, "lineardrag")) { rbc->linearDrag = floatRead[0]; }
            else if (!strcmp(typeBuff, "angulardrag")) { rbc->angularDrag = floatRead[0]; }
            else if (!strcmp(typeBuff, "friction")) { rbc->friction = floatRead[0]; }
            else if (!strcmp(typeBuff, "rollingfriction")) { rbc->rollingFriction = floatRead[0]; }
            else if (!strcmp(typeBuff, "restitution")) { rbc->restitution = floatRead[0]; }
        }
        else
        {
            LogError("Problem reading rigid body properties");
            delete rbc;
            return 0;
        }
    }
    
    LogWarning("syntax error, did not find end bracket (]) for rigid body properties, returning null. fix that junk.");
    delete rbc;
    return 0;
    /*  RigidBody
        [
            mass: 5.0
            lineardrag: 0.0
            angulardrag: 0.0
            friction: 0.5
            rollingfriction: 1.0
            restitution: 0.0
        ]*/
}

CollisionShapeConstructor *ReadCollisionShape(FILE *file)
{
     //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
  
    //read type line
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    if (sscanf(buff, " type %s", typeBuff) != 1)
        return 0;
    
    
    //trigger or not?
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    if (sscanf(buff, " trigger %i", &intRead[0]) != 1)
    {
        LogError("Could not read trigger property");
        return 0;
    }
    
    CollisionShapeConstructor *sc = new CollisionShapeConstructor;
    sc->isTrigger = intRead[0];
    //read shape data
    if (!strcmp(typeBuff, "box")) 
    {
        //read half extents
        //read type line
        if (!fgets(buff, sizeof(buff), file)) 
        {
            delete sc;
            return 0;
        }
        if (sscanf(buff, " %f %f %f", &floatRead[0], &floatRead[1], &floatRead[2]) == 3)
        {
             //eat bracket;
            fgets(buff, sizeof(buff), file);
            sc->type = SHAPE_BOX;
            sc->boxHalfWidths.x = floatRead[0];
            sc->boxHalfWidths.y = floatRead[1];
            sc->boxHalfWidths.z = floatRead[2];
            return sc;
        }
        else
            LogError("Error reading box shape extents");
        
    }
    else if (!strcmp(typeBuff, "box2d")) 
    {
        //read half extents
        //read type line
        if (!fgets(buff, sizeof(buff), file)) 
        {
            delete sc;
            return 0;
        }
        if (sscanf(buff, " %f %f", &floatRead[0], &floatRead[1]) == 2)
        {
             //eat bracket;
            fgets(buff, sizeof(buff), file);
            sc->type = SHAPE_BOX2D;
            sc->boxHalfWidths.x = floatRead[0];
            sc->boxHalfWidths.y = floatRead[1];
            sc->boxHalfWidths.z = 0.0f;
            return sc;
        }
        else
            LogError("Error reading box2d shape extents");
        
    }
    else if (!strcmp(typeBuff, "sphere")) 
    {
        if (!fgets(buff, sizeof(buff), file))     
        {
            delete sc;
            return 0;
        }
        if (sscanf(buff, " %f", &floatRead[0]) == 1)
        {
            //eat bracket;
            fgets(buff, sizeof(buff), file);
            sc->type = SHAPE_SPHERE;
            sc->sphereRadius = floatRead[0];
            return sc;
        }
        else
            LogError("Error reading sphere radius");
    }
    else if (!strcmp(typeBuff, "mesh")) 
    {
        //eat bracket;
        fgets(buff, sizeof(buff), file);
        sc->type = SHAPE_MESH;
        return sc;
    }
    else
    {
        LogError("Unkown shape type??");
    }
    /*CollisionShape
        [
            type "sphere" "box" or "mesh"  TODO: add others
            data radius   vector3   null
            NOTE: only provide data for the type
        ]*/
    
    return 0;
}

CharacterControllerConstructor *ReadCharController(FILE *file)
{
    //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    
    //width
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " width %f", &floatRead[0]) != 1)
        return 0;
    
    //height
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " height %f", &floatRead[1]) != 1)
        return 0;
    
    //step height
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " stepheight %f", &floatRead[2]) != 1)
        return 0;
    
     //up offset
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " yoffset %f", &floatRead[3]) != 1)
        return 0;
    
    //fwd offset
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " zoffset %f", &floatRead[4]) != 1)
        return 0;
    
    
    
    //eat bracket
    fgets(buff, sizeof(buff), file);
    
    CharacterControllerConstructor *cc = new CharacterControllerConstructor;
    cc->width = floatRead[0];
    cc->height = floatRead[1];
    cc->stepHeight = floatRead[2];
    cc->yOffset = floatRead[3];
    cc->zOffset = floatRead[4];
    return cc;
     /* CharacterController
        [
            width 1.0;
            height 2.0;
            stepheight 0.3;
        ]*/
}


//generates texcoords for the sprite,
//if you are creating sprites manually you will need to generate texcoords!
SpriteConstructor *ReadSprite(FILE *file, int numFrames)
{
    //enter bracket
    if (!fgets(buff, sizeof(buff), file))     
        return 0; 
    
    int originX, originY;
    int sizeX, sizeY;
    int rows, cols;
    int textureSize;
    int type;

    //sprite type  0 normal, 1 stencil, 2 clipped
    if (!fgets(buff, sizeof(buff), file))
        return 0;
    if (sscanf(buff, " type %i", &type) != 1)
        return 0;
    //origin
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " origin %i %i", &originX, &originY) != 2)
        return 0;
     //dimensions
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " dimensions %f %f", &floatRead[0], &floatRead[1]) != 2)
        return 0;
     //offset
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " offset %f %f", &floatRead[2], &floatRead[3]) != 2)
        return 0;
    //frame size
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " framesize %i %i", &sizeX, &sizeY) != 2)
        return 0;
    //rows
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " rows %i", &rows) != 1)
        return 0;
    //cols
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " cols %i", &cols) != 1)
        return 0;
    //texture atlas size
    if (!fgets(buff, sizeof(buff), file))     
        return 0;
    if (sscanf(buff, " texturesize %i", &textureSize) != 1)
        return 0;
    
    SpriteConstructor *spriteConstructor = new SpriteConstructor();
    spriteConstructor->dimensions.x = floatRead[0];
    spriteConstructor->dimensions.y = floatRead[1];
    spriteConstructor->offset.x = floatRead[2];
    spriteConstructor->offset.y = floatRead[3];
    spriteConstructor->numFrames = numFrames;
    if (numFrames == 1)
        spriteConstructor->frames = new SpriteFrame;
    else if (numFrames > 1)
        spriteConstructor->frames = new SpriteFrame[numFrames];
    else
        return 0;
    int curRow = 0;
    int curCol = 0;
    for (int i = 0 ; i < numFrames; i++)
    {
        if (curCol == cols)
        {
            curRow++;
            curCol = 0;
        }
        spriteConstructor->frames[i].dimensions = Vector2((float)sizeX / textureSize, (float)sizeY / textureSize);
        
        
        spriteConstructor->frames[i].texcoords = Vector2( (originX + (curCol * (float)sizeX)) / textureSize, 
                                                          (originY + (curRow * (float)sizeY)) / textureSize);   
        curCol++;
    }
    spriteConstructor->animated = spriteConstructor->numFrames > 1;
      
    //eat bracket
    fgets(buff, sizeof(buff), file);

    //set type  -- defaults to normal if invalid
    if (type == 1)
        spriteConstructor->spriteType = SPRITE_TYPE_STENCILS;
    else if (type == 2)
        spriteConstructor->spriteType = SPRITE_TYPE_CLIPPED;
    else
        spriteConstructor->spriteType = SPRITE_TYPE_NORMAL;
    
    return spriteConstructor;
}

//TODO add nested entities / transform parenting
BlueprintResource* Builder::CreateBlueprint(const char* filename, int _priority, ResourceLoadMemInfo *mem)
{

#if defined(__ANDROID__)
    if (!mem) return 0;
    FILE * file = fmemopen(mem->mem, mem->size, "r");
#else
    FILE * file = fopen(filename, "r");
#endif
    if( file == NULL )
    {
        LogError("CreatBlueprint() : cannot open file !\n");
        return 0;
    }
    
   
    char c;
    //seek to the first entity {
    do 
    {
        c = fgetc(file);
    }
    while(c != '{' && c != EOF);
    //clear the line, move into bracket
    if (!fgets(buff, sizeof(buff), file))
        return 0;
    
    //analyze component type,
    //switch to read that type
    //read header data
    //if list, read list elements
    ComponentTypes type;
    BlueprintResource *bp = new BlueprintResource(_priority);
    bool reading = true;
    while(reading)
    {
        bool tilemap = false; //for sprite component
        //eat a line
        if (!fgets(buff, sizeof(buff), file))
            return 0;
        
        //check for end bracket }
        for (int i = 0; i < sizeof(buff); i++)
        {
            if (buff[i] == '\n')
                break;
            if (buff[i] == '}')
            {
                reading = false;
                break;
            }
        }
        
       //TODO i guess i need to make an entity file, 
       //change all the resource loaders to use fixed directories, and give this a go
        if (reading)
        {
        //check component type
            if (sscanf(buff, " %s", typeBuff) != 1)
            {
                LogError("Error reading component type");
                continue;
            }
            if (!strcmp(typeBuff, "Mesh")) { type = CMP_MESH; }
            else if (!strcmp(typeBuff, "Material")) { type = CMP_MATERIAL; }
            else if (!strcmp(typeBuff, "Logic")) { type = CMP_LOGIC; }
            else if (!strcmp(typeBuff, "SkeletalMesh")) { type = CMP_SKELETAL_MESH; }
            else if (!strcmp(typeBuff, "AnimationController")) { type = CMP_ANIMATION_CONTROLLER; }
            else if (!strcmp(typeBuff, "RigidBody")) { type = CMP_RIGIDBODY; }
            else if (!strcmp(typeBuff, "CollisionShape")) { type = CMP_COLLISION_SHAPE; }
            else if (!strcmp(typeBuff, "CharacterController")) { type = CMP_CHARACTER_CONTROLLER; }
            else if (!strcmp(typeBuff, "Sprite")) { type = CMP_SPRITE; }
            else if (!strcmp(typeBuff, "Tilemap")) { type = CMP_SPRITE; tilemap = true;}
            else if (!strcmp(typeBuff, "Name")) 
            {
                if (sscanf(buff, " %s %s", typeBuff, nameBuff) == 2)
                {
                    bp->name = nameBuff;
                }
                else
                {
                    LogWarning("entity has no name, setting to Unknown");
                    bp->name = "Unknown";
                }
                continue;
            }
            //TODO assuming priority 0 for everything, if i actually want to implement priorities, this is where i go ahead
            //and add priority loading
            switch (type)
            {
                case CMP_MESH:
                { 
                    //read line again, but grab filename too
                    if (sscanf(buff, " %s %s %i", typeBuff, nameBuff, &intRead[0]) == 3)
                    {
                        bp->meshConstructor = new MeshConstructor;
                        bp->meshConstructor->mdl = LoadModel(nameBuff, intRead[0]); //0 or non zero for static collision mesh
                    }
                    else
                        LogError("Error reading component type");   
                    
                }
                break;
                case CMP_SPRITE:
                { 
                    if (tilemap)
                    {
                        if (sscanf(buff, " %s %s", typeBuff, nameBuff) == 2)
                        {
                            bp->spriteConstructor = new SpriteConstructor;
                            bp->spriteConstructor->tiled = true;
                           
                            bp->spriteConstructor->tilemapFile = string(nameBuff);
                        }
                        else
                            LogError("Error reading tilemap component");
                    }
                    else
                    {
                        //read line again, but grab filename too
                        if (sscanf(buff, " %s %i", typeBuff, &intRead[0]) == 2)
                        {
                            if (intRead[0] <= 0)
                            {
                                LogError("Sprite frames must be at least 1!");
                            }
                            else
                            {
                                bp->spriteConstructor = ReadSprite(file, intRead[0]);
                                if (!bp->spriteConstructor)
                                    LogError("could not load sprite");
                                
                            }
                        }
                        else
                            LogError("Error reading sprite component data");    
                    }
                }
                break;
                case CMP_MATERIAL:
                {
                bp->materialConstructor = ReadMaterial(file, _priority);
                if (bp->materialConstructor == 0)
                    LogError("Cannot read material");                  
                }
                break;
                case CMP_SKELETAL_MESH:
                {
                    //read line again, but grab filename too
                    if (sscanf(buff, " %s %s", typeBuff, nameBuff) == 2)
                    {
                    bp->skMeshConstructor = new SkinnedMeshConstructor();
                    bp->skMeshConstructor->animatedModel = LoadAnimatedMesh(nameBuff); //0 or non zero for static collision mesh
                    }
                    else
                        LogError("Error reading component type");
                }
                break;
                case CMP_LOGIC:
                {
                    if (sscanf(buff, " %s %s", typeBuff, nameBuff) == 2)
                    {
                        bp->logicScript = string(nameBuff);
                    }
                    else
                        LogError("Error reading component type");
                }
                break;
                case CMP_ANIMATION_CONTROLLER:
                {
                    bp->animControllerConstructor = ReadAnimController(file, _priority);
                }
                break;
                //NOTE rigid body still needs a shape, if shape doesnt exist, delete rigid body or asign a default shape?
                case CMP_RIGIDBODY:
                {
                    bp->rbConstructor = ReadRigidbody(file);
                }
                break;
                //NOTE: if mesh shape, we must have a mesh component attached.
                case CMP_COLLISION_SHAPE:
                {
                    bp->shapeConstructor = ReadCollisionShape(file);
                    //need to eat up the end bracket  this is getting ugly.
                   /* if (!fgets(buff, sizeof(buff), file))
                    {
                        LogError("Blueprint broken."); //ugly, but tons of outputs on failure cases at least.
                        return 0;
                    }*/
                    
                }
                break;
                case CMP_CHARACTER_CONTROLLER:
                {
                    bp->charConstructor = ReadCharController(file);
                }
                break;
                default:
                    LogWarning("Unkown component type in blueprint!? (maybe name -- fixthis)");
                    break;
            }
        }
    }
   
   fclose(file);
   //done reading the file..
   
   return bp;
    
}
