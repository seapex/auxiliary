//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>

//#include<time.h>
//#include<fcntl.h>
//#include <math.h>

#include "comtrade_test.h"
#include "generic.h"

/*bool IsInt(const char *str)
{
    for (int i=0; i<strlen(str); i++) {
        if(!isdigit(str[i])) return false;
    }
    return true;
}*/

int main (int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s cmd filename [sn] [count]\n", argv[0]);
        printf("\tcmd -- view,wav2rms\n");
        printf("\n");
        exit(1);
    }
    
    int cmd;
    if (!strcmp(argv[1], "view")) {
        cmd = 0;
    } else if (!strcmp(argv[1], "wav2rms")) {
        cmd = 1;
    } else {
        printf("Invalid cmd:%s\n", argv[1]);
        return 1;
    }
    int sn, count;
    switch(argc) {
        case 3:
            sn = 1;
            count = 0x7fffffff;
            break;
        case 4:
            if (IsInt(argv[3])) {
                sscanf(argv[3], "%d", &sn);
                count = 1;
            } else {
                printf("The sn must be a number!\n");
                return 1;
            }
            break;
        case 5:
        default:
            if (IsInt(argv[3]) && IsInt(argv[3])) {
                sscanf(argv[3], "%d", &sn);
                sscanf(argv[4], "%d", &count);
            } else {
                printf("The sn & count must be a number!\n");
                return 1;
            }
            break;
    }
    if(sn<1) {
        printf("sn must be > 0\n");
        return 1;
    }
    
    switch (cmd) {
        case 1:
            Wave2Rms(argv[2]);
            break;
        default:
            ShowData(argv[2], sn, count);
            break;
    }

}


