//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

#include "rcl.h"
#include "time_cst.h"

static const float kSmpFrq = 12800;
int sv_buf_[2560];
int sv_out_[2560];

/*!
Generate simulator signal
*/
void GenSignal(float pow_freq)
{
    float amp = 1;

    double d1;
    for (int x=0; x<2560; x++) {
        d1 = amp*sqrt(2)*cos(2*kM_PI*pow_freq*x/kSmpFrq);// + kM_PI/4);
        sv_buf_[x] = d1 * 1000;
    }
}

void Test(int val)
{
    GenSignal(val);
    RCL rcl(12800);
    rcl.SetCapPara(1);
    
    int loops = 10;
    double d1, d2;
    StopWatch (0, 1, NULL);
    for (int m=0; m<loops; m++) {
        for (int j=0; j<12; j++) {
            for (int i=0; i<2560; i++) {
                d1 = sv_buf_[i];
                sv_out_[i] = rcl.Capacitor(d1);
            }
        }
    }
    StopWatch (0, 0, "capacitor");
/*
    printf("sv_buf_ = \n");
    for (int i=0; i<256; i++) {
        printf("%d ", sv_buf_[i]);
    }
    printf("\n\n");
    for (int i=0; i<256; i++) {
        printf("%d ", sv_out_[i]);
    }
    printf("\n\n");
*/
}

int main (int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s power_freq\n", argv[0]);
        exit(1);
    }

    int num;
    sscanf(argv[1], "%d", &num);
    Test(num);

    return 0;
}



