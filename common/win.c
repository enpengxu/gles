#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <time.h>

#include "win.h"

void win_default(win_t * win)
{
	memset(win, 0, sizeof(*win));
	win->size[0] = -1;
	win->size[1] = -1;
	win->nbuffers = 2;
	win->format = SCREEN_FORMAT_RGBA8888;
	win->usage = SCREEN_USAGE_OPENGL_ES2;
}

int win_init(win_t * win,
			 screen_context_t screen_ctx,
			 egl_ctx_t * egl_ctx )
{
	int rc;

	win->egl_ctx = egl_ctx;
	win->screen_ctx = screen_ctx;

	rc = screen_create_window(&win->screen_win, screen_ctx);
	if (rc) {
		perror("screen_create_window");
		win_fini(win);
		return -1;
	}

	rc = screen_set_window_property_iv(win->screen_win,
									   SCREEN_PROPERTY_FORMAT,
									   &win->format);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
		win_fini(win);
		return -1;
	}
	rc = screen_set_window_property_iv(win->screen_win,
									   SCREEN_PROPERTY_USAGE,
									   &win->usage);
	if (rc) {
		perror("screen_set_window_property_iv");
		win_fini(win);
		return -1;
	}
	if (win->size[0] > 0 && win->size[1] > 0) {
		rc = screen_set_window_property_iv(win->screen_win,
										   SCREEN_PROPERTY_SIZE,
										   win->size);
		if (rc) {
			perror("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE)");
			win_fini(win);
			return -1;
		}
	} else {
		rc = screen_get_window_property_iv(win->screen_win,
										   SCREEN_PROPERTY_SIZE,
										   win->size);
		if (rc) {
			perror("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE)");
			win_fini(win);
			return -1;
		}
	}

	if (win->pos[0] != 0 || win->pos[1] != 0) {
		rc = screen_set_window_property_iv(win->screen_win,
										   SCREEN_PROPERTY_POSITION,
										   win->pos);
		if (rc) {
			perror("screen_set_window_property_iv(SCREEN_PROPERTY_POSITION)");
			win_fini(win);
			return -1;
		}
	}
	rc = screen_create_window_buffers(win->screen_win, win->nbuffers);
	if (rc) {
		perror("screen_create_window_buffers");
		win_fini(win);
		return -1;
	}

	struct {
		EGLint render_buffer[2];
		EGLint none;
	} egl_surf_attr = {
		.render_buffer = { EGL_RENDER_BUFFER, EGL_BACK_BUFFER },
		.none = EGL_NONE
	};

	win->egl_surf = eglCreateWindowSurface(egl_ctx->egl_disp,
										   egl_ctx->egl_conf,
										   win->screen_win,
										   (EGLint*)&egl_surf_attr);

	if (win->egl_surf == EGL_NO_SURFACE) {
		egl_perror("eglCreateWindowSurface");
		win_fini(win);
		return -1;
	}

	rc = egl_make_current(egl_ctx, win->egl_surf, win->egl_surf);
	if (rc) {
		win_fini(win);
		return -1;
	}

	rc = eglSwapInterval(egl_ctx->egl_disp, 0);
	if (rc != EGL_TRUE) {
		egl_perror("eglSwapInterval");
		win_fini(win);
		return -1;
	}

	rc = screen_create_event(&win->screen_ev);
	if (rc) {
		perror("screen_create_event");
		win_fini(win);
		return -1;
	}
	return 0;
}

void win_fini(win_t * win)
{
	if(win->screen_ev){
		screen_destroy_event(win->screen_ev);
		win->screen_ev = 0;
	}

	if(egl_make_current(win->egl_ctx, EGL_NO_SURFACE, EGL_NO_SURFACE)){
	}

	if(win->egl_surf){
		eglDestroySurface(win->egl_ctx->egl_disp, win->egl_surf);
		win->egl_surf = 0;
	}
	if(win->screen_win){
		screen_destroy_window(win->screen_win);
		win->screen_win = 0;
	}
	win->egl_ctx = 0;
	win->rbuf = 0;
}

int win_loop(win_t *win)
{
	int pause = 0;
	int rc, val, pos[2];
	struct timespec to;
	uint64_t t, last_t, delta;
	int frames = 0;
	//int frame_limit_counter = -1;

	rc = egl_make_current(win->egl_ctx, win->egl_surf, win->egl_surf);
	if (rc) {
		win_fini(win);
		return -1;
	}

	while (1) {
		while (!screen_get_event(win->screen_ctx, win->screen_ev, pause ? ~0 : 0)) {
			rc = screen_get_event_property_iv(win->screen_ev, SCREEN_PROPERTY_TYPE, &val);
			if (rc || val == SCREEN_EVENT_NONE) {
				break;
			}
			switch (val) {
			case SCREEN_EVENT_CLOSE:
				return 1;
			case SCREEN_EVENT_POINTER:
				screen_get_event_property_iv(win->screen_ev, SCREEN_PROPERTY_BUTTONS, &val);
				if (val) {
					screen_get_event_property_iv(win->screen_ev, SCREEN_PROPERTY_SOURCE_POSITION, pos);
				}
				break;
			case SCREEN_EVENT_KEYBOARD:
				screen_get_event_property_iv(win->screen_ev, SCREEN_PROPERTY_KEY_FLAGS, &val);
				if (val & KEY_DOWN) {
					screen_get_event_property_iv(win->screen_ev, SCREEN_PROPERTY_KEY_SYM, &val);
					switch (val) {
					case KEYCODE_ESCAPE:
						return 1;
					case KEYCODE_F:
						pause = !pause;
						break;
					default:
						break;
					}
				}
				break;
			}
		}

		if (pause) {
			continue;
		}

		/* get current render buffer */
		{
			screen_buffer_t bufs[6];
			int rc  = screen_get_window_property_pv(win->screen_win,
													SCREEN_PROPERTY_RENDER_BUFFERS,
													(void **)bufs);
			if(rc){
				break;
			}
			win->rbuf = bufs[0];
		}

		if(win->render_func){
			if(-1 == win->render_func(win)){
				break;
			}
		}else {
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		//rc = eglSwapBuffers(win->egl_ctx->egl_disp, win->egl_surf);
		//if (rc != EGL_TRUE) {
		if(egl_swapbuffers(win->egl_ctx)){
			egl_perror("eglSwapBuffers");
			break;
		}

		clock_gettime(CLOCK_REALTIME, &to);
		t = timespec2nsec(&to);

		if (frames == 0) {
			last_t = t;
		} else {
			delta = t - last_t;
			if (delta >= 5000000000LL) {
				printf("%d frames in %6.3f seconds = %6.3f FPS\n",
					   frames, 0.000000001f * delta, 1000000000.0f * frames / delta);
				fflush(stdout);
				frames = -1;
			}
		}
		frames++;
	}
	return 0;
}
