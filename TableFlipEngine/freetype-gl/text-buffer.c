/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "opengl.h"
#include "text-buffer.h"

#define SET_GLYPH_VERTEX(value,x0,y0,z0,s0,t0,r,g,b,a) { \
    glyph_vertex_t *gv=&value;                           \
    gv->x=x0; gv->y=y0; gv->z=z0;                        \
    gv->u=s0; gv->v=t0;                                  \
    gv->r=r; gv->g=g; gv->b=b; gv->a=a;} 


//#define SET_GLYPH_VERTEX(value,x0,y0,z0,s0,t0,r,g,b,a,sh,gm) { \
//gv->shift=sh; gv->gamma=gm;}


// ----------------------------------------------------------------------------

//HACK use a single global shader, the right way would involve a ShaderResource
GLuint gShader = 0;
unsigned int gWinWidth=1024;
unsigned int gWinHeight=1024;

void text_buffer_set_screensize(unsigned int width, unsigned int height)
{
    gWinWidth  = width;
    gWinHeight = height;
}


    text_buffer_t *
text_buffer_new( size_t depth )
{
    if (gShader == 0)
    {
        // G_Shader = shader_load("Data/Shaders/freetype-gl/text.vert",
        //                       "Data/Shaders/freetype-gl/text.frag");
        gShader = shader_load("Data/Shaders/freetype-gl/v3f-t2f-c4f.vert",
                "Data/Shaders/freetype-gl/v3f-t2f-c4f.frag");

    }

    text_buffer_t *self = (text_buffer_t *) malloc (sizeof(text_buffer_t));
    self->buffer = vertex_buffer_new(
            //"vertex:3f,tex_coord:2f,color:4f,ashift:1f,agamma:1f" );
        "vertex:3f,tex_coord:2f,color:4f" );
    //,ashift:1f,
    self->manager = font_manager_new( 512, 512, depth );
    self->shader  = gShader;
    self->shader_texture = glGetUniformLocation(self->shader, "texture");
    self->shader_pixel   = glGetUniformLocation(self->shader, "pixel");
    self->line_start     = 0;
    self->line_ascender  = 0;
    self->base_color.r   = 0.0;
    self->base_color.g   = 0.0;
    self->base_color.b   = 0.0;
    self->base_color.a   = 1.0;
    self->line_descender = 0;

    //constraint data
    self->dimensions.x = 0;
    self->dimensions.y = 0;
    self->lastGlyph    = 0; //needed for width
    return self;
}

// ----------------------------------------------------------------------------
    void
text_buffer_delete( text_buffer_t * self )
{
    vertex_buffer_delete( self->buffer );

    //HACK see above G_Shader. if multiple shaders are desired
    //use a texture resource, or a shader cache to limit duplicate programs
    //glDeleteProgram( self->shader );

    free( self );
}

// ----------------------------------------------------------------------------
    void
text_buffer_clear( text_buffer_t * self )
{    
    assert( self );

    vertex_buffer_clear( self->buffer );
    self->line_start = 0;
    self->line_ascender = 0;
    self->line_descender = 0;
    self->lastGlyph = 0;
    self->dimensions.x = 0;
    self->dimensions.y = 0;
}


// ----------------------------------------------------------------------------
    void
text_buffer_render( text_buffer_t * self )
{
    //make sure theres something to render. 
    if (self->buffer->indices->size < 3)
        return;


    glEnable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, self->manager->atlas->id );

    if( self->manager->atlas->depth == 1 )
    {
        //glDisable( GL_COLOR_MATERIAL );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendColor( 1, 1, 1, 1 );
    }
    else
    {
        //glEnable( GL_COLOR_MATERIAL );
        //glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE <LeftMouse>

        //looks good with heavier gamma
        //glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
        //glBlendColor( 1.0, 1.0, 1.0, 1.0 );

        //glBlendFunc( GL_CONSTANT_COLOR_EXT,  GL_ONE_MINUS_SRC_COLOR );
        // glBlendColor( self->base_color.r,
        // self->base_color.g,
        // self->base_color.b,
        // self->base_color.a );

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendColor( 1, 1, 1, 1 );
    }

    //glUseProgram( self->shader );
    glUniform1i( self->shader_texture, 0 );
    glUniform3f( self->shader_pixel,
            1.0/self->manager->atlas->width,
            1.0/self->manager->atlas->height,
            self->manager->atlas->depth );
    vertex_buffer_render( self->buffer, GL_TRIANGLES );
    //glUseProgram( 0 );
    glDisable(GL_BLEND);
    glEnable( GL_DEPTH_TEST );
}

