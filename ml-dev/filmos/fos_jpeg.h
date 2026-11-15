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
/* fos_jpeg_rows on two cores, for a band of 16 rows (fewer only at the
 * bottom). The file has a restart marker every few 16 x 16 blocks, so a
 * row's blocks from fos_jpeg_split on (0: the row is one piece) are a part
 * of it on their own: fos_jpeg_part makes them on the other core, their
 * coefficients into coef (FOS_JPEG_MCU a block, at its place in the row)
 * and their bits into o, and only reads j. This core makes the blocks
 * before them with fos_jpeg_main, then fos_jpeg_join puts o's bits after
 * them (o too small: it writes them again from coef). */
#define FOS_JPEG_MCU 384
struct fos_jpeg_out { void *f; uint8_t *buf; int n, cap, ok; uint32_t bits; int nbits; };
int  fos_jpeg_mcus(const struct fos_jpeg *j);
int  fos_jpeg_split(const struct fos_jpeg *j);
void fos_jpeg_part(const struct fos_jpeg *j, const uint8_t *rgb, int stride, int rows, int16_t *coef, struct fos_jpeg_out *o);
void fos_jpeg_main(struct fos_jpeg *j, const uint8_t *rgb, int stride, int rows);
void fos_jpeg_join(struct fos_jpeg *j, const int16_t *coef, struct fos_jpeg_out *o, int rows);

int fos_jpeg_write(const char *path, const uint8_t *rgb, int stride, int width, int height,
                   int quality, const struct fos_jpeg_meta *meta);

#endif
