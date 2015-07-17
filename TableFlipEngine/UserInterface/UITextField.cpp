/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#include "text-buffer.h"
#include "UITextField.h"

UITextField::UITextField() 
{
    allowInput = true;
    text.SetFont("Data/Fonts/VeraMono.ttf");
    text.SetFontSize(10);
    text.SetGamma(1.0);
    cursor.SetFont("Data/Fonts/VeraMono.ttf");
    cursor.SetFontSize(40);
    cursor.SetGamma(1.0);
    cursor.output.clear();
    cursor.output.append(L"|");
    //no dimensions = do not try to constrain
    cursor.SetDimensions(Vector2(0.0));     
    //after mouse is released, hold the focus untill
    //mousedown, escape, or enter, or whatever other keys
    retainFocus   = true;
    inFocus       = false;
    cursorVisible = false;
    blinkDelay    = 400;
    lastBlink     = 0;
    textOffset    = Vector2(0.0, 0.0);
    frameWidth    = 0;
    skinVerts     = 0;
    numVerts      = 0;
    glGenBuffers(1, &vboPos);
}

UITextField::~UITextField()
{
    glDeleteBuffers(1, &vboPos);
    if (skinVerts)
        delete[] skinVerts;
}

void UITextField::CheckMinDimensions()
{
    float minHeight = ptToPx(text.getFontPtSize());
    if (dimensions.y < minHeight + frameWidth+frameWidth)
    {
        dimensions.y = minHeight + frameWidth+frameWidth;
        // dimensions.y += ; //some extra padding
        Invalidate();
    }
}

void UITextField::SetFontSize(float size)
{
    text.SetFontSize(size);
    cursor.SetFontSize(size);
    CheckMinDimensions();
}

void UITextField::SetFontWeight(float w)
{ 
    text.SetGamma(w);
    cursor.SetGamma(w);
}

void UITextField::SetOffset(Vector2 pos)
{
    UIElement::SetOffset(pos);
    text.SetOffset(offset);
    Invalidate();
}


void UITextField::AddToWindow(UIWindow* wnd)
{

    UIElement::AddToWindow(wnd);
    vboTc      = wnd->skin.getTexcoords(UI_SKIN_TEXTFIELD);
    frameWidth = wnd->skin.getInfo(UI_SKIN_TEXTFIELD)->width;
    numVerts   = wnd->skin.getNumVerts(UI_SKIN_TEXTFIELD);
    //just delete the old ones    
    if (skinVerts)
    {
        delete[] skinVerts;
    }
    skinVerts = new Vector2[numVerts];


    text.offset = offset;
    CheckMinDimensions();
    text.SetDimensions(dimensions);
    text.AddToWindow(wnd);

    //move text a bit for frame
    textOffset = Vector2(frameWidth);
}




void UITextField::Invalidate()
{
    UIElement::Invalidate();
    text.Invalidate();
    //note, this is not tested with large offsets
    //just frame width...
    text.textOffset   = textOffset;
    cursor.textOffset = textOffset;

    unsigned int num; 
    parentWindow->skin.BuildTextArea(position, dimensions,
            UI_SKIN_TEXTFIELD, skinVerts, &num); 

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * num, 
            skinVerts, GL_DYNAMIC_DRAW); 

}



void UITextField::Draw()
{
    if (hidden)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(gUIManager->GetPositionLocation());
    glVertexAttribPointer(gUIManager->GetPositionLocation(), 
            2, GL_FLOAT, GL_FALSE, 0,0);

    glBindBuffer(GL_ARRAY_BUFFER, vboTc);
    glEnableVertexAttribArray(gUIManager->GetTexcoordLocation());
    glVertexAttribPointer(gUIManager->GetTexcoordLocation(), 2, 
            GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);


}


//all the cursor hackery here
void UITextField::MouseDown(Vector2 pt)
{
    if (!allowInput)
        return; // relying on automatic invalidation at the moment

    if (text.output.length() < 1)
        return;

    lastBlink = 0;
    cursorVisible = false;
    //find the cursor 
    unsigned int strIdx = text.getBufferIndex();
    unsigned int idx = strIdx; //global window text index
    const text_buffer_t *buff = parentWindow->getStaticText();
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

    int i;                             //4 verts per glyph
    for (i = 0; i < text.output.length()*4; i+=4)
    {
        //disregard y, only care about first xposition(single line only)
        check = Vector2(glyph[i].x, glyph[i].y);
        dist  = fabs(pt.x - check.x);
        if (dist < minDist)
        {
            minIdx  = idx;
            min     = check;
            minDist = dist;
        }
        idx++;
    }
    i--;//rewind to last index

    //check the last vert (clicked after text)
    check = Vector2(glyph[i].x, glyph[i].y);
    dist  = fabs(pt.x - check.x); 
    if (dist < minDist) //nothing to append here (end of text)
    {
        minIdx  = idx;
        min     = check;
        minDist = dist;
    }

    //set the cursor position
    cursorIdx   = minIdx - strIdx; //local index to the left of cursor
}

