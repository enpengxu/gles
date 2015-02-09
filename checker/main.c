#include "win.h"
#include "gldebug.h"

egl_ctx_t egl_ctx;
screen_context_t screen_ctx;
static win_t win;

	float vb[] = {
		-0.5, -0.5, 0.0, 1.0,
		+0.5, -0.5, 0.0, 1.0,
		+0.5, +0.5, 0.0, 1.0,
		-0.5, +0.5, 0.0, 1.0,
	};
	float tc[] = {
		0.0, 0.0, 
		2.0, 0.0, 
		2.0, 2.0, 
		0.0, 2.0, 
	};

static void initbuf()
{
	
}
static void myinit(void)
{    
    glClearColor (0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
	glDebugError();

	glDefaultShaders();
	
	glActiveTexture(GL_TEXTURE0);
	int tex = glDebugGenTextureChecker();
	//glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if 1
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif	
	glDebugError();
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vb);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tc);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	glDebugError();
 }

int render(win_t *win)
{
	glClearColor(0.75f, 0, 0.75f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	glDebugError();

	//glFinish();
	return 0;
}


 int main(int argc, char **argv)
{
	int rc;
	rc = screen_create_context(&screen_ctx, 0);
	if (rc) {
		fprintf(stderr, "screen_create_context failed with error %d (0x%08x)\n", rc, rc);
		return -1;
	}

	egl_default(&egl_ctx);
	rc = egl_init(&egl_ctx, screen_ctx);
	if(rc){
		return -1;
	}
	win_default(&win);
	win.render_func = render;
	
	rc = win_init(&win, screen_ctx, &egl_ctx);
	if (rc) {
		fprintf(stderr, "win_init failed with error %d (0x%08x)\n", rc, rc);
		return -1;
	}
	myinit();
	
	rc = win_loop(&win);

	win_fini(&win);
	egl_fini(&egl_ctx);

	screen_destroy_context(screen_ctx);
	return 0;
}
