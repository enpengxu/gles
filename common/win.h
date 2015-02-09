#ifndef __gles2_window_h
#define __gles2_window_h

#include <screen/screen.h>
#include "egl.h"

typedef struct win_s {
	/* params */
	int size[2];
	int pos[2];
	int nbuffers;
	int format;
	int usage;
	int (*render_func)(struct win_s * );
	void * private;
	
	egl_ctx_t * egl_ctx;
	EGLSurface egl_surf;

	/* screen API stuff */
	screen_context_t screen_ctx;
	screen_window_t screen_win;
	screen_event_t screen_ev;
	/* render buffer */
	screen_buffer_t rbuf;
}win_t;

void win_default(win_t * win);
int win_init(win_t * win, screen_context_t screen_ctx, egl_ctx_t * egl_ctx);
int win_loop(win_t *win);
void win_fini(win_t *win);

#endif
