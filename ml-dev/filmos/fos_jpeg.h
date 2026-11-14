#ifndef _fos_jpeg_h_
#define _fos_jpeg_h_

#include <stdint.h>

/* what goes into the JPEG's EXIF (any may be NULL) */
struct fos_jpeg_meta
{
    const char *when;           /* "2026-10-22 17:45" */
    const char *desc;           /* the film */
    const char *fstop;          /* "2.8" */
};

/* width x height RGB pixels (rows stride bytes apart) as a baseline JPEG
 * at path; quality 1..100. 1 when written. */
/* the same a band of rows at a time, for a picture made as it is written:
 * begin, then its rows from the top, 16 at a time (fewer only at the
 * bottom), then end (1 if all of it was written). NULL: can't be opened. */
struct fos_jpeg;
struct fos_jpeg *fos_jpeg_begin(const char *path, int width, int height, int quality, const struct fos_jpeg_meta *meta);
void fos_jpeg_rows(struct fos_jpeg *j, const uint8_t *rgb, int stride, int rows);
int  fos_jpeg_end(struct fos_jpeg *j);
/* fos_jpeg_rows in two parts, for a band of 16 rows (fewer only at the
 * bottom) whose blocks are made on two cores: fos_jpeg_prep makes the
 * coefficients of 16 x 16 blocks m0 .. m1 - 1 (of fos_jpeg_mcus) into
 * coef, FOS_JPEG_MCU each at its place, and only reads j; fos_jpeg_emit
 * then writes them all, in order. The file is byte for byte the same. */
#define FOS_JPEG_MCU 384
int  fos_jpeg_mcus(const struct fos_jpeg *j);
void fos_jpeg_prep(const struct fos_jpeg *j, const uint8_t *rgb, int stride, int rows, int m0, int m1, int16_t *coef);
void fos_jpeg_emit(struct fos_jpeg *j, const int16_t *coef, int rows);

int fos_jpeg_write(const char *path, const uint8_t *rgb, int stride, int width, int height,
                   int quality, const struct fos_jpeg_meta *meta);

#endif
