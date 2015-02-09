#include "screen_util.h"
#include "pattern.pix"
#include "convert.h"

/* create a yuv buffer and fill pattern data in it */
screen_pixmap_t
create_yuv_pixmap(screen_context_t ctx, int format)
{
	int i;
	int bsize[2] = { pattern.width, pattern.height };
	int usage = SCREEN_USAGE_WRITE|SCREEN_USAGE_VIDEO;
	int stride;

	switch (format) {
		case SCREEN_FORMAT_YVU9:
			bsize[0] = pattern.width & ~3;
			bsize[1] = pattern.height & ~3;
			break;
		case SCREEN_FORMAT_YUV420:
		case SCREEN_FORMAT_NV12:
		case SCREEN_FORMAT_YV12:
			bsize[0] = pattern.width & ~1;
			bsize[1] = pattern.height & ~1;
			break;
		case SCREEN_FORMAT_UYVY:
		case SCREEN_FORMAT_YUY2:
		case SCREEN_FORMAT_YVYU:
		case SCREEN_FORMAT_V422:
			bsize[0] = pattern.width & ~1;
			bsize[1] = pattern.height;
			break;
		case SCREEN_FORMAT_AYUV:
			bsize[0] = pattern.width;
			bsize[1] = pattern.height;
			break;
		default:
			return 0;
	}
	screen_pixmap_t pix = pixmap_create(ctx, format, bsize, usage, 1);
	if(!pix){
		return 0;
	}
	screen_buffer_t buf = pixmap_buffer(pix);
	if(!buf){
		pixmap_destroy(pix);
		return 0;
	}
	void * pointer = buffer_lock(buf, bsize, &stride, &format);
	if(!pointer){
		pixmap_destroy(pix);
		return 0;
	}
	int dstride;
	int sstride = pattern.width * pattern.bytes_per_pixel;
	const unsigned char *src;
	unsigned char *dstUV;
	unsigned char *dstY;
	unsigned char *dst;
	
	if (format != SCREEN_FORMAT_NV12) {
		if (format != SCREEN_FORMAT_V422) {
			dst = pointer;
			src = pattern.pixel_data;
			dstride = stride;
		} else {
			dst = pointer + stride * (bsize[1] - 1);
			src = pattern.pixel_data;
			dstride = -stride;
		}
		for (i = 0; i < bsize[1]; i++) {
			switch (format) {
			case SCREEN_FORMAT_UYVY:
			case SCREEN_FORMAT_V422:
				rgb888_to_uyvy(dst, src, bsize[0]);
				break;
			case SCREEN_FORMAT_YUY2:
				rgb888_to_yuy2(dst, src, bsize[0]);
					break;
			case SCREEN_FORMAT_YVYU:
				rgb888_to_yvyu(dst, src, bsize[0]);
				break;
			case SCREEN_FORMAT_AYUV:
				rgb888_to_ayuv(dst, src, bsize[0]);
				break;
			default:
				fprintf(stderr, "unsupported pixel format\n");
				pixmap_destroy(pix);
				return 0;
			}
			src += sstride;
			dst += dstride;
		}
	} else {
		dstride = stride;
		dstY = pointer;
		dstUV = pointer + dstride * bsize[1];
		src = pattern.pixel_data;
		
		for (i = 0; i < bsize[1]; i += 2) {
			rgb888_to_nv12(dstY, dstUV, dstride, src, sstride, bsize[0]);
			src += sstride * 2;
			dstY += dstride * 2;
			dstUV += dstride;
		}
	}
	buffer_unlock(buf);
	return pix;
}
