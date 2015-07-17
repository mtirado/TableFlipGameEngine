/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "CharacterController.h"
#include "Entity.h"
#include "AnimationResource.h"



class btKinematicClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
        btKinematicClosestNotMeRayResultCallback (btCollisionObject* me) : btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        {
                m_me = me;
        }

        virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
        {
                if (rayResult.m_collisionObject == m_me)
                        return 1.0;

                return ClosestRayResultCallback::addSingleResult (rayResult, normalInWorldSpace);
        }
protected:
        btCollisionObject* m_me;
};

class btKinematicClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
        btKinematicClosestNotMeConvexResultCallback (btCollisionObject* me, const btVector3& up, btScalar minSlopeDot)
        : btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
        , m_me(me)
        , m_up(up)
        , m_minSlopeDot(minSlopeDot)
        {
        }

        virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
        {
            //for trigger filtering
            if (!convexResult.m_hitCollisionObject->hasContactResponse())
            return btScalar(1.0);
                if (convexResult.m_hitCollisionObject == m_me)
                        return btScalar(1.0);

                btVector3 hitNormalWorld;
                if (normalInWorldSpace)
                {
                        hitNormalWorld = convexResult.m_hitNormalLocal;
                } else
                {
                        ///need to transform normal into worldspace
                        hitNormalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal;
                }

                btScalar dotUp = m_up.dot(hitNormalWorld);
                if (dotUp < m_minSlopeDot) {
                        return btScalar(1.0);
                }

                return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
        }
protected:
        btCollisionObject* m_me;
        const btVector3 m_up;
        btScalar m_minSlopeDot;
};





CharacterController::CharacterController(btPairCachingGhostObject* _ghostObject,btConvexShape* convexShape,
					 btScalar stepHeight, int upAxis) : btKinematicCharacterController(_ghostObject,convexShape,stepHeight)
{

  m_useGhostObjectSweepTest = true;
  ghostObject = _ghostObject;
  capsule = convexShape;
  walkSpeed = 0.20f;
  runSpeed  = 0.215f;
  turnWalkSpeed = 9.0f;
  turnRunSpeed = 3.0f;
  forwardSpeed = 0.0f;
  strafeSpeed = 0.0f;
  direction = 0.0f;
  dampener = 0.1f;  //slow the acceleration a bit
  velocity=btVector3(0,0,0);
  forward = turn = strafe = 0.0f;
  yOffset = s_yOffset = 0.0;
  zOffset = s_zOffset = 0.0;
  rLegAngle = lLegAngle = 0.0f;
  
 
  
}

CharacterController::~CharacterController()
{
    Physics::GetInstance()->RemoveCharacter(this);
    delete capsule;
    delete ghostObject;
}