// ----------------------------------------------------------------------------
    void
text_buffer_printf( text_buffer_t * self, vec2 *pen, ... )
{
    markup_t *markup;
    wchar_t *text;
    va_list args;

    if( vertex_buffer_size( self->buffer ) == 0 )
    {
        self->origin = *pen;
    }

    va_start ( args, pen ); 
    do {
        markup = va_arg( args, markup_t * );
        if( markup == NULL )
        {
            return;
        }
        text = va_arg( args, wchar_t * );
        if (text != NULL) //on linux's wcslen crashes if null
            text_buffer_add_text( self, pen, markup, text, wcslen(text) );
    } while( markup != 0 );
    va_end ( args );
}


// ----------------------------------------------------------------------------
//maybe should probably put this in text_buffer_t
//this gets set right before every print to the substrings xy location
static vec2 tmpOrigin;
float xMax; //how far we got on the x
    text_buffer_pinfo
text_buffer_printf_index( text_buffer_t * self, vec2 *pen, markup_t *markup, wchar_t *text  )
{
    //markup_t *markup;
    //wchar_t *text;

    text_buffer_pinfo returnMe;
    returnMe.startIdx    = -1;  //indicates an error
    returnMe.numGlyphs   =  0;
    returnMe.lastGlyph   =  NULL;
    returnMe.width       = 0.0;
    returnMe.height      = 0.0;

    if( vertex_buffer_size( self->buffer ) == 0 )
    {
        self->origin = *pen;
    }

    if( markup == NULL )
    {
        return returnMe;
    }

    //index before we print..
    returnMe.startIdx = (int)vertex_buffer_size( self->buffer );

    if (text != NULL) //on linux's(whatev libc this may be) wcslen crashes if null
    {
        int numGlyphs = text_buffer_add_text( self, pen, markup, text, wcslen(text) );
        returnMe.lastGlyph = self->lastGlyph;
        if (numGlyphs > 0)
            returnMe.numGlyphs = numGlyphs; //this many glyphs were printed
        else if (numGlyphs < 0)
        {
            returnMe.numGlyphs = -numGlyphs; //flip to positive
            //this means any text containers should be marked as "full"
            //and reject any further input that may append text
        }
        else
        {
            //printf("=======PRINT FAILED=========");
            returnMe.startIdx = -1; //it failed
        }
    }

    returnMe.width  = xMax   - tmpOrigin.x;
    returnMe.height = pen->y - tmpOrigin.y;
    //yep capped at INT_MAX ohno!
    return returnMe;
}


// ----------------------------------------------------------------------------
    void
text_buffer_move_last_line( text_buffer_t * self, float dy )
{
    size_t i, j;
    for( i=self->line_start; i < vector_size( self->buffer->items ); ++i )
    {
        ivec4 *item = (ivec4 *) vector_get( self->buffer->items, i);
        for( j=item->vstart; j<item->vstart+item->vcount; ++j)
        {
            glyph_vertex_t * vertex =
                (glyph_vertex_t *)  vector_get( self->buffer->vertices, j );
            vertex->y -= dy;
        }
    }
}


float text_buffer_get_font_height( text_buffer_t *self, markup_t *markup)
{
    if (!markup)
    {
        //printf("no markup: text_buffer_get_font_height");
        return 0;
    }

    font_manager_t * manager = self->manager;

    if( !markup->font )
    {
        markup->font = font_manager_get_from_markup( manager, markup );
        if (!markup->font)
            return 0;
    }

    return markup->font->height;
    
}


//helper prints, to split funcitonality for text constraints


