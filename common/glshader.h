#ifndef __gl_draw__h__
#define __gl_draw__h__

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

//enum {
//	GL_POINTS =0,
//	GL_QUADS,
//};

void glBegin(int mode);
void glVertex3f(float x, float y, float z);
void glTexCoord2f(float x, float y);
void glEnd();


/* attribute 0: vertex
   attribute 1: tcoord0
   .... 
   TODO. add more parameters to do it.
*/
int glCreatePgm(const char *vsrc, const char * fsrc,
				const char * vtt, ...);
int glDefaultShaders();


#endif
