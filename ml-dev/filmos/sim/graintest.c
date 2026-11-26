/* graintest: the grain the darkroom draws is the film stock's.
 *
 *   ./out/graintest
 *
 * A film stock's look gives a strength (grain) and a clump width (grain_size). The print's
 * grain has to be as strong at every clump width, its clumps as wide as Darkrooms draws them
 * -- grain_size against the frame's short side, so wider on a bigger print -- and a clump must
 * not turn into a pattern of its own (no alternation: #104).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../fos.h"
#include "../fos_dev.h"

/* the camera's port, as far as a test needs it: nothing */
void *fos_port_alloc(int size) { return malloc(size); }
void  fos_port_free(void *p) { free(p); }
void  fos_port_log(const char *msg) { (void) msg; }
int   fos_port_ms(void) { return 0; }
int   fos_port_dev_cap(void) { return 0; }
void  fos_dev_test_sample(int x, int y, int v) { (void) x; (void) y; (void) v; }

#define W 720
#define H 160

static uint8_t print[H * W * 3];
static int fail;

static void ok(const char *what, int pass, const char *detail)
{
    if (!pass) fail = 1;
    printf("%s  %s%s%s\n", pass ? "ok  " : "FAIL", what, *detail ? ": " : "", detail);
}

/* Darkrooms' grain profile, read off its own prints of the same frames at 2736 x 1824 and
 * 5472 x 3648 (three film stocks at both sizes): at u = lag / clump, how much a pixel's grain
 * agrees with the one u clumps along. It is the same shape at every film stock and every size. */
static const double RHO[][2] = {
    { 0.0, 1.000 }, { 0.5, 0.887 }, { 0.667, 0.761 }, { 0.75, 0.699 }, { 1.0, 0.653 },
    { 1.333, 0.442 }, { 1.5, 0.400 }, { 2.0, 0.242 }, { 2.5, 0.104 }, { 3.0, 0.088 },
    { 3.5, 0.060 }, { 4.0, 0.039 }, { 5.0, 0.008 }, { 6.0, 0.002 }, { 8.0, 0.0 }
};

static double rho(double u)
{
    unsigned n = sizeof(RHO) / sizeof(RHO[0]);
    if (u <= RHO[0][0]) return RHO[0][1];
    for (unsigned i = 1; i < n; i++)
        if (u <= RHO[i][0])
            return RHO[i - 1][1] + (RHO[i][1] - RHO[i - 1][1]) *
                   (u - RHO[i - 1][0]) / (RHO[i][0] - RHO[i - 1][0]);
    return 0.0;
}

/* the grain drawn: its strength in levels, and how much a pixel's grain agrees with the one
 * beside it (the clump's width) and with the one two along */
static void measure(int mono, int c, int level, double *strength, double *r1, double *r2)
{
    fos_dev_test_grain(print, W, H, c, 2400, mono, level);
    double m = 0, v = 0, c1 = 0, c2 = 0;
    long n = 0;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) m += print[3 * (y * W + x) + 1], n++;
    m /= n;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            double a = print[3 * (y * W + x) + 1] - m;
            v += a * a;
            if (x + 1 < W) c1 += a * (print[3 * (y * W + x + 1) + 1] - m);
            if (x + 2 < W) c2 += a * (print[3 * (y * W + x + 2) + 1] - m);
        }
    v /= n;
    *strength = sqrt(v);
    *r1 = c1 / (v * n);
    *r2 = c2 / (v * n);
}

/* the drawn grain's own clump, for the plated runs to be held to */
static double drawn_a1, drawn_a2;

/* the mean and the variance of a field's green channel */
static void mean_var(const uint8_t *p, int w, int h, double *m, double *v)
{
    double s = 0, q = 0;
    long n = (long) w * h;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) s += p[3 * (y * w + x) + 1];
    s /= n;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) { double a = p[3 * (y * w + x) + 1] - s; q += a * a; }
    *m = s; *v = q / n;
}

/* how much a pixel's grain agrees with the one d along the row, and with the one d down the
 * column, in the terms measure() uses */
/* Both columns are centred on the field's own mean, not on their own: a pair that repeats still
 * reads exactly 1, which is what the repeat guard turns on, while the edge readings are a shifted
 * correlation whose bounds are the measured ones.  v is past the point of the correlation. */
