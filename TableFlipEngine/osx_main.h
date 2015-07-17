/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef _OSX_MAIN_H__
#define _OSX_MAIN_H__
#define MOBILE_SHADOWS 1

bool InitEngine(unsigned int _width, unsigned int _height);
void UpdateEngine();
void ShutdownEngine();
void ResizeWindow(unsigned int _width, unsigned int _height);

void MouseDown(int btn, float x, float y);
void MouseUp(int btn, float x, float y);
//absolute position, and delta
void MouseMove(float x, float y);
void SetWindowRect(float x, float y, float w, float h);

void KeyPressed(unsigned short key);
void KeyReleased(unsigned short key);

#endif
