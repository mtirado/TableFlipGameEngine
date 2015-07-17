/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "UITextView.h"

UITextView::UITextView()
{
    text = new UITextOut(L"ABCDE TEST");
    cursor = new UITextOut(L"|");

    text->SetDimensions(Vector2(0.0));
    text->SetOffset(Vector2(0.0));
    text->SetFont("Data/Fonts/VeraMono.ttf");
    text->SetFontSize(10);
    text->SetGamma(1.0);
    cursor->SetFont("Data/Fonts/VeraMono.ttf");
    cursor->SetFontSize(10);
    cursor->SetGamma(1.0);
    //no dimensions = do not  constrain
    cursor->SetDimensions(Vector2(0.0));     
    theWin.setWindowPadding(0.0, 0.0);
    //text->AddToWindow(&theWin);
    //theWin.AddStaticText(text);
    //theWin.AddElement(text);
    theWin.dimensions = dimensions;

    //theWin.AddElement(cursor);
    frameWidth = 0.0;
    allowInput = true;
    retainFocus = true;
    inFocus = false;
    cursorVisible = false;
    blinkDelay = 400;
    lastBlink = 0;

}

UITextView::~UITextView()
{
    //the window will clean up our elements for us.
}

void UITextView::SetDimensions(Vector2 dim)
{
    UIView::SetDimensions(dim);
    text->SetDimensions(Vector2(dim.x, 0.0));
    //TODO figure out where to word wrap
    Invalidate();
}


void UITextView::SetTextOffset(Vector2 off)
{
    textOffset = off;
    text->textOffset = off;
}

//TODO every time text->is added, recalc scroll dimensions.






void UITextView::SetFontSize(float size)
{
    text->SetFontSize(size);
    cursor->SetFontSize(size);
    CheckMinDimensions();
}

void UITextView::SetFontWeight(float w)
{ 
    text->SetGamma(w);
    cursor->SetGamma(w);
}

void UITextView::SetOffset(Vector2 pos)
{
    UIElement::SetOffset(pos);
    text->SetOffset(offset);
    cursor->SetOffset(offset);
    Invalidate();
}

void UITextView::CheckMinDimensions()
{
    float minHeight = ptToPx(text->getFontPtSize());
    if (dimensions.y < minHeight + frameWidth+frameWidth)
    {
        dimensions.y = minHeight + frameWidth+frameWidth;
        // dimensions.y += ; //some extra padding
        Invalidate();
    }
}


void UITextView::AddToWindow(UIWindow* wnd)
{

    UIElement::AddToWindow(wnd);

    theWin.dimensions = dimensions;

    //add and reset pos/dimensions
    theWin.AddElement(text);
    text->SetOffset(Vector2(0.0, 0.0));

    frameWidth = theWin.getFramePixels();
    // numVerts   = wnd->skin.getNumVerts(UI_SKIN_TEXTFIELD);
    //just delete the old ones    
    /* if (skinVerts)
       {
       delete[] skinVerts;
       }
       skinVerts = new Vector2[numVerts];

*/
    //text->offset = offset;
    CheckMinDimensions();
    text->SetDimensions(Vector2(dimensions.x, 0.0));
    //theWin.AddElement(text);
    //text->AddToWindow(wnd);

    //disabled becuase frame must be created AFTER view is added
    //annoying little bug right now
    //move text a bit for frame
    //textOffset = Vector2(theWin.getFramePixels());
    //text->textOffset   = textOffset;
    //cursor->textOffset = textOffset;
}




void UITextView::Invalidate()
{
    UIView::Invalidate();
    theWin.InvalidateStaticText();
    //note, this is not tested with large offsets
    //just frame width..
    frameWidth = theWin.getFramePixels();

    textOffset = Vector2(frameWidth);
    text->textOffset   = textOffset;
    cursor->textOffset = textOffset;
    cursor->Invalidate();
    text->Invalidate();


    //this is how UITextField renders
    /*unsigned int num; 
      parentWindow->skin.BuildTextArea(position, dimensions,
      UI_SKIN_TEXTFIELD, skinVerts, &num); 

      glBindBuffer(GL_ARRAY_BUFFER, vboPos);
      glEnableVertexAttribArray(gUIManager->GetPositionLocation());
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * num, 
      skinVerts, GL_DYNAMIC_DRAW); */

}



void UITextView::Draw()
{
    UIView::Draw();


    //UITextField Draw
    /*if (hidden)
      return;

      glBindBuffer(GL_ARRAY_BUFFER, vboPos);
      glEnableVertexAttribArray(gUIManager->GetPositionLocation());
      glVertexAttribPointer(gUIManager->GetPositionLocation(), 
      2, GL_FLOAT, GL_FALSE, 0,0);

      glBindBuffer(GL_ARRAY_BUFFER, vboTc);
      glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
      glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
      GL_FLOAT, GL_FALSE, 0, 0);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);*/


}