static double col_agree(const uint8_t *p, int w, int h, int x, int d, double m, double v)
{
    double s = 0, a2 = 0, b2 = 0;
    (void) v;
    for (int y = 0; y < h; y++)
    {
        double a = p[3 * (y * w + x) + 1] - m, b = p[3 * (y * w + x + d) + 1] - m;
        s += a * b; a2 += a * a; b2 += b * b;
    }
    return s / sqrt(a2 * b2 + 1e-9);      /* the correlation: a column that repeats reads 1 */
}

static double row_agree(const uint8_t *p, int w, int h, int y, int d, double m, double v)
{
    double s = 0, a2 = 0, b2 = 0;
    (void) v;
    for (int x = 0; x < w; x++)
    {
        double a = p[3 * (y * w + x) + 1] - m, b = p[3 * ((y + d) * w + x) + 1] - m;
        s += a * b; a2 += a * a; b2 += b * b;
    }
    return s / sqrt(a2 * b2 + 1e-9);
}

/* A field tall enough to hold several of every plate's row edges, and wide enough for its column
 * edges: WF x W is 1000 rows an edge column, and 4 and 6 edge columns for the camera's pair (10 and
 * 14 for 128+127), which is what lets the check see a seam at all. */
#define WF 1000
static uint8_t platef[WF * W * 3];

/* The plate's checks, for one configuration: with no FOS_PLATE the grain is drawn, and the
 * strength is still the absolute the stock asks for.  With a plate, its edges have to meet
 * themselves on both axes and a pair must not repeat a column inside the print (#188, review). */
static void check_plate(const char *lg, const char *p2)
{
    char b[200];
    int p0 = lg ? 1 << atoi(lg) : 0;
    int p1 = lg && p2 && atoi(p2) >= 64 ? atoi(p2) : 0;
    /* The checks read the field at x + these and count edges modulo them: refuse a pair that would
     * read past the field, leave no edge to find, or be too small for the plate to be built at all
     * - plate_lg under 6 leaves the grain drawn, and the checks would then pass on nothing (#188). */
    if (lg && (p0 < 64 || p0 > W - 2 || p0 > WF - 2))
    { printf("skip  a plate of %d is not checkable here\n", p0); return; }
    if (p1 && (p1 < 64 || p1 > W - 2 || p1 > WF - 2))
    { printf("skip  a second plate of %d is not checkable here\n", p1); return; }
    if (lg)
    {
        setenv("FOS_PLATE", lg, 1);
        setenv("FOS_PLATE2", p2 ? p2 : "0", 1);
        setenv("FOS_PLATE_ORIGIN", "0", 1);   /* so the test knows where the plate's edges fall */
    }
    else
    {
        unsetenv("FOS_PLATE");
        unsetenv("FOS_PLATE2");
    }
    fos_dev_test_plate_reset();               /* nothing left of the configuration before */

    /* the strength is an absolute number of levels at a one pixel clump, where the grain is a draw
     * a pixel: a plate has to come to the same number, not to a fraction or a multiple of it.  The
     * grain itself draws 5.33 levels in colour at 120 (#188, review). */
    {
        double s, a1, a2;
        measure(0, 256, 120, &s, &a1, &a2);
        snprintf(b, sizeof(b), "%s clump 1 px: %.2f levels against the grain's own 5.33",
                 lg ? "plated," : "drawn,", s);
        ok("the strength is the stock's, not a fraction of it", fabs(s - 5.33) < 0.25, b);
        if (!lg) { drawn_a1 = a1; drawn_a2 = a2; return; }
        /* the clump is the taps' doing: a plate built from the wrong taps keeps its strength and
         * loses this, which the strength check alone cannot see (#188, review) */
        snprintf(b, sizeof(b), "plated, clump 1 px: %.3f and %.3f against the drawn %.3f and %.3f",
                 a1, a2, drawn_a1, drawn_a2);
        ok("the plate's clump is the drawn grain's", fabs(a1 - drawn_a1) < 0.06 && fabs(a2 - drawn_a2) < 0.06, b);
    }

    {
        double m, v, e, in, r0, r1;
        int ne, ni;

        fos_dev_test_grain(platef, W, WF, 256, 2400, 0, 120);
        mean_var(platef, W, WF, &m, &v);

        /* every place the plate's index runs off one of its edges in this field, against the same
         * field's own inside: the seam this is for takes about 0.6 off the agreement, and the
         * estimate itself wanders by a tenth or two, so the bound catches the seam, not the wander */
        e = in = 0; ne = ni = 0;
        for (int x = 0; x + 1 < W; x++)
        {
            double a = col_agree(platef, W, WF, x, 1, m, v);
            if ((x + 1) % p0 == 0 || (p1 && (x + 1) % p1 == 0)) { e += a; ne++; }
            else if (x % 5 == 0) { in += a; ni++; }
        }
        e /= ne; in /= ni;
        snprintf(b, sizeof(b), "%d+%d across its edges %.3f at %d of them, inside it %.3f",
                 p0, p1, e, ne, in);
        ok("the plate's edges meet themselves", fabs(e - in) < 0.25, b);

        e = in = 0; ne = ni = 0;
        for (int y = 0; y + 1 < WF; y++)
        {
            double a = row_agree(platef, W, WF, y, 1, m, v);
            if ((y + 1) % p0 == 0 || (p1 && (y + 1) % p1 == 0)) { e += a; ne++; }
            else if (y % 5 == 0) { in += a; ni++; }
        }
        e /= ne; in /= ni;
        snprintf(b, sizeof(b), "%d+%d down its edges %.3f at %d of them, inside it %.3f",
                 p0, p1, e, ne, in);
        ok("the plate's edges meet themselves down the print", fabs(e - in) < 0.25, b);

        if (p1)
        {
            /* read from column 0, which is what makes the clamp's p <= W - 2 the whole bound */
            r0 = col_agree(platef, W, WF, 0, p0, m, v);
            r1 = col_agree(platef, W, WF, 0, p1, m, v);
            snprintf(b, sizeof(b), "%d+%d one plate along: %.3f at %d columns, %.3f at %d",
                     p0, p1, r0, p0, r1, p1);
            /* a repeat is 1.000 here, the camera's pair 0.87 and 0.80: an unnormalised sum read the
             * repeat *lower* than the healthy pair and passed it (#188, review) */
            ok("the pair does not repeat a column", fabs(r0) < 0.95 && fabs(r1) < 0.95, b);
        }
    }
}

