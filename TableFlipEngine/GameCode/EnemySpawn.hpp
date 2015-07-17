/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef _ENEMY_SPAWN_H
#define _ENEMY_SPAWN_H

#include "Components.h"
#include <Builder.h>

enum EnemyType { ENEMY_1A = 0, NUM_ENEMIES };

class EnemySpawn : public Logic
{
private:
    
    BlueprintResource *bp1A;
    unsigned int lastSpawn;
    unsigned int spawnDelay;
    
public:
   
    void Init()
    {
        bp1A = LoadBlueprint("Data/Blueprints/enemy1A.bp");
        activeUpdate = true;
        lastSpawn = GetTimecode();
        spawnDelay = 1000;
    }

    void SpawnEnemy(EnemyType type)
    {
        Vector3 pos = entity->GetTransform()->position;
        pos.y += 25;
        if (type == ENEMY_1A)
           Builder::GetInstance()->ConstructEntity(bp1A, pos);

        lastSpawn = GetTimecode();
            
    }
    
    void Update(float dt)
    {
        if (GetTimecode() > lastSpawn + spawnDelay)
            SpawnEnemy(ENEMY_1A);
    }
    
    void Destroy()
    {
    }
};


#endif