//prints single line unconstrained
int text_buffer_print_xaxis(text_buffer_t * self,
        vec2 * pen, markup_t * markup,
        wchar_t * text, size_t length)
{
    int i = 0;
    //move text down (draws from bottom left, so get it into top left space
    pen->y += self->line_descender + markup->font->descender + markup->font->height;
    self->line_descender = 0;
    self->line_ascender = 0;
    self->line_start = vector_size( self->buffer->items );
    self->lastGlyph = 0;

    //always draw at least 1 char regardless of dimensions
    text_buffer_add_wchar( self, pen, markup, text[0],  0);
    for( i=1; i<length; ++i )
    {
        text_buffer_add_wchar( self, pen, markup, text[i], text[i-1] );
        //if (text_buffer_add_wchar( self, pen, markup, text[i], text[i-1] ) < 0)
        //  an error happened if ^^...
    }
    return i;
}


/* word wrap
 * while printing check for x overflow, if overflowing
 * we will need to rewind and insert a newline where the overflow
 * happened. we will have to recalculate any newlines after the
 * overflow index, uses \r for word wrap returns, \n's in the
 * source text will always remain untouched. and any \r's should
 * be converted back into spaces and re-wrapped.
 *
 */  

//if true, any \r's should be converted back to spaces
static int rewrap = 0;
//print downward infinitely wrapping at dimensions.x
int text_buffer_print_xyaxis(text_buffer_t * self,
        vec2 * pen, markup_t * markup,
        wchar_t * text, size_t length)
{
    rewrap  = 0;
    int i = 0;    
    texture_font_t *font = markup->font;

    //move text down (draws from bottom left, so get it into top left space
    pen->y += self->line_descender + markup->font->descender + markup->font->height;
    self->line_descender = 0;
    self->line_ascender = 0;
    self->line_start = vector_size( self->buffer->items );
    self->lastGlyph = 0;

    //always draw at least 1 char regardless of dimensions
    text_buffer_add_wchar( self, pen, markup, text[0],  0);
    for( i=1; i<length; ++i )
    {
        //check dimensions, if at end write a newline
        if (self->lastGlyph != 0)
        {   
            if (rewrap == 1)
            {
                printf("----------- console spam, verify i work right. -----------\nlocation: text_buffer_print_xyaxis()\n----------------------------------------------------------\n\n");
                if (text[i] == L'\r')
                    text[i] = L' ';//convert back to a space. 
            }            
            
            if (pen->x + self->lastGlyph->width > tmpOrigin.x + self->dimensions.x)
            {
                //find the first space.
                int rewind = 0;
                while (text[i-rewind] != L' ')
                {
                    //wrap on this letter if no spaces.
                    if (rewind == i || rewind > vertex_buffer_size(self->buffer))  
                    {
                        printf("breaking contiguous string");
                        rewind = 0;
                        break;
                    }
                    rewind += 1;
                }

                printf("rewind: %i\n", rewind);
                //erase vertex_buffer glyphs, dont use i (spaces have no glyph)
                int startIdx = vertex_buffer_size(self->buffer) - rewind;
                int z;
                for (z = 0; z < rewind; z+=1)
                {
                    printf("rewinding(%i) size: %i\n", startIdx + z,
                            vertex_buffer_size(self->buffer));
                    vertex_buffer_erase( self->buffer, startIdx );
                }
                i -= rewind;     //rewind counter 
                text[i] = L'\r'; //set input text
                text_buffer_add_wchar( self, pen, markup, L'\r', text[i-1] );
                rewrap = 1; //new wordwrap, everything after here should be re-wrapped
                ++i;
                //go back to the first space
                
            }
        }
        //try to add the char
        if (text_buffer_add_wchar( self, pen, markup, text[i], text[i-1]) < 0)
        {
            printf("----------- FAILED TO ADD GLYPH -----------");            
            break;
        }
    }
    return i;
}

