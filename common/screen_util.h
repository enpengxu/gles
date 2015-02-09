#ifndef __screen_util_h__
#define __screen_util_h__

#include <screen/screen.h>
#include <malloc.h>
#include <memory.h>

static inline void *
buffer_lock(screen_buffer_t buf, int * size, int *stride, int * fmt)
{
	void *ptr;
	int rc = screen_get_buffer_property_pv(buf, SCREEN_PROPERTY_POINTER, &ptr);
	if (rc) {
		return 0;
	}
	rc = screen_get_buffer_property_iv(buf, SCREEN_PROPERTY_STRIDE, stride);
	if (rc) {
		return 0;
	}
	rc = screen_get_buffer_property_iv(buf, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if (rc) {
		return 0;
	}
	rc = screen_get_buffer_property_iv(buf, SCREEN_PROPERTY_FORMAT, fmt);
	if (rc) {
		return 0;
	}
	return ptr;
}

static inline void
buffer_unlock(screen_buffer_t buf)
{
	/* nothing to do */
}


static inline screen_buffer_t
pixmap_buffer(screen_pixmap_t pixmap)
{
	screen_buffer_t buf;
	if( 0 == screen_get_pixmap_property_pv(pixmap,
										   SCREEN_PROPERTY_RENDER_BUFFERS,
										   (void **)&buf)){
		return buf;
	}
	return 0;
}

static inline screen_pixmap_t
pixmap_create(screen_context_t ctx,
			  int format,
			  int size[2],
			  int usage,
			  int nbuffers)
{
	screen_pixmap_t pix;
	int rc = screen_create_pixmap(&pix, ctx);
	if(rc){
		return 0;
	}
	rc = screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_USAGE, &usage);
	if (rc) {
		screen_destroy_pixmap(pix);
		return 0;
	}
	rc = screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_FORMAT, &format);
	if (rc) {
		screen_destroy_pixmap(pix);
		return 0;
	}
	rc = screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if (rc) {
		screen_destroy_pixmap(pix);
		return 0;
	}
	/* create buffer */
	if(nbuffers > 0){
		rc = screen_create_pixmap_buffer(pix);
		if (rc) {
			screen_destroy_pixmap(pix);
			return 0;
		}
	}
	return pix;
}

static inline int
pixmap_destroy(screen_pixmap_t pix){
	return screen_destroy_pixmap(pix);
}

static inline screen_pixmap_t
pixmap_from_buffer(screen_context_t ctx, screen_buffer_t buf)
{
	screen_pixmap_t pix;

	int rc = screen_create_pixmap(&pix, ctx);
	if(rc){
		return 0;
	}
	rc = screen_attach_pixmap_buffer(pix, buf);	
	if(rc){
		screen_destroy_pixmap(pix);
		pix = 0;
	}
	return pix;
}
static inline screen_pixmap_t
pixmap_from_buffer_format(screen_context_t ctx, screen_buffer_t buf, int format)
{
	screen_pixmap_t pix;

	int rc = screen_create_pixmap(&pix, ctx);
	if(rc){
		return 0;
	}
	rc = screen_set_pixmap_property_iv(pix, SCREEN_PROPERTY_FORMAT, &format);
	if(rc){
		screen_destroy_pixmap(pix);
		pix = 0;
	}
	rc = screen_attach_pixmap_buffer(pix, buf);	
	if(rc){
		screen_destroy_pixmap(pix);
		pix = 0;
	}
	return pix;
}

static inline screen_pixmap_t
pixmap_attach_buffer_as(screen_context_t ctx, screen_buffer_t sbuf,
						int w, int h, int fmt)
{
	void * img;
	int size[2];
	screen_pixmap_t pix;
	int rc = screen_create_pixmap(&pix, ctx);
	if(rc){
		return 0;
	}
	screen_buffer_t dbuf;
	rc = screen_create_buffer(&dbuf);
	if(rc){
		return 0;
	}
	rc = screen_get_buffer_property_pv(sbuf, SCREEN_PROPERTY_NATIVE_IMAGE, &img);
	if(rc){
		return 0;
	}
	rc = screen_set_buffer_property_pv(dbuf, SCREEN_PROPERTY_NATIVE_IMAGE, &img);
	if(rc){
		return 0;
	}
	size[0] = w, size[1] = h;
	rc = screen_set_buffer_property_iv(dbuf, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if(rc){
		return 0;
	}
	rc = screen_set_buffer_property_iv(dbuf, SCREEN_PROPERTY_FORMAT, &fmt);
	if(rc){
		return 0;
	}
	rc = screen_attach_pixmap_buffer(pix, dbuf);
	if(rc){
		return 0;
	}
	return pix;
}		

#endif