//velocity += acceleration - friction*velocity    or velocity squared
void CharacterController::playerStep(btCollisionWorld* collisionWorld, btScalar dt)
{
    
    
    //turn!
    btMatrix3x3 orn = ghostObject->getWorldTransform().getBasis();
    btQuaternion q = btQuaternion(btVector3(0,1,0),turn * 0.03);
    orn *= btMatrix3x3(q);
    ghostObject->getWorldTransform ().setBasis(orn);
    
    btTransform t = ghostObject->getWorldTransform();
    btVector3 fwd = t.getBasis()[2];
    btVector3 lft = t.getBasis()[0];
    lft.normalize();
    fwd.normalize();
    //move forward or back
    forwardSpeed = (forward * dampener);
    if (forwardSpeed > runSpeed)
        forwardSpeed = runSpeed;
    else if (forwardSpeed < -runSpeed)
        forwardSpeed = runSpeed;
        
    strafeSpeed = (strafe * dampener);
    if (strafeSpeed > walkSpeed)
        strafeSpeed = walkSpeed;
    else if (strafeSpeed < -walkSpeed)
        strafeSpeed = -walkSpeed;
        
        
    velocity += ((fwd * runSpeed) * (-forward)) - velocity * dampener;
    velocity += (lft * (strafe)) - velocity * dampener;
    
   setVelocityForTimeInterval(velocity, dt);
  
   if (entity->GetAnimationController())
   {
       if (forward)
           entity->GetAnimationController()->SetAnimation("walk", 0.133);
       else
           entity->GetAnimationController()->SetAnimation("idle", 0.133);
   }
  
   //btKinematicCharacterController::playerStep(collisionWorld, dt);
   //default controller step code
   
  
//      printf("playerStep(): ");
//      printf("  dt = %f", dt);

        // quick check...
        if (!m_useWalkDirection && m_velocityTimeInterval <= 0.0) {
//              printf("\n");
                return;         // no motion
        }

        m_wasOnGround = onGround();

        // Update fall velocity.
        m_verticalVelocity -= m_gravity * dt;
        if(m_verticalVelocity > 0.0 && m_verticalVelocity > m_jumpSpeed)
        {
                m_verticalVelocity = m_jumpSpeed;
        }
        if(m_verticalVelocity < 0.0 && btFabs(m_verticalVelocity) > btFabs(m_fallSpeed))
        {
                m_verticalVelocity = -btFabs(m_fallSpeed);
        }
        m_verticalOffset = m_verticalVelocity * dt;


        btTransform xform;
        xform = m_ghostObject->getWorldTransform ();

//      printf("walkDirection(%f,%f,%f)\n",walkDirection[0],walkDirection[1],walkDirection[2]);
//      printf("walkSpeed=%f\n",walkSpeed);

        stepUp (collisionWorld);
        if (m_useWalkDirection) {
                stepForwardAndStrafe (collisionWorld, m_walkDirection);
        } else {
                //printf("  time: %f", m_velocityTimeInterval);
                // still have some time left for moving!
                btScalar dtMoving =
                        (dt < m_velocityTimeInterval) ? dt : m_velocityTimeInterval;
                m_velocityTimeInterval -= dt;

                // how far will we move while we are moving?
                btVector3 move = m_walkDirection * dtMoving;

                //printf("  dtMoving: %f", dtMoving);

                // okay, step
                stepForwardAndStrafe(collisionWorld, move);
        }
        stepDown (collisionWorld, dt);

        // printf("\n");

        xform.setOrigin (m_currentPosition);
        m_ghostObject->setWorldTransform (xform);

   
   
   
   
   
   
   
   
   forward = turn = strafe = 0.0f;
  
}

void CharacterController::debugDraw(btIDebugDraw* debugDrawer)
{
    btVector3 start = ghostObject->getWorldTransform().getOrigin();
    debugDrawer->drawLine(start, start+ghostObject->getWorldTransform().getBasis()[0] * 3, btVector3(0.99f,0,0));
    debugDrawer->drawLine(start, start+ghostObject->getWorldTransform().getBasis()[1] * 3, btVector3(0,0.99f,0));
    debugDrawer->drawLine(start, start+ghostObject->getWorldTransform().getBasis()[2] * 3, btVector3(0,0,0.99f));
    btKinematicCharacterController::debugDraw(debugDrawer);
}


