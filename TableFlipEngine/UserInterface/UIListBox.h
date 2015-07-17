/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UIView.h"
#include "UITextOut.h"

///list of text
///this is not very efficient, but for simplicity and sanity
///i am rendering each line of text independently.
///use vboPos and vboTc in subclasses of this if ya like
///their buffer ID's are generated in constructor here
class UIListBox : public UIView
{
private:
    unsigned int itemHeight;
    UITextOut   *selection;
    UITextOut   *hoverElem;
    Vector2      highlightCoords;
    Vector2      hoverCoords;

    //updates texcoord buffer
    void InvalidateTexcoords();
public:
    UIListBox();
    ~UIListBox();

    ///add text element, and set its (void * Element::data)
    UIElement *AddToList(wchar_t *text, void *data);

    ///note, this deletes the element as it was created in listbox too
    ///if not found, we probably just leaked memory
    bool RemoveFromList(UIElement *elem);

    virtual void MouseDown(Vector2 pt);
    virtual void HoverFunc();
    virtual void DrawPreText(); //renders right before text
    virtual void Invalidate();
    virtual void Update();
    virtual void MouseOut();
    
    void SetHighlightCoords(Vector2 tc);
    void SetHoverCoords(Vector2 tc);  
    void InvalidateSelection(); 

    inline UIElement *getSelection() { return selection; }
};