//all the cursor hackery here
void UITextView::MouseDown(Vector2 pt)
{

    if (!allowInput)
        return; 
    if (text->output.length() < 1)
        return;

    lastBlink = 0;
    cursorVisible = false;
    //find the cursor 
    unsigned int strIdx = text->getBufferIndex();
    unsigned int idx = strIdx; //global window text->index
    text_buffer_t *buff = theWin.getStaticText();
    //get glyph vert data from freetype-gl
    glyph_vertex_t * glyph = 
        (glyph_vertex_t *)vector_get(buff->buffer->vertices, idx*4);

    //figure out a cursor position using glyph vert data    
    Vector2 min; //closest point to cursor
    min = Vector2(glyph[0].x, pt.y);
    float minDist = fabs(pt.x - min.x);
    int minIdx = idx;
    Vector2 check, vecTo;
    float dist;
    float x;
    float fontHeight = text_buffer_get_font_height(buff, &text->fontStyle);

    float screenHeight = gRenderer->height;
    int i;                             //4 verts per glyph
    for (i = 0; i < text->output.length()*4; i+=4)
    {
        //note, font.y is bottom oriented, ui points are top oriented
        dist  = fabs(pt.y - (screenHeight - glyph[i].y));
        
        if (dist < fontHeight)
        {
            LOGOUTPUT << "glyph[" << i << "].y: " << screenHeight - glyph[i].y
                  << "\npt.y: " << pt.y
                  << "\ndist: " << dist
                  << "\nfont height: " << fontHeight; LogInfo();

            check = Vector2(glyph[i].x, glyph[i].y);
            dist  = fabs(pt.x - check.x);
            if (dist < minDist)
            {
                minIdx  = idx;
                min     = check;
                min.y   = screenHeight - min.y; //convert base
                minDist = dist;
            }
        }
        idx++;
    }
    i--;//rewind to last index

    //check the last vert (clicked after text->
    check = Vector2(glyph[i].x, glyph[i].y);
    dist  = fabs(pt.x - check.x); 
    if (dist < minDist) //nothing to append here (end of text->
    {
        minIdx  = idx;
        min     = check;
        min.y   = screenHeight - min.y;
        minDist = dist;
    }

    //set the cursor position
    cursorIdx   = minIdx - strIdx; //local index to the left of cursor
}

void UITextView::FocusFunc()
{
    inFocus = true;

    if (allowInput)
    {
        //there was a print error, probably no text, (just cleared)
        if (text->getBufferIndex() < 0)
        {
#if defined(UI_DEBUG_MODE)
            LogError("UITextView::FocusFunc() -- negative buffer index");
#endif
            cursorIdx = 0;
        }

        text_buffer_t *buff = theWin.getStaticText();

        LOGOUTPUT << " cursor Idx: " << cursorIdx 
                  << " strlen: " << text->output.size(); LogInfo();
        //get cursor glyph so we know its offset
        texture_glyph_t *cursGlyph = text_buffer_get_glyph(buff, 
                &cursor->fontStyle, L'|');
        //texture_glyph_t *selectGlyph = text_buffer_get_glyph(buff,
        //        &text->fontStyle, text->output.c_str()[cursorIdx]);

        //update cursor position
        if (cursorIdx > 0)
        {
            if (cursorIdx >= text->getNumGlyphsDrawn())
                cursorIdx = text->getNumGlyphsDrawn();

            unsigned int vertIdx = (cursorIdx)*4;


            size_t sz = vector_size(buff->buffer->vertices);
            if (vertIdx > sz)
            {
                if (sz > 0)
                    vertIdx = sz;
                else 
                    vertIdx = 0; //failsafe
            }
            vertIdx += (text->getBufferIndex())*4;



            //get glyph position
            glyph_vertex_t * glyphVert = 
                (glyph_vertex_t *)vector_get(buff->buffer->vertices,vertIdx-1); //get top vert

            cursorPos.x = glyphVert->x - cursGlyph->offset_x;
            //find the 'base line'
            //some trickery. get the row offset, and calculate the y baseline
            //based on font height.  FONT HEIGHT is the key... whilst converting 
            //coordinate system y base  :( 
            int row = (position.y - (gRenderer->height - glyphVert->y))
                    / text->fontStyle.font->height;
            cursorPos.y = position.y - row * text->fontStyle.font->height;

            //LOGOUTPUT << "cursorPos: " << cursorPos; LogInfo();

        }
        else //no glyphs, use elem pos
        {
            cursorPos = position;
            cursorPos.x -= cursGlyph->offset_x;
        }


        if (GetTimecode() > lastBlink + blinkDelay)
        {
            cursorVisible = !cursorVisible;
            lastBlink = GetTimecode();
            if (cursorVisible)
            {
                theWin.AddStaticText(cursor);
                theWin.AddStaticText(text);
            }
            else
                theWin.RemoveStaticText(cursor);
        }

        if (cursorPos.x > position.x + text->getDimensions().x)
            cursorPos.x = position.x + text->getDimensions().x;
        cursor->SetOffset(Vector2(cursorPos.x - theWin.getWindowPadding().x,
                    cursorPos.y));
        theWin.InvalidateStaticText();
        CheckInput();
    }
}