bool CharacterController::recoverFromPenetration ( btCollisionWorld* collisionWorld)
{
        // Here we must refresh the overlapping paircache as the penetrating movement itself or the
        // previous recovery iteration might have used setWorldTransform and pushed us into an object
        // that is not in the previous cache contents from the last timestep, as will happen if we
        // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
        //
        // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
        // paircache and the ghostobject's internal paircache at the same time.    /BW

        btVector3 minAabb, maxAabb;
        m_convexShape->getAabb(m_ghostObject->getWorldTransform(), minAabb,maxAabb);
        collisionWorld->getBroadphase()->setAabb(m_ghostObject->getBroadphaseHandle(), 
                                                 minAabb, 
                                                 maxAabb, 
                                                 collisionWorld->getDispatcher());
                                                 
        bool penetration = false;

        //check btCollisionDispatcher::getNewManifold, redundant call??
        collisionWorld->getDispatcher()->dispatchAllCollisionPairs(m_ghostObject->getOverlappingPairCache(), collisionWorld->getDispatchInfo(), collisionWorld->getDispatcher());

        m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
        
        btScalar maxPen = btScalar(0.0);
        for (int i = 0; i < m_ghostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
        {
                m_manifoldArray.resize(0);

                btBroadphasePair* collisionPair = &m_ghostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];
                
                //NOTE: NO CONTACT RESPONSE TRIGGER FIX
                //for trigger filtering
                if (!static_cast<btCollisionObject*>(collisionPair->m_pProxy0->m_clientObject)->hasContactResponse() ||
                    !static_cast<btCollisionObject*>(collisionPair->m_pProxy1->m_clientObject)->hasContactResponse())
                continue;
                
                if (collisionPair->m_algorithm)
                        collisionPair->m_algorithm->getAllContactManifolds(m_manifoldArray);

                
                for (int j=0;j<m_manifoldArray.size();j++)
                {
                        btPersistentManifold* manifold = m_manifoldArray[j];
                        btScalar directionSign = manifold->getBody0() == m_ghostObject ? btScalar(-1.0) : btScalar(1.0);
                        for (int p=0;p<manifold->getNumContacts();p++)
                        {
                                const btManifoldPoint&pt = manifold->getContactPoint(p);

                                btScalar dist = pt.getDistance();

                                if (dist < 0.0)
                                //if (fabs(dist) > 0.2 /* m_addedMargin? */)
                                
                                {
                                        if (dist < maxPen)
                                        {
                                                maxPen = dist;
                                                m_touchingNormal = pt.m_normalWorldOnB * directionSign;//??

                                        }
                                        m_currentPosition += pt.m_normalWorldOnB * directionSign * dist * btScalar(0.2);
                                        penetration = true;
                                      //  m_manifoldArray.remove(manifold);
                                } else {
                                        //printf("touching %f\n", dist);
                                }
                        }
                        //TODO we _should_ remove so the manifold list being iterated to dispatch collision events
                        //is shorter, but maybe that whole system should be reworked? for now, just clearing manifold out
                        //manifold->clearManifold();
                        //m_manifoldArray.remove(manifold);
                }
        }
        btTransform newTrans = m_ghostObject->getWorldTransform();
        newTrans.setOrigin(m_currentPosition);
        m_ghostObject->setWorldTransform(newTrans);
//      printf("m_touchingNormal = %f,%f,%f\n",m_touchingNormal[0],m_touchingNormal[1],m_touchingNormal[2]);
        return penetration;
}


void CharacterController::preStep (btCollisionWorld* collisionWorld)
{
        
        int numPenetrationLoops = 0;
        m_touchingContact = false;
        while (recoverFromPenetration (collisionWorld))
        {
                numPenetrationLoops++;
                m_touchingContact = true;
                if (numPenetrationLoops > 4) //4
                {
                        //printf("character could not recover from penetration = %d\n", numPenetrationLoops);
                        break;
                }
        }

     // LogError("PRESTEP");
        m_currentPosition = m_ghostObject->getWorldTransform().getOrigin();
        m_targetPosition = m_currentPosition;
//      printf("m_targetPosition=%f,%f,%f\n",m_targetPosition[0],m_targetPosition[1],m_targetPosition[2]);

        
}


