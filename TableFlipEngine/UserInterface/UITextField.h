/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UITEXTEDIT_H__
#define __UITEXTEDIT_H__

#include "UIElement.h"
#include "UITextOut.h"
///TODO TODO make this a omnipotent all doing, text input class.
///a single line text field
class UITextField : public UIElement
{
protected:
    Vector2 textOffset;
    Vector4 tmpColor;
    Vector2 cursorPos; //if in focus, draw cursor here
    Vector2 *skinVerts;

    bool  inFocus;
    bool  cursorVisible;
    float frameWidth;
    int   numVerts;       

    unsigned int lastBlink; //timestamp for last cursor blink
    unsigned int blinkDelay;
    //just temp public for debug output
public:
    UITextOut cursor;  //overlay the cursor
    //index in local element string(not all window strings)
protected:
    unsigned int cursorIdx;
    unsigned int newlines; //to make glyph index adjustments,
    //insert a single wchar after supplied idx
    void InsertCharAt(unsigned int idx, wchar_t c);
    //inserts a string after supplied index
    void InsertTextAt(unsigned int idx, TString<wchar_t> &text);
    //remove char at index
    void RemoveCharacterBefore(unsigned int idx);
    void CheckMinDimensions();

    UITextOut text;
public:
    UITextField();
    ~UITextField();

    //set text using this
    virtual TString<wchar_t> &getString() { return text.output; }

    bool allowInput;

    virtual void SetOffset(Vector2 pos);
    virtual void AddToWindow(UIWindow* wnd);
    virtual void Invalidate();
    virtual void MouseDown(Vector2 pt);
    virtual void FocusFunc();
    virtual void DropFocus();
    virtual void HoverFunc();
    virtual void Draw();
    virtual void CheckInput();

    void SetTextOffset(Vector2 off) { 
        textOffset = off; 
        SetOffset(offset);  
    }

    Vector2 GetTextOffset() { 
        return textOffset;
    }

    
    void SetFontSize(float size);
    void SetFontWeight(float w);    
    
};

#endif
