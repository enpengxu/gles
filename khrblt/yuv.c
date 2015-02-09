#include "win.h"
#include "yuvbuf.h"
#include "screen_util.h"
#include "glshader.h"

extern egl_ctx_t egl_ctx;
extern screen_context_t screen_ctx;

static 	screen_pixmap_t pixmap_yuv;
static 	screen_pixmap_t pixmap_rgba;
static	struct egl_image yuv_img;

static GLuint gl_tex = 0;
static GLuint gl_pgm = 0;
static GLuint gl_uniform_src;
static GLuint gl_uniform_size;

static const char *vsrc = 
	"attribute vec4 vertex;\n"
	"varying mediump vec2 tcoord;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = vec4(vertex.xy, 0.0, 1.0);\n"
	"    tcoord = vertex.zw;\n"
	"}";

static const char * fsrc_yuyv = 
	"uniform sampler2D src;\n"
	"uniform highp vec2 tex_size;\n"
	"varying mediump vec2 tcoord;\n"
	"const lowp mat3 coeffs = mat3 ("
	"      1.1643,  1.1643, 1.1643,"
	"          0,   -0.39173, 2.017, "
	"      1.5958,  -0.81290, 0.0 ); \n"
	"#define yuv2rgba(yuv) "
	"    vec4(clamp(coeffs * yuv, vec3(0,0,0), vec3(1.0,1.0,1.0)), 1.0);\n"
	"highp vec4 sample_pix(sampler2D tex, highp vec2 pixcoord){"
	"     highp vec2 uv; "
	"     uv[0] = pixcoord.x * 0.5; "
	"     highp float r = fract(uv[0]); "
	"     uv[0] = floor(uv[0])/(tex_size[0]*0.5-1.0); "
	"     uv[1] = pixcoord.y/(tex_size[1]-1.0); "
	"     uv = clamp(vec2(0,0), vec2(1.0, 1.0), uv); "
	"     highp vec4 yuyv = texture2D(tex, uv);"
	"     highp float y = mix(yuyv[2], yuyv[0], ceil(r-0.25)); "
	"     return  vec4(y, yuyv.yw, 1.0); "
	"}\n"
	"mediump vec4 fs_tex2D(sampler2D tex, highp vec2 pos) {"
	"     highp vec2 pix = (tex_size-vec2(1.0, 1.0)) * pos;"
	"     return sample_pix(tex, floor(pix + vec2(0.5, 0.5))); "
	"}\n"
	"void main(void)\n"
	"{\n"
	"    highp vec4 yuva = fs_tex2D(src, tcoord) - vec4(0.0625, 0.5, 0.5, 0);"
	"    gl_FragColor = yuv2rgba(yuva.xyz);\n"
	"}";

static GLfloat vb_left[] = {
	-1.0f, -1.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
};

GLfloat vb_right[] = {
	0.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
};
	

static int
render_draw(win_t *win, GLfloat * vb)
{
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vb);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN,  0, 4);
	return 0;
}

/* blit uyvy to rgb565 buffer */
static int
render_blit(win_t *win)
{
	struct {
		int src_x[2];
		int src_y[2];
		int src_width[2];
		int src_height[2];
		int dst_x[2];
		int dst_y[2];
		int dst_width[2];
		int dst_height[2];
		int none;
	} screen_blit_attr = {
		.src_x = { SCREEN_BLIT_SOURCE_X, 0 },              
		.src_y = { SCREEN_BLIT_SOURCE_Y, 0 },              
		.src_width = { SCREEN_BLIT_SOURCE_WIDTH, 2*yuv_img.size[0] },
		.src_height = { SCREEN_BLIT_SOURCE_HEIGHT, yuv_img.size[1] },
		.dst_x = { SCREEN_BLIT_DESTINATION_X, 0 },         
		.dst_y = { SCREEN_BLIT_DESTINATION_Y, 0 },         
		.dst_width = { SCREEN_BLIT_DESTINATION_WIDTH, 2*yuv_img.size[0] }, 
		.dst_height = { SCREEN_BLIT_DESTINATION_HEIGHT, yuv_img.size[1] },
		.none = SCREEN_BLIT_END                           
	};
	screen_buffer_t yuv_buf = pixmap_buffer(pixmap_yuv);
	int rc = screen_blit(screen_ctx, win->rbuf,  yuv_buf, (const int *)&screen_blit_attr);
	if (rc) {
		perror("screen_blit");
		return rc;
	}
	screen_flush_blits(screen_ctx, 0);
	return 0;
}

int
render(win_t *win)
{
	int blit = 0;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if(blit){
		render_blit(win);
	}
	else{
		render_draw(win, vb_left);
	}

#if 0
	static int debug=0;
	if(debug==0){
		glDebugBufferDump("dump.png",1280, 900);
		debug = 1;
	}
#endif
	return 0;
}


int yuv_init()
{
	glActiveTexture(GL_TEXTURE0);
	pixmap_yuv = create_yuv_pixmap(screen_ctx, SCREEN_FORMAT_YUY2);
	screen_buffer_t buf = pixmap_buffer(pixmap_yuv);
	int size[2], stride, fmt;
	buffer_lock(buf, size, &stride, &fmt);
	buffer_unlock(buf);
	
	/* create a rgb565 faked pixmap */
	//screen_pixmap_t pixmap_rgba0;
	//pixmap_rgba0 = pixmap_attach(screen_ctx, pixmap_yuv, SCREEN_FORMAT_RGBA8888);
#if 0
	pixmap_rgba  = _pixmap_attach_buffer_as(screen_ctx,
										   buf,
										   size[0]/2,
										   size[1],
										   SCREEN_FORMAT_RGBA8888);
#else	
	pixmap_rgba  = pixmap_attach_buffer_as(screen_ctx,
										   buf,
										   size[0]/2,
										   size[1],
										   SCREEN_FORMAT_RGBA8888);
#endif	
	if (!pixmap_rgba) {
		fprintf(stderr, "pixmap_attach failed \n");
		return -1;
	}
	int rc = egl_img_init(&egl_ctx, pixmap_rgba, &yuv_img);
	if(rc){
		fprintf(stderr, "egl_img_init failed \n");
		return -1;
	}
	
	gl_pgm = glCreatePgm(vsrc, fsrc_yuyv, "vertex", 0);
	if(gl_pgm == -1){
		fprintf(stderr, "glCreatePgm failed \n");
		return -1;
	}
	gl_uniform_src  = glGetUniformLocation(gl_pgm, "src");
	gl_uniform_size = glGetUniformLocation(gl_pgm, "tex_size");
	
	glUseProgram(gl_pgm);
	glUniform1i(gl_uniform_src, 0);
	glUniform2f(gl_uniform_size,
				(float)yuv_img.size[0], (float)yuv_img.size[1]);
	
	GLenum target = GL_TEXTURE_2D; 
	glGenTextures(1, &gl_tex);
	glBindTexture(target, gl_tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	egl_ctx.pglEGLImageTargetTexture2D(target, (GLeglImageOES)yuv_img.img);

	return 0;
}