void UITextField::FocusFunc()
{

    inFocus = true;

    if (allowInput)
    {
        //there was a print error, probably no text (just cleared)
        if (text.getBufferIndex() < 0)
        {
            cursorIdx = 0;
        }

        text_buffer_t *buff = parentWindow->getStaticText();

        //get cursor glyph so we know its offset
        texture_glyph_t *cursGlyph = text_buffer_get_glyph(buff, 
                &cursor.fontStyle, L'|');

        //update cursor position
        if (cursorIdx > 0)
        {
            if (cursorIdx >= text.getNumGlyphsDrawn())
                cursorIdx = text.getNumGlyphsDrawn();

            unsigned int vertIdx = (cursorIdx)*4;

            size_t sz = vector_size(buff->buffer->vertices);
            if (vertIdx > sz)
            {
                if (sz > 0)
                    vertIdx = sz;
                else 
                    vertIdx = 0; //failsafe
            }
            vertIdx += (text.getBufferIndex())*4;

            //get glyph position
            glyph_vertex_t * scrnGlyph = 
                (glyph_vertex_t *)vector_get(buff->buffer->vertices,vertIdx-1);

            cursorPos.x = scrnGlyph->x - cursGlyph->offset_x;
            cursorPos.y = position.y;

        }
        else //no glyphs, use elem pos
        {
            cursorPos = position;
            cursorPos.x -= cursGlyph->offset_x;
        }


        //blink cursor
        if (GetTimecode() > lastBlink + blinkDelay)
        {
            cursorVisible = !cursorVisible;
            lastBlink = GetTimecode();
            if (cursorVisible)
                parentWindow->AddStaticText(&cursor);
            else
                parentWindow->RemoveStaticText(&cursor);
        }

        //constrain cursor
        if (cursorPos.x > position.x + text.dimensions.x)
            cursorPos.x = position.x + text.dimensions.x;

        //update pos
        cursor.SetOffset(Vector2(cursorPos.x, cursorPos.y));
        cursor.Invalidate();
        //TODO this forces a redraw while text fields in foxus
        //not very efficient.
        parentWindow->InvalidateStaticText();
        CheckInput();
    }

}

void UITextField::CheckInput()
{
    bool solidCursor = false;
    bool hiddenCursor = false;
    //make sure text will be within bounds
    char c = gInput->GetNextAsciiChar(); //TODO unicode input?
    if (c)
    {
        wchar_t wc;
        mbstowcs(&wc, &c, 1); //convert to unicode

        if (text.getLength() > 0 && text.lastGlyph)
        {
            unsigned int strIdx = text.getBufferIndex();
            text_buffer_t *buff = parentWindow->getStaticText();

            //get glyph vert data from freetype-gl glyph
            glyph_vertex_t *glyph = 
                (glyph_vertex_t *)vector_get(buff->buffer->vertices,strIdx*4);
            int idx = text.getNumGlyphsDrawn()*4;//text.getLength()*4;
            int xMax = glyph[idx-1].x; //check last vert
            int xAdvance = text.lastGlyph->advance_x;
            int xOffset  = text.lastGlyph->offset_x;
            int xWidth = text.lastGlyph->width;
            //make sure at least 1 glyph has been printed

            /*LOGOUTPUT   << "idx: " << idx 
              << "\nxMax: " << xMax
              << "\nxWidth: " << xWidth 
              << "\nxAdvance: " << xAdvance;
              LogInfo();*/

            texture_glyph_t *nextGlyph = 
                text_buffer_get_glyph(buff, &text.fontStyle, wc);
            xOffset = nextGlyph->offset_x;
            xWidth  = nextGlyph->width;

            //make sure the next char will be within bounds
            if (xMax + xWidth + xOffset + xAdvance >= text.getPosition().x 
                    + text.getDimensions().x)
            {
                c = 0;  
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
        if (cursorIdx < text.getBufferIndex() + text.getNumGlyphsDrawn())
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
        parentWindow->InvalidateStaticText();
        cursor.Invalidate();
        text.Invalidate(); 
        lastBlink = 0;
        cursorVisible = false; //blink will toggle this to true
    }

}


void UITextField::DropFocus()
{
    inFocus = false;
    color = tmpColor; //restore pre focus color
    if (cursorVisible)
        parentWindow->RemoveStaticText(&cursor);
}

void UITextField::HoverFunc()
{

}


void UITextField::InsertCharAt(unsigned int idx, wchar_t c)
{
    wchar_t str[2];
    if (c == L' ')
        c = L' ';
    str[0] = c;
    str[1] = 0; //needs null terminator TODO TString::append(wchar_t c)

    //check if inserting at beginning or end
    if (idx >= text.getLength())
    {
        text.output.append(str);
    }
    else if (idx == 0)
    {
        TString<wchar_t> right(text.output);
        text.output.fill(str);
        text.output.append(right);
    }
    else
    {
        //split the string, append char, append right side
        TString<wchar_t> left;
        left.fill(text.output.c_str(), idx);
        left.append(str);
        left.append(&(text.output.c_str())[idx]);
        text.output.fill(left);        
    }
}

void UITextField::InsertTextAt(unsigned int idx, TString< wchar_t >& text)
{

}

void UITextField::RemoveCharacterBefore(unsigned int idx)
{
    if (text.getLength() == 0)
        return;
    if (idx > text.getLength() || idx == 0)
        return;

    TString<wchar_t> left, right;

    if (idx > 1)
        left.fill(text.output, idx-1);

    unsigned int len = text.getLength();
    //if anything to right, append it
    if (idx < len)
    {
        right.fill(&(text.output.c_str())[idx]);
        text.output.clear();
        text.output.append(left);
        text.output.append(right);
    }
    else
    {
        text.output.clear();
        text.output.append(left);
    }
}



