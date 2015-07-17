/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "AnimationController.h"
#include "AnimationResource.h"
AnimationController::AnimationController(Entity *ent, Transform3d *trans, SkeletalMesh *_mesh)
{
    speed = 1.0f;
    entity = ent;
    transform = trans;
    playing = false;
   
    animTimer = 0.0f;
    mesh = _mesh;
    curAnim = 0;
    numTracks = 2;
    tracks = new AnimationTrack[numTracks];
    mainTrack = 0; //the main anim track (simple A*B blending right now)
    mixPos = 0.0f;
    lastTrack = 0;
    mixing = false;
    //copy skeleton, for bone corrections
    boneCorrections = new Md5Bone[mesh->numBones];
    memcpy(boneCorrections, mesh->bones, sizeof(Md5Bone) * mesh->numBones);
    for (int i = 0; i < mesh->numBones; i++)
    {
        boneCorrections[i].parent = 0; //this is set 1 if correction is active
        //forgot these should be set to 0..  these get added to the updated animation results
        boneCorrections[i].position = Vector3(0,0,0);
        boneCorrections[i].rotation = Quaternion(0,0,0,1);
    }

}

AnimationController::~AnimationController()
{
    
    map<string, AnimationResource*, StringCompareForMap>::iterator iter = animations.begin();
    while(iter != animations.end())
    {
        iter->second->RemoveRef();
        iter++;
    }
    delete[] boneCorrections;
    delete[] tracks;
}

void AnimationController::AddAnimation(AnimationResource *anim)
{
    //TODO quick dirty numBOnes check for validity.
    //i SHOULD be going through the whole hieracrhy and checking bone names.
    if (mesh->numBones != anim->numBones)
    {
        LOGOUTPUT << "AddAnimation() : " << anim->name << " - mismatching bone structure";
        LogError();
        return;
    }
    
    map<string, AnimationResource *, StringCompareForMap>::iterator iter = animations.find(anim->name);
    
    //found it
    if (iter != animations.end())
    {
        LOGOUTPUT << "AddAnimation() : " << anim->name << " - an animation with that name already exists";
        LogError();
    }
    else
        animations[anim->name] = anim;
    
}

void AnimationController::RemoveAnimation(string name)
{
    map<string, AnimationResource *, StringCompareForMap>::iterator iter = animations.find(name);
    
    //found it
    if (iter != animations.end())
    {
        iter->second->RemoveRef();
        animations.erase(iter);
    }
    else
    {
        LOGOUTPUT << "AnimationController.RemoveAnimation() : could not find animation: " << name;
        LogWarning();
    }
}

void AnimationController::SetAnimation(string name, float mixTime)
{
    if (curAnim)
        if (name == curAnim->name)
            return;
    map<string, AnimationResource *, StringCompareForMap>::iterator iter = animations.find(name);
    
    
    //found it
    if (iter != animations.end())
    {
        //TODO think about what to cap animation mixing to, if at all?
        //set to 2 seconds right now
        if (mixTime < 2.0 && mixTime > 0.0)
        {
            mixLength = mixTime;
            mixPos = 0.0f;
            mixing = true;
        }
        else
            mixing = false;
        
        curAnim = iter->second;
       // curFrame = 0; nextFrame = 1;
       // animTimer = 0.0;
        
        lastTrack = mainTrack;
        mainTrack++;
        if (mainTrack > numTracks-1)
            mainTrack = 0;
        
        tracks[mainTrack].anim = curAnim;
        tracks[mainTrack].animTimer = 0.0;
        tracks[mainTrack].curFrame = 0;
        tracks[mainTrack].forward = true;
        tracks[mainTrack].nextFrame = 1; //this should be last track if backwards?
    }
    else
    {   
        LOGOUTPUT << "AnimationController.SetAnimation() : could not find animation: " << name;
        LogError();
    }
}

