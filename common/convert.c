#include <gulliver.h>
#include <stdint.h>
#include "convert.h"

static inline uint8_t
rgb_to_y(uint8_t R, uint8_t G, uint8_t B)
{
    return min(abs(R * 2104 + G * 4130 + B * 802 + 4096 + 131072) >> 13, 235);
}

static inline int
rgb_to_cr(uint8_t R, uint8_t G, uint8_t B)
{
    return min(abs(R * -1214 + G * -2384 + B * 3598 + 4096 + 1048576) >> 13, 240);
}

static inline int
rgb_to_cb(uint8_t R, uint8_t G, uint8_t B)
{
    return min(abs(R * 3598 + G * -3013 + B * -585 + 4096 + 1048576) >> 13, 240);
}

void
rgb888_to_ayuv(unsigned char *dst, const unsigned char *src, size_t n)
{
    uint32_t *d = (uint32_t *)dst;

    do {
        *d = 0xff000000 |
            (rgb_to_y(src[0], src[1], src[2]) & 0xff) << 16 |
            (rgb_to_cr(src[0], src[1], src[2]) & 0xff) << 8 |
            (rgb_to_cb(src[0], src[1], src[2]) & 0xff);

        src += 3;
        d++;
    } while (--n);
}

void
rgb888_to_nv12(unsigned char *dstY, unsigned char *dstUV, int dstride, const unsigned char *src, int sstride, size_t n)
{
    int u, v;

    do {
        dstY[0] = rgb_to_y(src[0], src[1], src[2]);
        u = rgb_to_cr(src[0], src[1], src[2]);
        v = rgb_to_cb(src[0], src[1], src[2]);

        dstY[1] = rgb_to_y(src[3], src[4], src[5]);
        u += rgb_to_cr(src[3], src[4], src[5]);
        v += rgb_to_cb(src[3], src[4], src[5]);

        dstY[dstride] = rgb_to_y(src[sstride], src[sstride+1], src[sstride+2]);
        u += rgb_to_cr(src[sstride], src[sstride+1], src[sstride+2]);
        v += rgb_to_cb(src[sstride], src[sstride+1], src[sstride+2]);

        dstY[dstride+1] = rgb_to_y(src[sstride+3], src[sstride+4], src[sstride+5]);
        u += rgb_to_cr(src[sstride+3], src[sstride+4], src[sstride+5]);
        v += rgb_to_cb(src[sstride+3], src[sstride+4], src[sstride+5]);

        dstUV[0] = u >> 2;
        dstUV[1] = v >> 2;

        src += 6;
        dstY += 2;
        dstUV += 2;
    } while ((n -= 2) > 0);
}

void
rgb888_to_uyvy(unsigned char *dst, const unsigned char *src, size_t n)
{
    do {
#ifdef __LITTLEENDIAN__
        dst[0] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
        dst[1] = rgb_to_y(src[0], src[1], src[2]);
        dst[2] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
        dst[3] = rgb_to_y(src[3], src[4], src[5]);
#else
        dst[0] = rgb_to_y(src[3], src[4], src[5]);
        dst[1] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
        dst[2] = rgb_to_y(src[0], src[1], src[2]);
        dst[3] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
#endif
        src += 6;
        dst += 4;
    } while ((n -= 2) > 0);
}

void
rgb888_to_yuy2(unsigned char *dst, const unsigned char *src, size_t n)
{
    do {
#ifdef __LITTLEENDIAN__
        dst[0] = rgb_to_y(src[0], src[1], src[2]);
        dst[1] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
        dst[2] = rgb_to_y(src[3], src[4], src[5]);
        dst[3] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
#else
        dst[0] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
        dst[1] = rgb_to_y(src[3], src[4], src[5]);
        dst[2] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
        dst[3] = rgb_to_y(src[0], src[1], src[2]);
#endif
        src += 6;
        dst += 4;
    } while ((n -= 2) > 0);
}

void
rgb888_to_yvyu(unsigned char *dst, const unsigned char *src, size_t n)
{
    do {
#ifdef __LITTLEENDIAN__
        dst[0] = rgb_to_y(src[0], src[1], src[2]);
        dst[1] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
        dst[2] = rgb_to_y(src[3], src[4], src[5]);
        dst[3] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
#else
        dst[0] = (rgb_to_cr(src[0], src[1], src[2]) + rgb_to_cr(src[3], src[4], src[5])) / 2;
        dst[1] = rgb_to_y(src[3], src[4], src[5]);
        dst[2] = (rgb_to_cb(src[0], src[1], src[2]) + rgb_to_cb(src[3], src[4], src[5])) / 2;
        dst[3] = rgb_to_y(src[0], src[1], src[2]);
#endif
        src += 6;
        dst += 4;
    } while ((n -= 2) > 0);
}

void
fill_ayuv(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b)
{
    uint32_t *d = (uint32_t *)dst;
    uint32_t ayuv;

    ayuv = 0xff000000 |
            (rgb_to_y(r, g, b) & 0xff) << 16 |
            (rgb_to_cr(r, g, b) & 0xff) << 8 |
            (rgb_to_cb(r, g, b) & 0xff);

    do {
        *d = ayuv;
        d++;
    } while (--n);
}

void
fill_nv12(unsigned char *dstY, unsigned char *dstUV, int dstride, size_t n, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char y = rgb_to_y(r, g, b);
    unsigned char u = rgb_to_cr(r, g, b);
    unsigned char v = rgb_to_cb(r, g, b);

    do {
        dstY[0] = y;
        dstY[1] = y;
        dstY[dstride] = y;
        dstY[dstride+1] = y;
        dstUV[0] = u;
        dstUV[1] = v;

        dstY += 2;
        dstUV += 2;
    } while ((n -= 2) > 0);
}

void
fill_uyvy(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char y = rgb_to_y(r, g, b);
    unsigned char u = rgb_to_cr(r, g, b);
    unsigned char v = rgb_to_cb(r, g, b);

    do {
#ifdef __LITTLEENDIAN__
        dst[0] = u;
        dst[1] = y;
        dst[2] = v;
        dst[3] = y;
#else
        dst[0] = y;
        dst[1] = v;
        dst[2] = y;
        dst[3] = u;
#endif
        dst += 4;
    } while ((n -= 2) > 0);
}

void
fill_yuy2(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char y = rgb_to_y(r, g, b);
    unsigned char u = rgb_to_cr(r, g, b);
    unsigned char v = rgb_to_cb(r, g, b);

    do {
#ifdef __LITTLEENDIAN__
        dst[0] = y;
        dst[1] = u;
        dst[2] = y;
        dst[3] = v;
#else
        dst[0] = v;
        dst[1] = y;
        dst[2] = u;
        dst[3] = y;
#endif
        dst += 4;
    } while ((n -= 2) > 0);
}

void
fill_yvyu(unsigned char *dst, size_t n, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char y = rgb_to_y(r, g, b);
    unsigned char u = rgb_to_cr(r, g, b);
    unsigned char v = rgb_to_cb(r, g, b);

    do {
#ifdef __LITTLEENDIAN__
        dst[0] = y;
        dst[1] = v;
        dst[2] = y;
        dst[3] = u;
#else
        dst[0] = u;
        dst[1] = y;
        dst[2] = v;
        dst[3] = y;
#endif
        dst += 4;
    } while ((n -= 2) > 0);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
