/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __UITEXTVIEW_H__
#define __UITEXTVIEW_H__

#include "UIElement.h"
#include "UITextOut.h"
#include "UIView.h"

/**
 * some damn near duplicate code found in UITextField.
 * for input handling and whatnot...  this is pretty much
 * the 'view' equivalent of a text field, 
 * and allows multilines as well.
 */


///multi line scrollable text view
class UITextView : public UIView
{
protected:
    UITextOut *text;          //ui text elem
    UITextOut *cursor;        //input cursor
    Vector2 textOffset;       //need adjustments?

    Vector2 cursorPos; //if in focus, draw cursor here
    bool  inFocus;
    bool  cursorVisible;
    int   numVerts;       

    unsigned int lastBlink; //timestamp for last cursor blink
    unsigned int blinkDelay;
    unsigned int cursorIdx;
    float frameWidth;
    //insert a single wchar after supplied idx
    void InsertCharAt(unsigned int idx, wchar_t c);
    //inserts a string after supplied index
    void InsertTextAt(unsigned int idx, TString<wchar_t> &text);
    //remove char at index
    void RemoveCharacterBefore(unsigned int idx);

    void CheckMinDimensions();
    
public:
   
    UITextView();
    ~UITextView();

    
    bool allowInput;

    virtual void SetOffset(Vector2 pos);
    virtual void SetDimensions(Vector2 dim);
    virtual void AddToWindow(UIWindow* wnd);
    virtual void Invalidate();
    virtual void MouseDown(Vector2 pt);
    virtual void FocusFunc();
    virtual void DropFocus();
    virtual void HoverFunc();
    virtual void Draw();
    void CheckInput();

    void SetTextOffset(Vector2 off); 

    Vector2 GetTextOffset() { 
        return textOffset;
    }

    //set text using this
    //virtual TString<wchar_t> &getString() { return text->output; }
    virtual void SetText(wchar_t *str);
    virtual void AppendText(wchar_t *str);

    void SetFontSize(float size);
    void SetFontWeight(float w);  

    
};

#endif