int main(void)
{
    char b[200];
    static const int mono[2] = { 0, 1 };
    /* the clumps, in 1/256 of a print pixel: a draw a pixel, then a quarter of one and up */
    static const int cs[] = { 0, 64, 128, 256, 384, 512, 640 };

    /* the strength is the film stock's at every clump */
    for (int m = 0; m < 2; m++)
    {
        double s0, r1, r2, s, a1, a2;
        measure(mono[m], 0, 120, &s0, &r1, &r2);
        for (unsigned i = 1; i < sizeof(cs) / sizeof(cs[0]); i++)
        {
            measure(mono[m], cs[i], 120, &s, &a1, &a2);
            snprintf(b, sizeof(b), "%s clump of %d/256 of a pixel: %.2f levels against %.2f at a draw a pixel",
                     mono[m] ? "mono" : "colour", cs[i], s, s0);
            ok("the same strength", s > s0 * 0.85 && s < s0 * 1.15, b);
        }
    }

    /* and the clump drawn is Darkrooms': its agreement with the pixel beside it and with the one
     * two along is the profile read off Darkrooms' prints at that clump, from a quarter of a pixel
     * to a pixel and a half (#104) */
    for (unsigned i = 1; i < sizeof(cs) / sizeof(cs[0]) && cs[i] <= 384; i++)
    {
        double s, a1, a2, sd = cs[i] / 256.0;
        measure(0, cs[i], 120, &s, &a1, &a2);
        double want1 = rho(1.0 / sd), want2 = rho(2.0 / sd);
        snprintf(b, sizeof(b), "clump %.2f px: %.3f against Darkrooms' %.3f at one, %.3f against %.3f at two",
                 sd, a1, want1, a2, want2);
        ok("the clump is the one Darkrooms draws", fabs(a1 - want1) < 0.06 && fabs(a2 - want2) < 0.06, b);
    }

    /* the clumps are the film stock's: a wider one agrees more with its neighbour */
    for (int m = 0; m < 2; m++)
    {
        double s, a1, a2, prev = -1;
        /* from a clump of a third of a pixel up: below that the grain is one draw a pixel and cannot
         * be finer, so there is nothing there to widen */
        for (int c = 96; c <= 384; c += 32)
        {
            measure(mono[m], c, 120, &s, &a1, &a2);
            snprintf(b, sizeof(b), "%s, %d/256 of a pixel: %.3f (was %.3f at less)", mono[m] ? "mono" : "colour", c, a1, prev);
            ok("clumps wider with the size", a1 > prev, b);
            prev = a1;
        }
    }

    /* the weight the grain is added by is the lab's, strongest in the mid tones:
     * 160/247 at 40, 206/247 at 190 (#104, #186) */
    {
        double s40, s96, s190, d1, d2;
        measure(0, 1, 40, &s40, &d1, &d2);
        measure(0, 1, 96, &s96, &d1, &d2);
        measure(0, 1, 190, &s190, &d1, &d2);
        snprintf(b, sizeof(b), "40: %.2f, 96: %.2f, 190: %.2f levels", s40, s96, s190);
        ok("the weight is a dome over the tones", s96 > s40 * 1.4 && s96 > s190 * 1.1, b);
        snprintf(b, sizeof(b), "%.2f levels against the mid tones' %.2f, the lab's 160/247", s40, s96);
        ok("the weight over the shadows is the lab's", fabs(s40 - s96 * 160.0 / 247.0) < s96 * 0.05, b);
        snprintf(b, sizeof(b), "%.2f levels against the mid tones' %.2f, the lab's 206/247", s190, s96);
        ok("the weight at 190 is the lab's", fabs(s190 - s96 * 206.0 / 247.0) < s96 * 0.05, b);
    }

    /* and a clump is not a pattern: no alternation of the grain */
    for (int m = 0; m < 2; m++)
        for (int c = 0; c <= 768; c += 32)
        {
            double s, a1, a2;
            measure(mono[m], c, 120, &s, &a1, &a2);
            if (a2 <= -0.05)
            {
                snprintf(b, sizeof(b), "%s, %d/256 of a pixel: %.3f at two", mono[m] ? "mono" : "colour", c, a2);
                ok("no alternation", 0, b);
            }
        }
    ok("no alternation", 1, "every clump 0..1.5 pixels agrees with the one two along");

    /* the looks map: a coarser stock's grain is wider on every print, on a print a third of
     * the frame wide as wide as the stock's, and wider on a bigger print */
    {
        const struct fos_look *fast = 0, *slow = 0;
        for (int i = 0; i < fos_nlooks; i++)
        {
            if (!strcmp(fos_looks[i].key, "trix400")) fast = &fos_looks[i];
            if (!strcmp(fos_looks[i].key, "ektar100")) slow = &fos_looks[i];
        }
        if (!fast || !slow) ok("two film stocks to compare", 0, "trix400 and ektar100 are not in fos_looks");
        else
        {
            int f, s;
            f = fos_dev_test_grain_clump(fast, 1728, 3456);
            s = fos_dev_test_grain_clump(slow, 1728, 3456);
            snprintf(b, sizeof(b), "trix400 %d/256 of a pixel, ektar100 %d/256, on a third of the frame", f, s);
            ok("the coarser stock's grain is wider", f > s, b);

            int f3 = fos_dev_test_grain_clump(fast, 2592, 3456), s3 = fos_dev_test_grain_clump(slow, 2592, 3456);
            snprintf(b, sizeof(b), "large: trix400 %d/256 of a pixel, ektar100 %d/256", f3, s3);
            ok("a bigger print shows the same grain wider", f3 > f && s3 > s, b);

            /* the clump's width goes with the print's, one to one */
            snprintf(b, sizeof(b), "a third of the frame %d/256, half of it %d/256 (1.5 times)", f, f3);
            ok("in step with the print's width", (f3 * 10 > f * 14 && f3 * 10 < f * 16), b);

            snprintf(b, sizeof(b), "a print smaller than a twelfth of the frame: %d/256 of a pixel",
                     fos_dev_test_grain_clump(slow, 400, 3456));
            ok("no clump at all where the print can't show it", fos_dev_test_grain_clump(slow, 400, 3456) <= 26, b);
        }
    }

    /* the camera's 257 fold, which the simulator never takes, has to be the modulo exactly */
    {
        int bad = 0;
        for (int x = -300; x < 40000; x++)
            if (fos_dev_test_fold257(x) != ((x % 257) + 257) % 257) bad++;
        snprintf(b, sizeof(b), "%d of 40300 differ from the modulo", bad);
        ok("the 257 fold is the modulo", bad == 0, b);
    }

    /* the same checks with the grain drawn into a plate: the camera's own pair of 256 and 257, one
     * plate on its own, and a smaller pair whose second plate, 127, is awkwardly sized as well --
     * so the one run of this test covers the plate, and CI does not have to set anything */
    check_plate(NULL, NULL);
    check_plate("8", "257");
    check_plate("8", NULL);            /* one plate alone, which the camera never builds */
    check_plate("7", "127");

    printf("graintest: %s\n", fail ? "failed" : "all passed");
    return fail;
}