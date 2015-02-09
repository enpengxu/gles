#ifndef CONVERT_H_
#define CONVERT_H_

#include <stdlib.h>

void rgb888_to_ayuv(unsigned char *dst, const unsigned char *src, size_t n);
void rgb888_to_nv12(unsigned char *dstY, unsigned char *dstUV, int dstride, const unsigned char *src, int sstride, size_t n);
void rgb888_to_uyvy(unsigned char *dst, const unsigned char *src, size_t n);
void rgb888_to_yuy2(unsigned char *dst, const unsigned char *src, size_t n);
void rgb888_to_yvyu(unsigned char *dst, const unsigned char *src, size_t n);

void fill_ayuv(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b);
void fill_nv12(unsigned char *dstY, unsigned char *dstUV, int dstride, size_t n, unsigned char r, unsigned char g, unsigned char b);
void fill_uyvy(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b);
void fill_yuy2(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b);
void fill_yvyu(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b);

#endif /* CONVERT_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
