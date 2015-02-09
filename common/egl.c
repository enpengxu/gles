#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "screen_util.h"
#include "gldebug.h"
#include "egl.h"

void
egl_perror(const char *msg)
{
    static const char *errmsg[] = {
        "function succeeded",
        "EGL is not initialized, or could not be initialized, for the specified display",
        "cannot access a requested resource",
        "failed to allocate resources for the requested operation",
        "an unrecognized attribute or attribute value was passed in an attribute list",
        "an EGLConfig argument does not name a valid EGLConfig",
        "an EGLContext argument does not name a valid EGLContext",
        "the current surface of the calling thread is no longer valid",
        "an EGLDisplay argument does not name a valid EGLDisplay",
        "arguments are inconsistent",
        "an EGLNativePixmapType argument does not refer to a valid native pixmap",
        "an EGLNativeWindowType argument does not refer to a valid native window",
        "one or more argument values are invalid",
        "an EGLSurface argument does not name a valid surface configured for rendering",
        "a power management event has occurred",
    };
    fprintf(stderr, "%s: %s\n", msg, errmsg[eglGetError() - EGL_SUCCESS]);
}

static EGLConfig
choose_config(egl_ctx_t * ctx)
{
	EGLDisplay egl_disp = ctx->egl_disp;
	EGLConfig egl_conf = (EGLConfig)0;
	EGLConfig *egl_configs;
	EGLint egl_num_configs;
	EGLint val;
	EGLBoolean rc;
	EGLint i;

	rc = eglGetConfigs(egl_disp, NULL, 0, &egl_num_configs);
	if (rc != EGL_TRUE) {
		egl_perror("eglGetConfigs");
		return egl_conf;
	}
	if (egl_num_configs == 0) {
		fprintf(stderr, "eglGetConfigs: could not find a configuration\n");
		return egl_conf;
	}

	egl_configs = malloc(egl_num_configs * sizeof(*egl_configs));
	if (egl_configs == NULL) {
		fprintf(stderr, "could not allocate memory for %d EGL configs\n", egl_num_configs);
		return egl_conf;
	}

	rc = eglGetConfigs(egl_disp, egl_configs,
					   egl_num_configs, &egl_num_configs);
	if (rc != EGL_TRUE) {
		egl_perror("eglGetConfigs");
		free(egl_configs);
		return egl_conf;
	}

	for (i = 0; i < egl_num_configs; i++) {
		if (ctx->egl_conf_attr.config_id != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_CONFIG_ID, &val);
			if (val == ctx->egl_conf_attr.config_id) {
				egl_conf = egl_configs[i];
				break;
			} else {
				continue;
			}
		}
		eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_SURFACE_TYPE, &val);
		if (!(val & EGL_WINDOW_BIT)) {
			continue;
		}

		eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_RENDERABLE_TYPE, &val);
		if (!(val & EGL_OPENGL_ES2_BIT)) {
			continue;
		}

		eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_DEPTH_SIZE, &val);
		if (val == 0) {
			continue;
		}

		if (ctx->egl_conf_attr.red_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_RED_SIZE, &val);
			if (val != ctx->egl_conf_attr.red_size) {
				continue;
			}
		}
		if (ctx->egl_conf_attr.green_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_GREEN_SIZE, &val);
			if (val != ctx->egl_conf_attr.green_size) {
				continue;
			}
		}
		if (ctx->egl_conf_attr.blue_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_BLUE_SIZE, &val);
			if (val != ctx->egl_conf_attr.blue_size) {
				continue;
			}
		}
		if (ctx->egl_conf_attr.alpha_size != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_ALPHA_SIZE, &val);
			if (val != ctx->egl_conf_attr.alpha_size) {
				continue;
			}
		}
		if (ctx->egl_conf_attr.samples != EGL_DONT_CARE) {
			eglGetConfigAttrib(egl_disp, egl_configs[i], EGL_SAMPLES, &val);
			if (val != ctx->egl_conf_attr.samples) {
				continue;
			}
		}

		egl_conf = egl_configs[i];
		break;
	}

	free(egl_configs);

	if (egl_conf == (EGLConfig)0) {
		fprintf(stderr, "eglChooseConfig: could not find a matching configuration\n");
	}
	return egl_conf;
}

void egl_default(egl_ctx_t * egl_ctx)
{
	memset(egl_ctx, 0, sizeof(*egl_ctx));
	egl_ctx->disp_id = EGL_DEFAULT_DISPLAY;
#if 1
	egl_ctx->egl_conf_attr.red_size = 5;
	egl_ctx->egl_conf_attr.green_size = 6;
	egl_ctx->egl_conf_attr.blue_size = 5;
	egl_ctx->egl_conf_attr.alpha_size = 0;
#else
	egl_ctx->egl_conf_attr.red_size = 8;
	egl_ctx->egl_conf_attr.green_size = 8;
	egl_ctx->egl_conf_attr.blue_size = 8;
	egl_ctx->egl_conf_attr.alpha_size = 8;
#endif
	egl_ctx->egl_conf_attr.samples = EGL_DONT_CARE;
	egl_ctx->egl_conf_attr.config_id = EGL_DONT_CARE;
}

