/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UISPRITE_H__
#define __UISPRITE_H__

#include "UserInterface.h"

//TODO sprite sheets, so we can batch instead of obscene amounts of texture changes
class UISprite
{
private:
  GLuint texCoordBuffer;
  float scale;
  //TODO: rotation

  TextureResource *texture;
  UISprite();
public:
  UISprite(TextureResource *tex) { scale = 1.0f; texture = tex; tex->AddRef(); }


};

#endif