void UITextView::CheckInput()
{

    //yup, prevents flying cursor of dooooomif (cursorIdx >= text->getNumGlyphsDrawn())
    if (cursorIdx > text->getNumGlyphsDrawn())
    {
        LogError("failure.");
        cursorIdx = text->getLength();
        //cursorIdx = text->getLength();
    }
    bool solidCursor = false;
    bool hiddenCursor = false;
    //make sure text->will be within bounds/
    char c = gInput->GetNextAsciiChar(); //TODO unicode input?
    if (c)
    {
        wchar_t wc;
        mbstowcs(&wc, &c, 1); //convert to unicode

        if (text->getLength() > 0 && text->lastGlyph)
        {
            unsigned int strIdx = text->getBufferIndex();
            text_buffer_t *buff = theWin.getStaticText();

            //get glyph vert data from freetype-gl glyph
            glyph_vertex_t *glyph = 
                (glyph_vertex_t *)vector_get(buff->buffer->vertices,strIdx*4);
            int idx = text->getNumGlyphsDrawn()*4;//text->getLength()*4;
            int xMax = glyph[idx-1].x; //check last vert
            int xAdvance = text->lastGlyph->advance_x;
            int xOffset  = text->lastGlyph->offset_x;
            int xWidth = text->lastGlyph->width;
            //make sure at least 1 glyph has been printed

            /*LOGOUTPUT   << "idx: " << idx 
              << "\nxMax: " << xMax
              << "\nxWidth: " << xWidth 
              << "\nxAdvance: " << xAdvance;
              LogInfo();*/


            texture_glyph_t *nextGlyph = 
                text_buffer_get_glyph(buff, &text->fontStyle, wc);
            xOffset = nextGlyph->offset_x;
            xWidth  = nextGlyph->width;

            //make sure the next char will be within bounds
            if (xMax + xWidth + xOffset + xAdvance >= text->getPosition().x 
                    + text->getDimensions().x)
            {
                //c = 0;  
                //this means we can't type anymore         
                //LogWarning("no room left to print, or move right");   
            }
        }
        if (c)
        {
            InsertCharAt(cursorIdx, wc);
            cursorIdx++;
            solidCursor = true;
        }
    }

    if (gInput->GetKeyDownThisFrame(KB_BACKSPACE))
    {
        RemoveCharacterBefore(cursorIdx);
        if (cursorIdx > 0)
            cursorIdx--;
        solidCursor = true;
    }
    if (gInput->GetKeyDownThisFrame(KB_DELETE))
    {
        RemoveCharacterBefore(cursorIdx+1);
        solidCursor = true;
    }
    if (gInput->GetKeyDownThisFrame(KB_LEFT))
    {
        if (cursorIdx > 0)
            cursorIdx--;
        solidCursor = true;
    }
    if (gInput->GetKeyDownThisFrame(KB_RIGHT))
    {
        if (cursorIdx < text->getBufferIndex() + text->getNumGlyphsDrawn())
            cursorIdx++;
        solidCursor = true;
    }
    if (gInput->GetKeyDownThisFrame(KB_RETURN))
    {
        if (actionCallback)
            actionCallback(this, 0);
        solidCursor = true;
    }
    //make the cursor visible
    if (solidCursor)
    {
        theWin.InvalidateStaticText(); 
        lastBlink = 0;
        cursorVisible = false; //blink will toggle this to true
    }


}


void UITextView::DropFocus()
{
    inFocus = false;
    //color = tmpColor; //restore pre focus color
    if (cursorVisible)
        theWin.RemoveStaticText(cursor);
}

void UITextView::HoverFunc()
{

}


void UITextView::InsertCharAt(unsigned int idx, wchar_t c)
{
    wchar_t str[2];
    if (c == L' ')
        c = L' ';
    str[0] = c;
    str[1] = 0; //needs null terminator TODO TString::append(wchar_t c)

    //check if inserting at beginning or end
    if (idx >= text->getLength())
    {
        text->output.append(str);
    }
    else if (idx == 0)
    {
        TString<wchar_t> right(text->output);
        text->output.fill(str);
        text->output.append(right);
    }
    else
    {
        //split the string, append char, append right side
        TString<wchar_t> left;
        left.fill(text->output.c_str(), idx);
        left.append(str);
        left.append(&(text->output.c_str())[idx]);
        text->output.fill(left);        
    }
}


void UITextView::RemoveCharacterBefore(unsigned int idx)
{
    if (text->getLength() == 0)
        return;
    if (idx > text->getLength() || idx == 0)
        return;

    TString<wchar_t> left, right;

    if (idx > 1)
        left.fill(text->output, idx-1);

    unsigned int len = text->getLength();
    //if anything to right, append it
    if (idx < len)
    {
        right.fill(&(text->output.c_str())[idx]);
        text->output.clear();
        text->output.append(left);
        text->output.append(right);
    }
    else
    {
        text->output.clear();
        text->output.append(left);
    }
}

//get text will need to strip \r's from the string
void UITextView::SetText(wchar_t *str)
{
    //text->output.clear();
    //text->output.append(str);
}


///TODO only check word wrap for the text that was just appended
///not the whole damn thing.
void UITextView::AppendText(wchar_t *str)
{
    //CheckWordWrap();
}

