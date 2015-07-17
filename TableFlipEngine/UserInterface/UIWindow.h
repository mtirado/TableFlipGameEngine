/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include "UserInterface.h"
#include "UIScrollbar.h"
#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "UISkin.h"

//returns boolean comparison between 2 elements
//imagine a less than symbol between, and lowest is thefirst element
//do not mix and match               < 
typedef bool (*elem_sort)(UIElement *, UIElement *);

class UISlider; //scrollbars
class UITextOut;
class UIWindow
{
    friend class UIView; //wraps window
private:
    GLuint vboPos;   //window area verts
    GLuint vboTc;    //window area texcoords
    GLuint vboFrame; //frame verts
    bool hidden;
    bool canDock;
    bool scrolls; //scrollbars?? (auto h and v)
    bool showTitlebar;
    //window frame
    float framePixels; //how many pixels wide
    Vector2 *frameVerts; //vertex data
    
    string Title;
    Vector2 aspectScale;
    //MUST CALL MOVE WINDOW, to invalidate. derpalerp
    Vector2 position; //do not set directly
    
    //maximum element bounding box point (min is 0,0)
    Vector2 elemMax;

    //rectangle that describes window content region 
    //adjusted for frame, scrollbar, etc(?)_.
    Vector4 windowArea; //l, r, t, b,
    //not really used at the moment
    Vector2 windowPadding; //added interior margin default: 2px
    
    bool winScrolled;
    UIScrollbar *vScroll;
    UIScrollbar *hScroll;
    Vector2 scrollOffset; //how scrolled?
    Vector2 scrollDimensions; //and to what magnitude?

    //calculates the scroll area
    void CalcScrollDimensions();
    //how much space occupied by window frame, scrollbars, etc
    void CalcWindowMargin();

    ///calculates maximum element bounding box
    void CalcMaximum();
    
    //all of the text in a window contained in this buffer
    text_buffer_t  *staticTextBuffer;  //does not change often

    ///static text only updates when invalidated
    std::vector<UITextOut *> staticText;

    bool updateStaticText;
    //update if invalidated in draw
    void UpdateStaticText();
    void UpdateText(text_buffer_t *buffer, std::vector<UITextOut *> &texts);

public:

    /** this is  hack to calculate text dimensions
    for text formatting operations that happen on 
    an elements creation (eg, centering button text)*/
    void UpdateTextDimensions();


    ///either of these will invalidate the text buffer
    void AddStaticText      (UITextOut *txt);
    void RemoveStaticText   (UITextOut *txt);

    /**sets buffer update flag, call this when you modify text
     * so the window can reprint the text */
    void InvalidateStaticText() { updateStaticText = true;  }
    ///add scrollbars to window
    void CreateScrollbars(bool horizon, bool vert);
    ///add frame to window
    void CreateFrame();
    ///update frame position
    void InvalidateFrame();

public:
    UIElement *elements; //linked list
    elem_sort  sortFunc; //the active sorting function

    Vector2 dimensions;
    unsigned char winStencilId;
    UISkin skin;

    /**returns element that has mouse down, 
      if none, will return 0 */
    virtual UIElement *MouseDown(Vector2 pt);

    ///return element we're hovering
    virtual UIElement *MouseHover(Vector2 pt);

    virtual void MouseUp();
    virtual void FocusFunc();
    virtual void Update();

    ///draw everything
    virtual void Draw();

    ///move window to this point
    virtual void MoveWindow(Vector2 pt);
    ///updates scroll offsets
    virtual void ScrollWindow();

    ///invalidates all elements (redraw buffers)
    void InvalidateElements();
   
    void AddElement(UIElement *elem, bool ignoreMargin = false);
    bool RemoveElement(UIElement *elem); 

    ///sets the texcoord used for solid background color
    void SetBGTexcoords(Vector2 tc);


    inline const Vector2 &GetPosition() 
    { 
        return position; 
    }

    inline void Hide() 
    {
        hidden = true; 
    }

    inline void Show() 
    { 
        hidden = false; 
    }

    inline bool isHidden() 
    { 
        return hidden;
    }

    inline text_buffer_t *getStaticText() 
    { 
        return staticTextBuffer;  
    }

    inline Vector2 getScrollOffset() 
    { 
        return scrollOffset;
    }

    //has the window been scrolled?
    inline bool hasScrolled() 
    {
        return winScrolled;
    }

    inline UISlider *getVScroll() const { return vScroll;    }
    inline UISlider *getHScroll() const { return hScroll;    } 
    inline float getYMax()        const { return elemMax.y;  }
    inline float getXMax()        const { return elemMax.x;  }
    inline float getFramePixels() const { return framePixels;}
    
    inline void setWindowPadding(float px, float py) 
    {
        windowPadding.x = px;
        windowPadding.y = py;
    }

    inline Vector2 getWindowPadding() { return windowPadding; }
    inline Vector4 getWindowArea()    { return windowArea;    }
    
    ~UIWindow();
    UIWindow();

};
#endif
