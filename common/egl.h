#ifndef __egl_ctx_h__
#define __egl_ctx_h__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <screen/screen.h>

struct egl_image {
	screen_pixmap_t pix;
	EGLImageKHR img;
	int rgba_sampling;

	int size[2];
	int stride;
	int fmt;
};

typedef struct egl_ctx_s {
	/* params */
	EGLNativeDisplayType disp_id;
	struct {
		EGLint red_size;
		EGLint green_size;
		EGLint blue_size;
		EGLint alpha_size;
		EGLint samples;
		EGLint config_id;
	} egl_conf_attr;

	screen_context_t screen_ctx;
	
	/* egl stuff */
	EGLDisplay egl_disp;
	EGLConfig egl_conf;
	EGLContext egl_ctx;
	/* current surface */
	EGLSurface egl_dsurf;
	EGLSurface egl_rsurf;

	/* extension funcs */
	PFNEGLCREATEIMAGEKHRPROC peglCreateImage;
	PFNEGLDESTROYIMAGEKHRPROC peglDestroyImage;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pglEGLImageTargetTexture2D;
	int img2tex;
}egl_ctx_t;

void egl_default(egl_ctx_t * egl_ctx);
int  egl_init(egl_ctx_t * egl_ctx, screen_context_t screen_ctx);
void egl_fini(egl_ctx_t * ctx);
int  egl_make_current(egl_ctx_t * ctx, EGLSurface dsurf, EGLSurface rsurf);
int  egl_swapbuffers(egl_ctx_t * ctx);
void egl_perror(const char *msg);

//int  egl_img_init(egl_ctx_t * egl_ctx, screen_buffer_t buf,
//				 struct egl_image * img);
int  egl_img_init(egl_ctx_t * egl_ctx, screen_pixmap_t pix,
				 struct egl_image * img);
void egl_img_fini(egl_ctx_t * egl_ctx, struct egl_image * img);


#endif
