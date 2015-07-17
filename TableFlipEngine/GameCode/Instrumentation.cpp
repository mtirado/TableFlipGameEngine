/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "Lander.h"

void Instrumentation::Init(Lander *_lander)
{
    lander = _lander;
    //attitude gyro instrumentation
    attitudeYOffset = 14.0;
    bpAttMask = LoadBlueprint("Data/Blueprints/attitude-mask.bp");
    bpAttGyro = LoadBlueprint("Data/Blueprints/attitude-gyro.bp");
    bpAttCrosshair = LoadBlueprint("Data/Blueprints/attitude-crosshair.bp");

    mask = gBuilder->ConstructEntity(bpAttMask, Vector3(0.0,attitudeYOffset,0.0));
    mask->GetSprite()->getMaterial()->srcBlend = GL_SRC_ALPHA;
    mask->GetSprite()->getMaterial()->dstBlend = GL_ONE_MINUS_SRC_ALPHA;

    attitude = gBuilder->ConstructEntity(bpAttGyro, Vector3(0.0,attitudeYOffset,0.0));

    crosshair = gBuilder->ConstructEntity(bpAttCrosshair, Vector3(0.0,attitudeYOffset,0.0));
    crosshair->GetSprite()->getMaterial()->srcBlend = GL_SRC_ALPHA;
    crosshair->GetSprite()->getMaterial()->dstBlend = GL_ONE_MINUS_SRC_ALPHA;
}

void Instrumentation::Update()
{
    Transform3d *tr = lander->entity->GetTransform();
    Vector3 gUp(0.0, 0.99998, 0.0);
    Vector3 gRight = gUp.Cross(tr->GetBack());
    gRight.Normalize();
    Vector3 gFwd = tr->GetRight().Cross(gUp);
    gFwd.Normalize();
    Vector3 back = tr->GetBack(); //FIXME back is not normalized, acos retuns nan... UGH WTF!?

    float pitchRad = tr->GetBack().AngleBetween(gFwd);
    float trRoll  = tr->GetRight().AngleBetween(gRight);

    float pitchVal = pitchRad;
    if (fabs(pitchRad) <= 0.001)
        pitchRad = 0.001;
    if (fabs(trRoll) <= 0.001)
        trRoll = 0.001;
    float pitchDir = 1.0;
    float rollDir = 1.0;
    bool down = false;
    if (tr->GetRight().y < 0.0)
        trRoll *= -1.0;
    if (tr->GetBack().y < 0.0)
        pitchRad *= -1.0;

    float yTranslate = 12.0; //half of dimensions on screen
    //dis feels so hacky :[
    if ( pitchRad > 1.57)
        pitchRad *= -1.0;
    if (pitchRad < -1.57 && tr->GetBack().y < 0.0)
        pitchRad = -1.0 * (M_PI + pitchRad);
    else if (pitchRad < -1.57 && tr->GetBack().y >= 0.0)
        pitchRad = M_PI + pitchRad;//always neg
    else if (pitchRad > 1.570)
        pitchRad = 1.570;


    float transPitch = pitchRad != 0.0 ? (pitchRad / M_PI) * yTranslate : 0.001;
    attitude->GetTransform()->rotation.SetAxisAngle(Vector3(0.0,0.0,1.0),
                                                    trRoll);

    attitude->GetTransform()->position = attitude->GetTransform()->GetUp() * transPitch;
    attitude->GetTransform()->position.y += attitudeYOffset;
    attitude->GetTransform()->Update();
}

void Instrumentation::Destroy()
{
    if (!lander)
        return;

    lander = 0;
    attitude->DestroyAndRelease();
    mask->DestroyAndRelease();
    crosshair->DestroyAndRelease();
}