void CharacterController::stepDown ( btCollisionWorld* collisionWorld, btScalar dt)
{
        btTransform start, end;
        yOffset = zOffset = 0; //reset these every frame!
        // phase 3: down
        /*btScalar additionalDownStep = (m_wasOnGround && !onGround()) ? m_stepHeight : 0.0;
        btVector3 step_drop = getUpAxisDirections()[m_upAxis] * (m_currentStepOffset + additionalDownStep);
        btScalar downVelocity = (additionalDownStep == 0.0 && m_verticalVelocity<0.0?-m_verticalVelocity:0.0) * dt;
        btVector3 gravity_drop = getUpAxisDirections()[m_upAxis] * downVelocity; 
        m_targetPosition -= (step_drop + gravity_drop);*/

        btScalar downVelocity = (m_verticalVelocity<0.f?-m_verticalVelocity:0.f) * dt;
        if(downVelocity > 0.0 && downVelocity < m_stepHeight
                && (m_wasOnGround || !m_wasJumping))
        {
                downVelocity = m_stepHeight;
        }

        btVector3 step_drop = getUpAxisDirections()[m_upAxis] * (m_currentStepOffset + downVelocity);
        m_targetPosition -= step_drop;

        start.setIdentity ();
        end.setIdentity ();

        start.setOrigin (m_currentPosition);
        end.setOrigin (m_targetPosition);

        btKinematicClosestNotMeConvexResultCallback callback (m_ghostObject, getUpAxisDirections()[m_upAxis], m_maxSlopeCosine);
        
        callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
        callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;
        
        if (m_useGhostObjectSweepTest)
        {
                m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, 0.000);
        } else
        {
                collisionWorld->convexSweepTest (m_convexShape, start, end, callback, 0.0000);
        }

        if (callback.hasHit())
        {
                // we dropped a fraction of the height -> hit floor
                m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
                m_verticalVelocity = 0.0;
                m_verticalOffset = 0.0;
                m_wasJumping = false;
                
                float slopeAngle = acosf(callback.m_hitNormalWorld.dot(m_ghostObject->getWorldTransform().getBasis()[2])) - DegToRad(90);
                lLegAngle = rLegAngle = -RadToDeg(slopeAngle);// * 0.34;
                zOffset = slopeAngle * 0.1; //magic number, for z offsetting based on slope
               // static int downMod = 1.7;
               /* if (slopeAngle >0)
                {
                    lLegAngle *=downMod; //RadToDeg(slopeAngle);
                    rLegAngle *=downMod; //RadToDeg(slopeAngle);
                    zOffset *= downMod * 1.33;
                }*/
                //yOffset = -slopeAngle *0.05;
                
        } else {
                // we dropped the full height
                //falling
                m_currentPosition = m_targetPosition;
        }
        
        //update program controlled offsets (hacky solution for feet through world syndrome)
        zOffset += s_zOffset;
        yOffset += s_yOffset;
        
        //adjust animations feet to the normal of the triangle we're colliding with.
       /* SkeletalMesh *sk = entity->GetSkeletalMesh();
        if (sk)
        {
            int lHip = sk->GetBone("\"foot.L\"");
            int rHip = sk->GetBone("\"foot.R\""); //NOTE these names are not accurate, this is the thigh
            if (lHip > 0 && rHip > 0)
            {
                AnimationController *ac = entity->GetAnimationController();
                if (ac)
                {
                    Quaternion lh = Quaternion(0,0,lLegAngle,1);
                    Quaternion rh = Quaternion(0,0,rLegAngle,1);
                    rh.SetEurlerAngles(0, 0, rLegAngle);
                    lh.SetEurlerAngles(0, 0, lLegAngle);
                    lh.Normalize();
                    rh.Normalize();
                    //Vector3 tVec = Vector3(0.0f, 0.0f, 0.0f);
                    ac->SetBoneCorrection(lHip, 0, &lh);
                    ac->SetBoneCorrection(rHip, 0, &rh);
                } 
            }
        }*/
}