void AnimationController::SetBoneCorrection(unsigned int index, const Vector3 *position, const Quaternion *rotation)
{
    if (index > mesh->numBones -1) return;
   
    if (rotation)
    {
        //NOTE, i can walk the skeleton by checking the next indexes parent,
        //if it equals last index, it is a child, otherwise its a leaf node.
        
       
      // 
      
       
        if (index + 1 < mesh->numBones)
        {
            if (mesh->bones[index +1].parent == index)
            {
                LogWarning("SetBoneCorrection cannot be applied to bones with children");
                return;
            }
        }
           
             boneCorrections[index].rotation = *rotation;
             boneCorrections[index].parent = true; //active
   
              //  boneCorrections[walk].rotation = boneCorrections[walk-1].rotation;
           // if(first)
           //     boneCorrections[walk].position.z -= 0.2;
              
                //boneCorrections[walk].rotation = boneCorrections[walk-1].rotation;
                                                                  
                
                
                //this worked, but if bone local axis are not aligned. its useless
               /*   int walk = index+1; //look down hierarchy for children
                *   if (index + 1 >= mesh->numBones || mesh->bones->parent == 0)
            return; //no children or no parent, run away.
            
      // Vector3 rotatedPos = boneCorrections[index-1].rotation.RotatePoint(mesh->bones[index].position);
       Vector3 rotatedPos = boneCorrections[index-1].rotation.RotatePoint(mesh->bones[index-1].position);
       Vector3 firstCorrection = rotatedPos - mesh->bones[index].position;
        
        Vector3 firstPos = rotatedPos;
        bool first = true;
        while(walk < mesh->numBones)
        {
            if (mesh->bones[walk].parent == walk - 1)
            {Quaternion tr = boneCorrections[walk-1].rotation;
                    tr.z = tr.x;
                    tr.x = 0;
               // Quaternion fullRotation = boneCorrections[walk-1].rotation;
                  rotatedPos = tr.RotatePoint(mesh->bones[walk].position);
                if(first)
                    boneCorrections[walk].position = rotatedPos - mesh->bones[walk].position + firstCorrection;
                else
                {
                    
                    rotatedPos = tr.RotatePoint(mesh->bones[walk].position);
                    boneCorrections[walk].position = rotatedPos - mesh->bones[walk].position + boneCorrections[walk-1].position;
                }
                boneCorrections[walk].parent = true;
              walk++;
                first = false;
            }
            else
            {
                break;
            }
        }*/
            
    }
    /*
     *  rotatedPos = parent->rotation.RotatePoint(animPos);
            thisJoint->position.x = rotatedPos.x + parent->position.x;
            thisJoint->position.y = rotatedPos.y + parent->position.y;
            thisJoint->position.z = rotatedPos.z + parent->position.z;
            
            thisJoint->rotation = parent->rotation * animOrient;
            */
    //TODO not really going to support position i dont think, thats like an IK thing?
   // if (position)
    //    boneCorrections[index].position = *position;
    
}

//reset any childrn that may have been flagged
void AnimationController::ResetBoneCorrection(unsigned int index)
{
    if (index > mesh->numBones -1) return;
    boneCorrections[index].position = Vector3(0,0,0);
    boneCorrections[index].rotation = Quaternion(0,0,0,0);
    boneCorrections[index].parent = 0;
}

static float animInterp = 0.0f;
void AnimationController::Update(float dt)
{
    if (!curAnim || !playing)
        return;
    dt *= speed;
    animTimer += dt;
    
    if (animTimer >= curAnim->frameTime)
        animTimer = 0.0f;
   /* if (animTimer >= curAnim->frameTime)
    {
        animTimer = 0.0f;
        curFrame = nextFrame++;
        //wrap to 0
        if (nextFrame >= curAnim->numFrames - 1)
            nextFrame = 0; 
    }*/
   
    if (mixing)
    {
        mixPos += 0.001f * dt; //convert to seconds
        if (mixPos > mixLength)
            mixing = false;
        //keep updating previous track untill the mix is finished
        tracks[lastTrack].Update(dt);
    }
    
    tracks[mainTrack].Update(dt);
    
    animInterp = animTimer / curAnim->frameTime;
    
    //do SLERP here!
    InterpolateKeyframes(animInterp, mesh->bones);
    
}

