#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "md5.h"

int main (int argc, char **argv)
{
    assert (sizeof (u32) == 4);

    if (argc < 2) {
        fprintf (stderr, "usage: md5sum filenames\n");
        fprintf (stderr, "       md5sum -c md5_file\n");
        fprintf (stderr, "       md5sum -s string\n");
        exit (1);
    }
    
    char sum_buf[33];
    if (!strcmp(argv[1], "-c")) {
        return md5_check(argv[2]);
    } else if (!strcmp(argv[1], "-s")) {
        if (argc<3) {
            fprintf (stderr, "md5sum -s string\n");
            exit(1);
        }
        md5_sum(argv[2], sum_buf, 1);
        printf("%s\n", sum_buf);
    } else {
        char *fname;
        char stri[64], strj[64];
        for (argc--, argv++; argc; argv++, argc--) {
            puts(*argv);
            sscanf(*argv,"%s\\%s", stri, strj);
            printf("%s/%s\n", stri, strj);
            if (md5_sum(*argv, sum_buf, 0)>0) continue;
            fname = NULL; //strrchr(*argv, '/');
            if (!fname) fname = *argv;
            else fname++;
            printf("%s  %s\n", sum_buf, fname);
        }
    }
    return 0;
}

