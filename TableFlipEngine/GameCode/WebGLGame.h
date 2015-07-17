/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef WEBGLGAME_H__
#define WEBGLGAME_H__

#include <UserInterface.h>

void InitUI(Entity *base);
void UpdateUI();
void ShutdownUI();
void GameInit();
void GameUpdate();
void GameShutdown();

//ui functions,  yeah this could be a class or somethin
void SetMoney(double _money);


#endif
