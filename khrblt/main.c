#include "win.h"
#include "gldebug.h"

egl_ctx_t egl_ctx;
screen_context_t screen_ctx;
static win_t win;


extern int render(win_t *win);
extern int yuv_init();

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
		fprintf(stderr, "egl_init failed with error %d (0x%08x)\n", rc, rc);
		return -1;
	}
	win_default(&win);
	win.format = SCREEN_FORMAT_RGB565;
	win.render_func = render;
	
	rc = win_init(&win, screen_ctx, &egl_ctx);
	if (rc) {
		fprintf(stderr, "win_init failed with error %d (0x%08x)\n", rc, rc);
		return -1;
	}
	rc = yuv_init();
	if (rc) {
		fprintf(stderr, "yuv_init failed with error %d (0x%08x)\n", rc, rc);
		return -1;
	}
	rc = win_loop(&win);

	win_fini(&win);
	egl_fini(&egl_ctx);

	screen_destroy_context(screen_ctx);
	return 0;
}
