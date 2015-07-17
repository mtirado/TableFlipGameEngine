/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UITextOut.h"
//TODO probably should rename this to UIText
void UITextOut::Init()
{
    elementId = ELEM_UITEXTOUT;
    bufferIndex = 0;

    //insert default font here
    fontFamily = std::string("Data/Fonts/SAVINGSB_.TTF");
    fontStyle.family  = const_cast<char *>(fontFamily.c_str());
    fontStyle.font    = 0;
    fontStyle.size    = 16.0;
    dimensions.y      = ptToPx(16.0);
    fontStyle.bold    = 0;
    fontStyle.italic  = 0;
    fontStyle.rise    = 0.0;
    fontStyle.spacing = 0.0;
    fontStyle.gamma   = 1.0;

    fontStyle.foreground_color    = black;//white;
    fontStyle.background_color    = white;
    fontStyle.underline           = 0;
    fontStyle.underline_color     = white;
    fontStyle.overline            = 0;
    fontStyle.overline_color      = white;
    fontStyle.strikethrough       = 0;
    fontStyle.strikethrough_color = white;

    //is set when added to window
    fontHeight     = 0;
    textDimensions = Vector2(0.0); //set by freetype-gl
    textOffset     = Vector2(0.0);
    glyphsDrawn    = 0;
}

void UITextOut::SetFont(string file)
{
    fontFamily = file;
    fontStyle.family  = const_cast<char *>(fontFamily.c_str());
}

void UITextOut::SetFontSize(float size)
{
    fontStyle.size = size;
    float minHeight = ptToPx(size);
    if (dimensions.y < minHeight)
    {
        dimensions.y = minHeight;
        Invalidate();
    }
}


//NOTE if you invalidate more than once per frame, bad shit is going to happen
//forget why exactly... but that bug should be fixed with the new invalidate flags
//InvalidateStaticText() just sets a boolean value in window.
void UITextOut::Invalidate()
{ 
    UIElement::Invalidate();

    /*char output_[512];
      wcstombs(output_, output.c_str(), 512);
      LOGOUTPUT << "text : " << output_;
      LogInfo();*/

    parentWindow->InvalidateStaticText();
}

void UITextOut::SetOffset(Vector2 pos)
{
    offset = pos;
    UIElement::Invalidate();
}

void UITextOut::CalcFontHeight()
{
    fontHeight = text_buffer_get_font_height( parentWindow->getStaticText(), 
                                              &fontStyle);
}

void UITextOut::AddToWindow(UIWindow *wnd)
{
    
    UIElement::AddToWindow(wnd);
    parentWindow->AddStaticText(this);

    //area spans the whole 'line' in view
    /* dimensions.x  = wnd->dimensions.x
       - wnd->skin.getInfo(UI_SKIN_SCROLLBAR)->width 
       - wnd->skin.getInfo(UI_SKIN_FRAME)->width
       - wnd->getWindowPadding().x;
       */
    CalcFontHeight();
}

UITextOut::UITextOut()
{
    Init();
}

//set to true if text needs to be updated every frame.
UITextOut::UITextOut(const wchar_t *text)
{
    Init();
    output.fill(text);
    //fontStyle.font = texture_font_new_from_file( atlas, 13, "Data/Fonts/VeraMono.ttf" );
    //buffer = text_buffer_new( 1 );

}

UITextOut::~UITextOut()
{

}