void AnimationController::InterpolateKeyframes(float interp, Md5Bone *outSkeleton)
{
    Md5Bone *frameA = curAnim->keyframes[tracks[mainTrack].curFrame];
    Md5Bone *frameB = curAnim->keyframes[tracks[mainTrack].nextFrame];
    
    
    if (mixing) //mixing last track and previous track
    {
        Md5Bone *prevFrameA = tracks[lastTrack].anim->keyframes[tracks[lastTrack].curFrame];
        Md5Bone *prevFrameB = tracks[lastTrack].anim->keyframes[tracks[lastTrack].nextFrame];
        //interp each joint
        for (int i = 0; i < mesh->numBones; i++)
        {
            //copy parent (is this really needed??)
            outSkeleton[i].parent = frameA[i].parent;
        // frameOut[i].nParent = frameA[i].nParent;

            //linear interpolation for position
            //Vector3 aPos = frameA[i].position;
            //Vector3 bPos = frameB[i].position;
           // Vector3 paPos = prevFrameA[i].position;
            //Vector3 pbPos = prevFrameB[i].position;
            //Vector3 newPos;
            //NOTE should add float * vector3 operator
           
            
            Quaternion lastRot, curRot;
            //Vector3 curTrackPos = frameA[i].position + ( (frameB[i].position - frameA[i].position) * interp);
            //Vector3 lastTrackPos = prevFrameA[i].position + ( (prevFrameB[i].position - prevFrameA[i].position) * interp);
           // outSkeleton[i].position = lastTrackPos + (( curTrackPos - lastTrackPos) * (mixPos / mixLength));
           
            Vector3 curTrackPos = frameA[i].position + ( (frameB[i].position - frameA[i].position) * interp);
            Vector3 lastTrackPos = prevFrameA[i].position + ( (prevFrameB[i].position - prevFrameA[i].position) * interp);
            outSkeleton[i].position = lastTrackPos + (( curTrackPos - lastTrackPos) * (mixPos / mixLength));
          
            
            
            // outSkeleton[i].position.x = frameA[i].position.x + (interp * (frameB[i].position.x - frameA[i].position.x));
           // outSkeleton[i].position.y = frameA[i].position.y + (interp * (frameB[i].position.y - frameA[i].position.y));
            //outSkeleton[i].position.z = frameA[i].position.z + (interp * (frameB[i].position.z - frameA[i].position.z));
            
            //newPos = outSkeleton[i].position;
            //SLOW ASS test slerp for orientation.
            if (boneCorrections[i].parent) //using parent variable as a boolean flag, if correction is active or not
            {
                lastRot =  prevFrameA[i].rotation.Slerp(prevFrameB[i].rotation, interp);
                curRot = frameA[i].rotation.Slerp(frameB[i].rotation, interp);
            // outSkeleton[i].position += boneCorrections[i].position;
               // outSkeleton[i].rotation = lastRot.Slerp(curRot, mixPos / mixLength) * boneCorrections[i].rotation;;
               outSkeleton[i].rotation = frameA[i].rotation.Lerp(frameB[i].rotation, interp) * boneCorrections[i].rotation; //then rotate by correction amount
            }
            else
                outSkeleton[i].rotation = frameA[i].rotation.Slerp(frameB[i].rotation, interp);
            
            //CHEBYSHEV WILL RISE FROM THE DEAD AND MAKE THIS FUNCTION WORK!!!!
            //ChebyshevSlerp(frameA[i].qOrientation, frameB[i].qOrientation, i, &frameOut[i].qOrientation, interp);
        }
    }
    else   //no mixing happening
    {
        //interp each joint
        for (int i = 0; i < mesh->numBones; i++)
        {
            //copy parent (is this really needed??)
            outSkeleton[i].parent = frameA[i].parent;
        // frameOut[i].nParent = frameA[i].nParent;

            //linear interpolation for position
            Vector3 aPos = frameA[i].position;
            Vector3 bPos = frameB[i].position;
            //Vector3 newPos;
            outSkeleton[i].position.x = frameA[i].position.x + (interp * (frameB[i].position.x - frameA[i].position.x));
            outSkeleton[i].position.y = frameA[i].position.y + (interp * (frameB[i].position.y - frameA[i].position.y));
            outSkeleton[i].position.z = frameA[i].position.z + (interp * (frameB[i].position.z - frameA[i].position.z));
            
            //newPos = outSkeleton[i].position;
            //SLOW ASS test slerp for orientation.
            if (boneCorrections[i].parent) //using parent variable as a boolean flag, if correction is active or not
            {
            // outSkeleton[i].position += boneCorrections[i].position;
                outSkeleton[i].rotation = frameA[i].rotation.Slerp(frameB[i].rotation, interp) * boneCorrections[i].rotation; //then rotate by correction amount
            }
            else
                outSkeleton[i].rotation = frameA[i].rotation.Slerp(frameB[i].rotation, interp);
            
            //CHEBYSHEV WILL RISE FROM THE DEAD AND MAKE THIS FUNCTION WORK!!!!
            //ChebyshevSlerp(frameA[i].qOrientation, frameB[i].qOrientation, i, &frameOut[i].qOrientation, interp); 
        }
    }

}


void AnimationTrack::Update(float dt)
{
    animTimer += dt;
    if (animTimer >= anim->frameTime)
    {
        animTimer = 0.0f;
        if (forward)
        {
            curFrame = nextFrame++;
            //wrap to first frame
            if (nextFrame >= anim->numFrames - 1)
                nextFrame = 0; 
        }
        else
        {
            curFrame = nextFrame--;
            //wrap to last frame
            if (nextFrame < 0)
                nextFrame = anim->numFrames -1; 
        }
    }
}
