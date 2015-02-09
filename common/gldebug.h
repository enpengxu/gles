#ifndef __gl_debug_h__
#define __gl_debug_h__

int glDebugBufferDump(char * name,int w, int h);
int glDebugGenTextureChecker();

#define glDebugError()  \
	{																	\
		EGLint eglerr = eglGetError();									\
		GLenum glerr = glGetError();									\
		if(EGL_SUCCESS != eglerr)										\
			printf("eglGetError %d @ %s : %d \n", eglerr, __FILE__, __LINE__); \
		if(GL_NO_ERROR != glerr)										\
			printf("glGetError %d @ %s : %d \n", glerr, __FILE__, __LINE__); \
	}

#endif
