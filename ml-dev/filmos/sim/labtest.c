/* labtest: a roll's RAW files found in another DCIM folder than its log (fos_lab_frames).
 *   mkdir -p /tmp/card/DCIM/101CANON /tmp/card/DCIM/101TEST_; a ROLL_013.TXT with IMG_9115/9116 in 101CANON,
 *   the two files in 101TEST_; cc -I.. -DFOS_DEV_TEST -o /tmp/lt labtest.c port_files.c films.c ../fos_lab.c ../fos_film.c ../fos_gfx.c -lm
 *   FOS_CARD=/tmp/card/ /tmp/lt */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fos.h"
#include "fos_dev.h"
#include <stdlib.h>
void *fos_port_alloc(int n){return malloc(n);} void fos_port_free(void *p){free(p);} void fos_port_log(const char *m){puts(m);} void fos_port_roll(struct fos_roll *r){memset(r,0,sizeof(*r));}
int main(void)
{
    struct fos_lab_frame fr[36];
    int n = fos_lab_frames(13, fr, 36);
    printf("frames %d\n", n);
    for (int i = 0; i < n; i++) printf("  %d %s missing %d\n", fr[i].no, fr[i].cr2, fr[i].missing);
    assert(n == 2 && !fr[0].missing && !fr[1].missing && strstr(fr[0].cr2, "101TEST_/IMG_9115.CR2"));
    puts("ok");
    return 0;
}
