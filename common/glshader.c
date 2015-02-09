#include "glshader.h"
#include <stdarg.h>



static const char * vs_vt = 
	"attribute vec4 vertex;\n"
	"attribute vec2 tcoord0;\n"
	"varying mediump vec2 tcoord;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = vec4(vertex.xy, 0.0, 1.0);\n"
	"    tcoord = tcoord0.xy;\n"
	"}";

static const char * fs_vt = 
	"uniform sampler2D tex;\n"
	"varying mediump vec2 tcoord;\n"
	"void main(void)\n"
	"{\n"
	"    gl_FragColor = vec4(tcoord, 0, 1.0);\n"
	"    gl_FragColor = texture2D(tex, tcoord);\n"
	"}";


int glCreatePgm(const char *vsrc, const char * fsrc,
				const char *vatt, ... )
{
	int rc;
	GLuint vshdr = glCreateShader(GL_VERTEX_SHADER);
	if (!vshdr) {
		egl_perror("glCreateShader");
		return -1;
	}
	glShaderSource(vshdr, 1, &vsrc, 0);
	glCompileShader(vshdr);
	glGetShaderiv(vshdr, GL_COMPILE_STATUS, &rc);
	if (rc == GL_FALSE) {
		return -1;
	}
	GLuint fshdr = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fshdr) {
		egl_perror("glCreateShader");
		glDeleteShader(vshdr);
		return -1;
	}
	
	glShaderSource(fshdr, 1, &fsrc, 0);
	glCompileShader(fshdr);
	glGetShaderiv(fshdr, GL_COMPILE_STATUS, &rc);
	if (rc == GL_FALSE) {
		glDeleteShader(vshdr);
		glDeleteShader(fshdr);
		return -1;
	}
	
	GLuint pgm = glCreateProgram();
	if (!pgm) {
		egl_perror("glCreateProgram");
		glDeleteShader(vshdr);
		glDeleteShader(fshdr);
		return -1;
	}
	glAttachShader(pgm, vshdr);
	glAttachShader(pgm, fshdr);

	va_list vl;
	va_start(vl, vatt);
	const char * att = vatt;
	int idx = 0;
	while(att){
		glBindAttribLocation(pgm, idx++, att);
		att = va_arg(vl, const char *);
	}
	va_end(vl);
	glLinkProgram(pgm);

	glGetProgramiv(pgm, GL_LINK_STATUS, &rc);
	if (rc == GL_FALSE) {
		glDeleteShader(vshdr);
		glDeleteShader(fshdr);
		return -1;
	}
	return pgm;
}

int glDefaultShaders()
{
	int pgm = glCreatePgm(vs_vt, fs_vt, "vertex", "tcoord0", 0);
	int tex  = glGetUniformLocation(pgm, "tex");

	glUseProgram(pgm);
	glUniform1i(tex, 0);
	return pgm;
}