void CharacterController::stepUp(btCollisionWorld* world)
{
        // phase 1: up
        btTransform start, end;
        m_targetPosition = m_currentPosition + getUpAxisDirections()[m_upAxis] * (m_stepHeight + (m_verticalOffset > 0.f?m_verticalOffset:0.f));

        start.setIdentity ();
        end.setIdentity ();

        /* FIXME: Handle penetration properly */
        start.setOrigin (m_currentPosition + getUpAxisDirections()[m_upAxis] * (m_convexShape->getMargin() + m_addedMargin));
        end.setOrigin (m_targetPosition);

        btKinematicClosestNotMeConvexResultCallback callback (m_ghostObject, -getUpAxisDirections()[m_upAxis], btScalar(0.7071));
        callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
        callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;
        
        if (m_useGhostObjectSweepTest)
        {
                m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, 0.0);
       }
        else
        {
                world->convexSweepTest (m_convexShape, start, end, callback);
        }
        
        if (callback.hasHit())
        {
            float slopeAngle = callback.m_hitNormalWorld.dot(getUpAxisDirections()[m_upAxis]);
                // Only modify the position if the hit was a slope and not a wall or ceiling.
                if(slopeAngle > 0.0)
                {                    
                        // we moved up only a fraction of the step height
                        m_currentStepOffset = m_stepHeight * callback.m_closestHitFraction;
                        m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);
                }
                m_verticalVelocity = 0.0;
                m_verticalOffset = 0.0;
        } else {
                m_currentStepOffset = m_stepHeight;
                m_currentPosition = m_targetPosition;
        }
}

void CharacterController::stepForwardAndStrafe(btCollisionWorld* collisionWorld, const btVector3& walkMove)
{
        // printf("m_normalizedDirection=%f,%f,%f\n",
        //      m_normalizedDirection[0],m_normalizedDirection[1],m_normalizedDirection[2]);
        // phase 2: forward and strafe
        btTransform start, end;
        m_targetPosition = m_currentPosition + walkMove;

        start.setIdentity ();
        end.setIdentity ();
        
        btScalar fraction = 1.0;
        btScalar distance2 = (m_currentPosition-m_targetPosition).length2();
//      printf("distance2=%f\n",distance2);

        if (m_touchingContact)
        {
                if (m_normalizedDirection.dot(m_touchingNormal) > btScalar(0.0))
                {
                        updateTargetPositionBasedOnCollision (m_touchingNormal);
                }
        }

        int maxIter = 4; // was 10

        while (fraction > btScalar(0.01) && maxIter-- > 0)
        {
                start.setOrigin (m_currentPosition);
                end.setOrigin (m_targetPosition);
                btVector3 sweepDirNegative(m_currentPosition - m_targetPosition);

                btKinematicClosestNotMeConvexResultCallback callback (m_ghostObject, sweepDirNegative, btScalar(0.00001));
                callback.m_collisionFilterGroup = getGhostObject()->getBroadphaseHandle()->m_collisionFilterGroup;
                callback.m_collisionFilterMask = getGhostObject()->getBroadphaseHandle()->m_collisionFilterMask;


                btScalar margin = m_convexShape->getMargin();
                m_convexShape->setMargin(margin + m_addedMargin);

// 
                if (m_useGhostObjectSweepTest)
                {
                        m_ghostObject->convexSweepTest (m_convexShape, start, end, callback, 0.000);
                } else
                {
                        collisionWorld->convexSweepTest (m_convexShape, start, end, callback, collisionWorld->getDispatchInfo().m_allowedCcdPenetration);
                }
                
                m_convexShape->setMargin(margin);

                
                fraction -= callback.m_closestHitFraction;

                if (callback.hasHit())
                {       
                        // we moved only a fraction
                        btScalar hitDistance;
                        hitDistance = (callback.m_hitPointWorld - m_currentPosition).length();

//                      m_currentPosition.setInterpolate3 (m_currentPosition, m_targetPosition, callback.m_closestHitFraction);

                        updateTargetPositionBasedOnCollision (callback.m_hitNormalWorld);
                        btVector3 currentDir = m_targetPosition - m_currentPosition;
                        distance2 = currentDir.length2();
                        if (distance2 > SIMD_EPSILON)
                        {
                                currentDir.normalize();
                                /* See Quake2: "If velocity is against original velocity, stop ead to avoid tiny oscilations in sloping corners." */
                                if (currentDir.dot(m_normalizedDirection) <= btScalar(0.0))
                                {
                                        break;
                                }
                        } else
                        {
//                              printf("currentDir: don't normalize a zero vector\n");
                                break;
                        }

                } else {
                        // we moved whole way
                        m_currentPosition = m_targetPosition;
                }

        //      if (callback.m_closestHitFraction == 0.f)
        //              break;

        }
}
