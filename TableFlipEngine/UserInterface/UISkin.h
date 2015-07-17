/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UI_SKIN_H__
#define __UI_SKIN_H__
#include "UserInterface.h"
#include "Utilities.h"
#include <vector>
///define new skinnable objects here
enum UISkinnable 
{ 
    UI_SKIN_FRAME = 0, UI_SKIN_SCROLLBAR, UI_SKIN_LISTBOX,
    UI_SKIN_BTN,       UI_SKIN_BTN_HOVER, UI_SKIN_BTN_DOWN,
    UI_SKIN_TEXTFIELD, UI_SKIN_SLIDER,    
    UI_SKIN_NUM 
};

struct UISkinInfo
{
    float width;
    float height;
    UISkinInfo()
    {
        width  = 0.0;
        height = 0.0;
    }
};

/**manages texcoord positions for whatever elements you want
 * could be multiple skins but to avoid texture switching
 * always use ONE texture in ui, or sort by textures(gl;hf)
 * if you don't like the default ui skin, set up your own here!
 * this is really just a texcoord buffer manager. 
 * note that all point checks use rectangles, so custom
 * shapes would need custom 'collision' checks for mouse down/hover
 * or even better yet: UISprite */
class UISkin
{
private:
    //square power of two
    unsigned int atlasRes;

    ///texcoords (share these across skinned elements)
    GLuint texcoords[UI_SKIN_NUM];
    Vector2 atlasOrigin[UI_SKIN_NUM];
    //misc data about skin, eg: frame thickness
    UISkinInfo info[UI_SKIN_NUM];
    unsigned int numVerts[UI_SKIN_NUM];


public:    //vert + texcoord generators

    ///frame doesnt need margins since it lives on the edge!!
    void BuildFrame(Vector2 position, Vector2 dimensions,
                        Vector2 *out, unsigned int *outNum);

    ///position and dimension should be adjusted with frame/scrollbar/etc
    ///margins before sending to these functions
    void BuildScrollbar(Vector2 position, Vector2 dimensions, float scrollVal,
                        Vector2 *out, unsigned int *outNum);
    
    void BuildSlider(Vector2 position, Vector2 dimensions, float slideVal,
                        Vector2 *out, unsigned int *outNum);

    
    ///builds a button, (needs skin type for origin)
    void BuildButton(Vector2 position, Vector2 dimensions,
                    UISkinnable elem, Vector2 *out, unsigned int *outNum);

    ///elem is for different flavors of textfield 
    void BuildTextArea(Vector2 position, Vector2 dimensions,
                    UISkinnable elem, Vector2 *out, unsigned int *outNum);

    //no positions, only texcoords
    void BuildListBox();

    

public:
    UISkin(); //TODO load skin file??
   ~UISkin();

    
    ///clean up
    void Destroy();

    inline unsigned int getAtlasRes()
    {
        return atlasRes;
    }

    inline unsigned int getNumVerts(UISkinnable obj)
    {
        return (obj >= UI_SKIN_NUM) ? 0 : numVerts[obj];
    }
    inline GLuint getTexcoords(UISkinnable obj)
    {
        return (obj >= UI_SKIN_NUM) ? 0 : texcoords[obj];
    }

    inline UISkinInfo *getInfo(UISkinnable obj)
    {
        return (obj >= UI_SKIN_NUM) ? 0 : &info[obj];
    }
    
};

#endif