//prints untill it reaches constraint dimensions
int text_buffer_print_xy_constrained(text_buffer_t * self,
        vec2 * pen, markup_t * markup,
        wchar_t * text, size_t length)
{
    int i = 0;    
    texture_font_t *font = markup->font;

    //move text down (draws from bottom left, so get it into top left space
    pen->y += self->line_descender + markup->font->descender + markup->font->height;
    self->line_descender = 0;
    self->line_ascender = 0;
    self->line_start = vector_size( self->buffer->items );
    self->lastGlyph = 0;

    //always draw at least 1 char regardless of dimensions
    text_buffer_add_wchar( self, pen, markup, text[0],  0);
    for( i=1; i<length; ++i )
    {
        //check dimensions, if at end write a newline
        if (self->lastGlyph != 0)
        {                                               
            if (pen->x + self->lastGlyph->width > tmpOrigin.x + self->dimensions.x)
            {
                if (pen->y + font->height >= tmpOrigin.y + self->dimensions.y)
                {
                    //cannot add a new line
                    break; //truncate
                }
                else //add newline
                {
                    text_buffer_add_wchar( self, pen, markup, L'\n', text[i-1] );
                }
            }

            //check y bounds (bottom left is (0, 1)
            if (pen->y - self->lastGlyph->height < 
                    tmpOrigin.y + self->lastGlyph->height - self->dimensions.y )
            {
                //text_buffer_add_wchar( self, pen, markup, L'\0', text[i-1] );
                break; //truncate
            }
        }

        //try to add the char
        if (text_buffer_add_wchar( self, pen, markup, text[i], text[i-1]) < 0)
        {
            printf("----------- FAILED TO ADD GLYPH -----------");
            break;
        }
    }

    return i;
}


// ----------------------------------------------------------------------------
    int
text_buffer_add_text( text_buffer_t * self,
        vec2 * pen, markup_t * markup,
        wchar_t * text, size_t length )
{
    font_manager_t * manager = self->manager;
    if( markup == NULL )
    {
        return 0;
    }

    if( !markup->font )
    {
        markup->font = font_manager_get_from_markup( manager, markup );
        if( ! markup->font )
        {
            fprintf( stderr, "Houston, we've got a problem !\n" );
            exit( EXIT_FAILURE );
        }
    }

    texture_font_t *font = markup->font;

    if( length == 0 )
    {
        length = wcslen(text);
        if (length == 0)
            return 0; 
    }
    if( vertex_buffer_size( self->buffer ) == 0 )
    {
        self->origin = *pen;
    }

    
    int i = 0;
    xMax = -100000.0; // start out low ;)
    //for newline return, remember where we started
    //this buffer contains multiple indexed text outs with unique offsets.
    tmpOrigin = *pen;

    //print with full constraint
    if (self->dimensions.x > 0.0  &&  self->dimensions.y > 0.0)
    {
        i = text_buffer_print_xy_constrained(self, pen, markup, text, length );
    } //print and scroll down when dimension.x is reached
    else if (self->dimensions.x > 0.0)
    {
        i = text_buffer_print_xyaxis(self, pen, markup, text, length );
    } //print forever on the x
    else
    {
        i = text_buffer_print_xaxis(self, pen, markup, text, length );
    }
    //doubt i would need a print y axis?
    
    // --- NOTE! ---    
    //reset bounds so other text is not affected.
    //bounds should be set on a per print basis
    self->dimensions.x = 0;
    self->dimensions.y = 0;

    return i;
}

// ----------------------------------------------------------------------------
    int