int egl_init(egl_ctx_t * egl_ctx, screen_context_t screen_ctx)
{
	int rc;
	egl_ctx->egl_disp = 0;
	egl_ctx->egl_conf = 0;
	egl_ctx->egl_ctx = 0;
	egl_ctx->egl_rsurf = 0;
	egl_ctx->egl_dsurf = 0;
	egl_ctx->screen_ctx = screen_ctx;
	egl_ctx->egl_disp = eglGetDisplay(egl_ctx->disp_id);
	if (egl_ctx->egl_disp == EGL_NO_DISPLAY) {
		egl_perror("eglGetDisplay");
		return -1;
	}
	
	rc = eglInitialize(egl_ctx->egl_disp, NULL, NULL);
	if (rc != EGL_TRUE) {
		egl_perror("eglInitialize");
		egl_fini(egl_ctx);
		return -1;
	}

	egl_ctx->egl_conf = choose_config(egl_ctx);
	if (egl_ctx->egl_conf == (EGLConfig)0) {
		egl_perror("choose_config");
		egl_fini(egl_ctx);
		return -1;
	}

	struct {
		EGLint client_version[2];
		EGLint none;
	} egl_ctx_attr = {
		.client_version = { EGL_CONTEXT_CLIENT_VERSION, 2 },
		.none = EGL_NONE
	};

	egl_ctx->egl_ctx = eglCreateContext(egl_ctx->egl_disp,
										egl_ctx->egl_conf,
										EGL_NO_CONTEXT,
										(EGLint*)&egl_ctx_attr);
	if (egl_ctx->egl_ctx == EGL_NO_CONTEXT) {
		egl_perror("eglCreateContext");
		egl_fini(egl_ctx);
		return -1;
	}
	return 0;
}
void egl_fini(egl_ctx_t * ctx)
{
	if(ctx){
		if(ctx->egl_disp)
			eglMakeCurrent(ctx->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if(ctx->egl_ctx)
			eglDestroyContext(ctx->egl_disp, ctx->egl_ctx);
		ctx->egl_disp = 0;
		ctx->egl_conf = 0;
		ctx->egl_ctx = 0;
		ctx->egl_rsurf = EGL_NO_SURFACE;
		ctx->egl_dsurf = EGL_NO_SURFACE;
	}
}

int egl_make_current(egl_ctx_t * ctx, EGLSurface dsurf, EGLSurface rsurf)
{
	int rc;
	if(dsurf == EGL_NO_SURFACE &&
	   rsurf == EGL_NO_SURFACE ){
		rc = eglMakeCurrent(ctx->egl_disp,
							EGL_NO_SURFACE,
							EGL_NO_SURFACE,
							EGL_NO_CONTEXT);
	}else {
		rc = eglMakeCurrent(ctx->egl_disp, dsurf, rsurf, ctx->egl_ctx);
	}
	if (rc != EGL_TRUE) {
		egl_perror("eglMakeCurrent");
		return -1;
	}
	ctx->egl_dsurf = dsurf;
	ctx->egl_rsurf = rsurf;

	if(!ctx->peglCreateImage){
		/*TODO. need to check extension string */
		ctx->peglCreateImage = (PFNEGLCREATEIMAGEKHRPROC)
			eglGetProcAddress("eglCreateImageKHR");
		ctx->peglDestroyImage = (PFNEGLDESTROYIMAGEKHRPROC)
			eglGetProcAddress("eglDestroyImageKHR");
		ctx->pglEGLImageTargetTexture2D = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
			eglGetProcAddress("glEGLImageTargetTexture2DOES");

		ctx->img2tex = 1;
		if( !ctx->peglCreateImage ||
			!ctx->peglDestroyImage ||
			!ctx->pglEGLImageTargetTexture2D){
			ctx->img2tex = 0;
		}
	}
	return 0;
}
int egl_swapbuffers(egl_ctx_t * ctx)
{
	assert(ctx->egl_dsurf);
	int rc = eglSwapBuffers(ctx->egl_disp, ctx->egl_dsurf);
	if (rc != EGL_TRUE) {
		egl_perror("eglSwapBuffers");
		return -1;
	}
	return 0;
}


int egl_img_init(egl_ctx_t * egl_ctx, screen_pixmap_t pix,
				 struct egl_image * img)
{
	if(!egl_ctx->img2tex){
		return -1;
	}

	img->pix = pix;
	if(!img->pix){
		return -1;
	}
	screen_buffer_t buf = pixmap_buffer(pix);
	buffer_lock(buf,
				img->size,
				&img->stride,
				&img->fmt);
	buffer_unlock(buf);

	img->img = egl_ctx->peglCreateImage(egl_ctx->egl_disp,
									EGL_NO_CONTEXT,
									EGL_NATIVE_PIXMAP_KHR,
									(EGLClientBuffer)img->pix,
									(const EGLint *)NULL);
	if(img->img){
		img->rgba_sampling = 1;
		return 0;
	}
	return -1;
}

void egl_img_fini(egl_ctx_t * egl_ctx, struct egl_image * img)
{
	if(img && img->pix){
		pixmap_destroy(img->pix);
		img->pix = 0;
		if(img->img){
			egl_ctx->peglDestroyImage(egl_ctx->egl_disp, img->img);
			img->img = 0;
		}
		if(img->pix){
			screen_destroy_pixmap(img->pix);
			img->pix = 0;
		}
	}
}
