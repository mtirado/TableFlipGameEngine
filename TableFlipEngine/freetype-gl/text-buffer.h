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




#ifndef __TEXT_BUFFER_H__
#define __TEXT_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vertex-buffer.h"
#include "font-manager.h"
#include "markup.h"
#include "shader.h"


/**
 * Use LCD filtering
 */
#define LCD_FILTERING_ON    3

/**
 * Do not use LCD filtering
 */
#define LCD_FILTERING_OFF 1

/**
 * @file   text-buffer.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup text-buffer Text buffer
 *
 *
 * <b>Example Usage</b>:
 * @code
 * #include "shader.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *
 *     return 0;
 * }
 * @endcode
 *
 * @{
 */


/**modifications:

text_buffer_pinfo text_buffer_printf_index()  -- glyph selection helper
added a version of text_buffer_printf that returns a struct holding information to 
identify substring boundaries in the single window text buffer start index in VBO
and the end index, identifying where a string has been truncated by bounds check
this way i can jump directly to position data, assuming it starts at index 0
that i am interested in retrieving and finding the offset of
see text_buffer_pinfo struct

sets screen window dimensions for converting bottom left to top left coordinates
TODO needs a solution that will allow multiple screens + resolutions

added a means to draw within a rectangle, glyph *lastGlyph & vec2 dimensions 
set before calling printf,
anything that cannot be pushed down a line is truncated
if dimensions are 0, single line of text, goes on forever.

split the print functions into varying constraint patterns
horizontal infinity, vertical w/ x dimension constraint (scrolls down)
xy constrained (will return upon reaching dimensions.xy

i use a y drawing offset, so i can translate to top left orientation
1000 - y  -- the freetype glyphs are bottom left oriented

added a few helper functions at the bottom 

\r is used as a word wrap specific newline, converts spaces to \r
on newlines, so you must strip \r's from the source string later

*/

typedef struct  text_buffer_pinfo {

    //index this printf call started printing at
    int startIdx;
    //how many successfully printed.
    int numGlyphs;
    //last glyph drawn
    texture_glyph_t *lastGlyph;

    //the assumed print rectangle
    float width;
    float height;

} text_buffer_pinfo;



/**
 * Text buffer structure
 */
typedef struct  text_buffer_t {
    /**
     * Vertex buffer 
     */
    vertex_buffer_t *buffer;

    /**
     * Font manager 
     */
    font_manager_t *manager;

    /**
     * Base color for text
     */
    vec4 base_color;

    /**
     * Pen origin
     */
    vec2 origin;

    //pen bounds, set before print, print will reset to 0
    vec2 dimensions;

    //last glyph drawn
    texture_glyph_t *lastGlyph;
    
    /**
     * Index (in the vertex buffer) of the line start
     */
    size_t line_start;

    /**
     * Current line ascender
     */
    float line_ascender;

    /**
     * Current line decender
     */
    float line_descender;

    /**
     * Shader handler
     */
    GLuint shader;

    /**
     * Shader "texture" location
     */
    GLuint shader_texture;

    /**
     * Shader "pixel" location
     */
    GLuint shader_pixel;

} text_buffer_t;

/**
 * Glyph vertex structure
 */
typedef struct glyph_vertex_t {
    /**
     * Vertex x coordinates
     */
    float x;

    /**
     * Vertex y coordinates
     */
    float y;

    /**
     * Vertex z coordinates
     */
    float z;

    /**
     * Texture first coordinate
     */
    float u;

    /**
     * Texture second coordinate
     */
    float v;

    /**
     * Color red component
     */
    float r;

    /**
     * Color green component
     */
    float g;

    /**
     * Color blue component
     */
    float b;

    /**
     * Color alpha component
     */
    float a;

} glyph_vertex_t;


//sets the global screen resolution 
void text_buffer_set_screensize(unsigned int width, unsigned int height);

/**
 * Creates a new empty text buffer.
 *
 * @param depth  Underlying atlas bit depth (1 or 3)
 *
 * @return  a new empty text buffer.
 *
 */
  text_buffer_t *
  text_buffer_new( size_t depth );

  /**
  * Deletes texture buffer and its associated shader and vertex buffer.
  *
  * @param  self  texture buffer to delete
  *
  */
void
text_buffer_delete( text_buffer_t * self );

/**
 * Render a text buffer.
 *
 * @param self a text buffer
 *
 */
  void
  text_buffer_render( text_buffer_t * self );


 /**
  * Print some text to the text buffer
  *
  * @param self a text buffer
  * @param pen  position of text start
  * @param ...  a series of markup_t *, wchar_t * ended by NULL
  *
  */
  void
  text_buffer_printf( text_buffer_t * self, vec2 * pen, ... );

  /**  modified from original source
  * Print some text to the text buffer
  * only handle one print at a time to avoid grouping
  * many prints with one index
  *
  * @param self a text buffer
  * @param pen  position of text start
  * @param ...  a series of markup_t *, wchar_t * ended by NULL
  *
  * @return starting VBO index ( idx < 0 is invalid).
  */
  text_buffer_pinfo 
  text_buffer_printf_index( text_buffer_t * self, vec2 * pen, 
                            markup_t *markup, wchar_t *text  );


 /**   MODIFIED to return int
  * Add some text to the text buffer
  *
  * @param self   a text buffer
  * @param pen    position of text start
  * @param markup Markup to be used to add text
  * @param text   Text to be added
  * @param length Length of text to be added
  *
  * @return number of glyphs drawn 
  */
  int 
  text_buffer_add_text( text_buffer_t * self,
                        vec2 * pen, markup_t * markup,
                        wchar_t * text, size_t length );

 /**
  * Add a char to the text buffer
  *
  * @param self     a text buffer
  * @param pen      position of text start
  * @param markup   markup to be used to add text
  * @param current  charactr to be added
  * @param previous previous character (if any)
  *
  * @return negative for error 
  */
  int
  text_buffer_add_wchar( text_buffer_t * self,
                         vec2 * pen, markup_t * markup,
                         wchar_t current, wchar_t previous );

 /**
  * Clear text buffer
  *
  * @param self a text buffer
  */
    void
    text_buffer_clear( text_buffer_t * self );



    //added helpers

    /**
    * returns font height for given markup
    */
    float text_buffer_get_font_height( text_buffer_t *self,
                                       markup_t *markup );


    /**
     * gets a glyph for font style
     */
    texture_glyph_t *text_buffer_get_glyph(text_buffer_t *self,
                                           markup_t *markup,
                                           wchar_t symbol);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* #define __TEXT_BUFFER_H__ */