text_buffer_add_wchar( text_buffer_t * self,
        vec2 * pen, markup_t * markup,
        wchar_t current, wchar_t previous )
{
    size_t vcount = 0;
    size_t icount = 0;
    vertex_buffer_t * buffer = self->buffer;
    texture_font_t * font = markup->font;
    float gamma = markup->gamma;

    // Maximum number of vertices is 20 (= 5x2 triangles) per glyph:
    //  - 2 triangles for background
    //  - 2 triangles for overline
    //  - 2 triangles for underline
    //  - 2 triangles for strikethrough
    //  - 2 triangles for glyph
    glyph_vertex_t vertices[4*5];
    GLuint indices[6*5];
    texture_glyph_t *glyph;
    texture_glyph_t *black;
    float kerning = 0.0f;

    if( current == L'\n' || current == L'\r' )
    {
        pen->x  = tmpOrigin.x;//self->origin.x;
        pen->y += 1.0 + self->line_descender + markup->font->descender
                + markup->font->height;
        self->line_descender = 0;
        self->line_ascender = 0;
        self->line_start = vector_size( self->buffer->items );
        //self->lastGlyph = 0;
        return 0; //pretend it added something
    }

    //NOTE default behavior starts out ascended?
    /* if( markup->font->ascender > self->line_ascender )
       {
       float y = pen->y;
       pen->y += (markup->font->ascender - self->line_ascender);
       text_buffer_move_last_line( self, (int)(y-pen->y) );
       self->line_ascender = markup->font->ascender;
       }
       if( markup->font->descender < self->line_descender )
       {
    // self->line_descender = markup->font->descender;//markup->font->height * -1.0;//
    }*/

    glyph = texture_font_get_glyph( font, current );
    //black = texture_font_get_glyph( font, -1 );
    if( glyph == NULL )
    {
        return -1;
    }

    self->lastGlyph = glyph;

    if( previous && markup->font->kerning )
    {
        kerning = texture_glyph_get_kerning( glyph, previous );
    }
    pen->x += kerning;

    /* Background
       if( 0 && markup->background_color.alpha > 0 )
       {
       float r = markup->background_color.r;
       float g = markup->background_color.g;
       float b = markup->background_color.b;
       float a = markup->background_color.a;
       float x0 = ( pen->x -kerning );
       float y0 = (int)( pen->y + font->descender );
       float x1 = ( x0 + glyph->advance_x );
       float y1 = (int)( y0 + font->height + font->linegap );
       float s0 = black->s0;
       float t0 = black->t0;
       float s1 = black->s1;
       float t1 = black->t1;

       SET_GLYPH_VERTEX(vertices[vcount+0],
       (int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+1],
       (int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+2],
       (int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+3],
       (int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
       indices[icount + 0] = vcount+0;
       indices[icount + 1] = vcount+1;
       indices[icount + 2] = vcount+2;
       indices[icount + 3] = vcount+0;
       indices[icount + 4] = vcount+2;
       indices[icount + 5] = vcount+3;
       vcount += 4;
       icount += 6;
       }*/

    /* Underline
       if( markup->underline )
       {
       float r = markup->underline_color.r;
       float g = markup->underline_color.g;
       float b = markup->underline_color.b;
       float a = markup->underline_color.a;
       float x0 = ( pen->x - kerning );
       float y0 = (int)( pen->y + font->underline_position );
       float x1 = ( x0 + glyph->advance_x );
       float y1 = (int)( y0 + font->underline_thickness ); 
       float s0 = black->s0;
       float t0 = black->t0;
       float s1 = black->s1;
       float t1 = black->t1;

       SET_GLYPH_VERTEX(vertices[vcount+0],
       (int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+1],
       (int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+2],
       (int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+3],
       (int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
       indices[icount + 0] = vcount+0;
       indices[icount + 1] = vcount+1;
       indices[icount + 2] = vcount+2;
       indices[icount + 3] = vcount+0;
       indices[icount + 4] = vcount+2;
       indices[icount + 5] = vcount+3;
       vcount += 4;
       icount += 6;
       }*/

    /* Overline
       if( markup->overline )
       {
       float r = markup->overline_color.r;
       float g = markup->overline_color.g;
       float b = markup->overline_color.b;
       float a = markup->overline_color.a;
       float x0 = ( pen->x -kerning );
       float y0 = (int)( pen->y + (int)font->ascender );
       float x1 = ( x0 + glyph->advance_x );
       float y1 = (int)( y0 + (int)font->underline_thickness ); 
       float s0 = black->s0;
       float t0 = black->t0;
       float s1 = black->s1;
       float t1 = black->t1;
       SET_GLYPH_VERTEX(vertices[vcount+0],
       (int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+1],
       (int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+2],
       (int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+3],
       (int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
       indices[icount + 0] = vcount+0;
       indices[icount + 1] = vcount+1;
       indices[icount + 2] = vcount+2;
       indices[icount + 3] = vcount+0;
       indices[icount + 4] = vcount+2;
       indices[icount + 5] = vcount+3;
       vcount += 4;
       icount += 6;
       }*/
    /* Strikethrough 
       if( markup->strikethrough )
       {
       float r = markup->strikethrough_color.r;
       float g = markup->strikethrough_color.g;
       float b = markup->strikethrough_color.b;
       float a = markup->strikethrough_color.a;
       float x0  = ( pen->x -kerning );
       float y0  = (int)( pen->y + (int)font->ascender*.33);
       float x1  = ( x0 + glyph->advance_x );
       float y1  = (int)( y0 + (int)font->underline_thickness ); 
       float s0 = black->s0;
       float t0 = black->t0;
       float s1 = black->s1;
       float t1 = black->t1;
       SET_GLYPH_VERTEX(vertices[vcount+0],
       (int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+1],
       (int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+2],
       (int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
       SET_GLYPH_VERTEX(vertices[vcount+3],
       (int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
       indices[icount + 0] = vcount+0;
       indices[icount + 1] = vcount+1;
       indices[icount + 2] = vcount+2;
       indices[icount + 3] = vcount+0;
       indices[icount + 4] = vcount+2;
       indices[icount + 5] = vcount+3;
       vcount += 4;
       icount += 6;
       }*/


    // Actual glyph
    float r = markup->foreground_color.r;
    float g = markup->foreground_color.g;
    float b = markup->foreground_color.b;
    float a = markup->foreground_color.a;
    float x0 = ( pen->x + glyph->offset_x  );


    //printf("text color: <%f,%f,%f,%f> \n", r,g,b,a); 
    //NOTE all of these (float)'s were (int) for some reason causing glyphs to pop
    //im sure there is a reason, but the popping/jitter was bothering me

    //this y0 is translated into upper left oriented (freetype uses bottom left)
    float y0 = (int)( gWinHeight - pen->y + glyph->offset_y );
    float x1 = ( x0 + glyph->width  );
    float y1 = (int)( y0 - glyph->height );
    float s0 = glyph->s0;
    float t0 = glyph->t0;
    float s1 = glyph->s1;
    float t1 = glyph->t1;

    SET_GLYPH_VERTEX(vertices[vcount+0],
            (int)x0,y0,0,  s0,t0,  r,g,b,a);
    SET_GLYPH_VERTEX(vertices[vcount+1],
            (int)x0,y1,0,  s0,t1,  r,g,b,a);
    SET_GLYPH_VERTEX(vertices[vcount+2],
            (int)x1,y1,0,  s1,t1,  r,g,b,a);
    SET_GLYPH_VERTEX(vertices[vcount+3],
            (int)x1,y0,0,  s1,t0,  r,g,b,a);

    //old set with shift + gamma
    /*SET_GLYPH_VERTEX(vertices[vcount+0],
      (int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
      SET_GLYPH_VERTEX(vertices[vcount+1],
      (int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
      SET_GLYPH_VERTEX(vertices[vcount+2],
      (int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
      SET_GLYPH_VERTEX(vertices[vcount+3],
      (int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
      */

    //i reversed winding        +x  < here
    indices[icount + 0] = vcount+2;
    indices[icount + 1] = vcount+1;
    indices[icount + 2] = vcount+0;

    indices[icount + 3] = vcount+3;
    indices[icount + 4] = vcount+2;
    indices[icount + 5] = vcount+0;
    vcount += 4;
    icount += 6;

    vertex_buffer_push_back( buffer, vertices, vcount, indices, icount );
    
    if (pen->x > xMax)
        xMax = pen->x;
    //printed. advance the pen position
    pen->x += glyph->advance_x * (1.0 + markup->spacing);
    
    return 0;
}

texture_glyph_t *text_buffer_get_glyph(text_buffer_t *self, 
        markup_t *markup, 
        wchar_t symbol)
{
    if (!markup)
        return NULL;

    font_manager_t * manager = self->manager;

    if( !markup->font )
    {
        markup->font = font_manager_get_from_markup( manager, markup );
        if (!markup->font)
            return NULL;
    }

    texture_glyph_t *glyph = texture_font_get_glyph( markup->font, symbol );
    return glyph;

}

