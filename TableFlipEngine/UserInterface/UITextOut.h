/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UITEXTOUT_H__
#define __UITEXTOUT_H__

#include "UIElement.h"
#include "UIWindow.h"

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"
#include "shader.h"
#include "mat4.h"
#include "TString.h"

static vec4 white = {{0,1,1,1}};
static vec4 blue  = {{0,0,1,1}};
static vec4 black = {{0,0,0,1}};
static vec4 none  = {{0,0,1,0}};

///textout should have 0 dimensions, unless you need it to be constrained (word wrapped)
class UITextOut : public UIElement
{
    //we're all friends here i hope...
    friend class UIWindow; //prints these things
    friend class UITextField;
    friend class UITextView;

private:
    ///starting index in freetype-gl text buffer
    int bufferIndex;
    /**number of glyphs rendered by text_buffer_printf_index
     * bufferIndex + glyphsDrawn = last index rendered before
       text_buffer_t truncation */
    int glyphsDrawn;
    
    float fontHeight;
    ///the dimensions of the text drawn to screen
    Vector2 textDimensions;
protected:
    //freetype example stuff
    std::string     fontFamily;

    /**if true, font rendering was constrained, and we should
       remove a character from the buffer*/
    bool blockInput;

    ///last glyph that was rendered
    texture_glyph_t *lastGlyph;




public:

    UITextOut();
    //set to true if text needs to be updated every frame.
    UITextOut(const wchar_t *text);
    ~UITextOut();
    
    ///offset is added right before print
    Vector2 textOffset;

    TString<wchar_t> output;
    markup_t         fontStyle;
    
    virtual void MouseDown(Vector2 pt){}
    virtual void FocusFunc(){}
    virtual void DropFocus(){}
    virtual void Init();
    virtual void Invalidate();
    virtual void AddToWindow(UIWindow *wnd);
    virtual void SetOffset(Vector2 pos);

    void SetFont(std::string file);
    void SetFontSize(float size);
    void SetGamma(float g) { fontStyle.gamma = g; } 

    //calculates font height if parent != 0
    void CalcFontHeight();
    
    inline Vector2 getTextDimensions() { return textDimensions; }
    inline float getFontHeight()       { return fontHeight;     }
    inline int getBufferIndex()        { return bufferIndex;    }
    inline unsigned int getLength()    { return output.length();}
    inline int getNumGlyphsDrawn()     { return glyphsDrawn;    }
    inline float getFontPtSize()       { return fontStyle.size; } 
    

};

#endif
