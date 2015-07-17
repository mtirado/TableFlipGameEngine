/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 


#ifndef __GL_UTIL_H__
#define __GL_UTIL_H__
#include "Utilities.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
/*#if ESSENTIAL_GL_PRACTICES_IOS

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#else

#import <OpenGL/OpenGL.h>

// OpenGL 3.2 is only supported on MacOS X Lion and later
// CGL_VERSION_1_3 is defined as 1 on MacOS X Lion and later
#if CGL_VERSION_1_3
// Set to 0 to run on the Legacy OpenGL Profile
#define ESSENTIAL_GL_PRACTICES_SUPPORT_GL3 1
#else
#define ESSENTIAL_GL_PRACTICES_SUPPORT_GL3 0
#endif //!CGL_VERSION_1_3

#if ESSENTIAL_GL_PRACTICES_SUPPORT_GL3
#import <OpenGL/gl3.h>
#else
#import <OpenGL/gl.h>
#endif //!ESSENTIAL_GL_PRACTICES_SUPPORT_GL3

#endif // !ESSENTIAL_GL_PRACTICES_IOS


//The name of the VertexArrayObject are slightly different in
// OpenGLES, OpenGL Core Profile, and OpenGL Legacy
// The arguments are exactly the same across these APIs however
#if ESSENTIAL_GL_PRACTICES_IOS
#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#else
#if ESSENTIAL_GL_PRACTICES_SUPPORT_GL3
#define glBindVertexArray glBindVertexArray
#define glGenVertexArrays glGenVertexArrays
#define glGenerateMipmap glGenerateMipmap
#define glDeleteVertexArrays glDeleteVertexArrays
#else
#define glBindVertexArray glBindVertexArrayAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glGenerateMipmap glGenerateMipmapEXT
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif //!ESSENTIAL_GL_PRACTICES_SUPPORT_GL3
#endif //!ESSENTIAL_GL_PRACTICES_IOS
*/
static inline const char * GetGLErrorString(GLenum error)
{
	const char *str;
	switch( error )
	{
		case GL_NO_ERROR:
			str = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			str = "GL_INVALID_OPERATION";
			break;
#if defined __gl_h_ || defined __gl3_h_
		case GL_OUT_OF_MEMORY:
			str = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			str = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
#endif
#if defined __gl_h_
		case GL_STACK_OVERFLOW:
			str = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			str = "GL_STACK_UNDERFLOW";
			break;
		case GL_TABLE_TOO_LARGE:
			str = "GL_TABLE_TOO_LARGE";
			break;
#endif
		default:
			str = "(ERROR: Unknown Error Enum)";
			break;
	}
	return str;
}


#endif // __GL_UTIL_H__

