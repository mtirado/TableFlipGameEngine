/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_

#include "Utilities.h"
#include "Resources.h"
#include "AnimationResource.h"
#include "SkeletalMesh.h"


//a track, mixed and updated by the controller to create final skeleton
class AnimationTrack
{
public:
    AnimationResource *anim;
    int curFrame;
    int nextFrame;
    
    float animTimer;
    bool forward;
    void Update(float dt);
    AnimationTrack() { curFrame = 0; nextFrame = 1;forward = true; animTimer = 0.0; }
};

class AnimationController  : public Engine::Component
{
private:
    SkeletalMesh *mesh;
    
    float animTimer;
    AnimationResource *curAnim;
    bool playing;
    bool mixing;  //are we currently mixing anims?
    float mixPos; //current mix value
    float mixLength; //when will the mix be finished 0-1 ratio of previous animations length
    float speed;
    //aditional translation or rotations added NOTE if sharing animations, these 
    //will need to be 'reskinned', but as of now, there are no shared anims
    Md5Bone *boneCorrections; //for aligning feet to terrain and cool stuff like that  **fake IK hax**?
   // Md5Bone *skeleton; //pointer to the real bones
   
    //our animations, TODO: verify skeletal structure before anim is added!!
    // TODO! verify the fucking skeleton already, now theres systems relying on this being written....
    map<string, AnimationResource *, StringCompareForMap> animations;
    int mainTrack; //current animation track (just simple A*B mixing right now)
    int lastTrack; //last animation track
    int numTracks;
    AnimationTrack *tracks;
    void InterpolateKeyframes(float interp, Md5Bone *outSkeleton);
    
    AnimationController();
public:
    //we need a mesh !
   AnimationController(Entity *ent, Transform3d *trans, SkeletalMesh *_mesh);
   ~AnimationController();
   
   
   
   void AddAnimation(AnimationResource *);
   void RemoveAnimation(string name);
   //mix = how long till new animation is mixed in, 0-1  ratio of last tracks animation length
   void SetAnimation(string name, float mixTime);
   void SetBoneCorrection(unsigned int index, const Vector3 *position, const Quaternion *rotation);
   void ResetBoneCorrection(unsigned int index);
   
   inline void setSpeed(float sp) { speed = sp; }
   inline bool isPlaying() { return playing; }
   inline void Play()      { playing = true; }
   inline void Pause()     { playing = false; }
   //TODO fix these functions to work with tracks? they may be ok actually
   inline void Stop()      { playing = false; animTimer = 0.0f; }
   inline void Rewind()    { animTimer = 0.0f; }
   
   void Update(float dt);
   
   //const AnimationResource &getCurrentAnim();
   const AnimationTrack *getCurrentTrack() { return &tracks[mainTrack]; }
   
};

#endif
