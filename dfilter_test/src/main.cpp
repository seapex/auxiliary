//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

#include "digital_filter.h"
#include "time_cst.h"

static const float kSmpFrq = 12800;
const int kSimValNum = 2560; //Number of simulated signal values 
int sv_buf_[kSimValNum];
int sv_out_[kSimValNum];

/*!
Generate simulator signal

    Input:  freq -- The frequency of the simulated signal. unit:Hz
*/
void GenSignal(float freq)
{
    float amp = 1;

    double d1;
    for (int x=0; x<kSimValNum; x++) {
        d1 = amp*sqrt(2)*cos(2*kM_PI*freq*x/kSmpFrq);// + kM_PI/4);
        sv_buf_[x] = d1 * 1000;
    }
}

/*!
    Input:  freq -- The frequency of the simulated signal. unit:Hz
            type -- kDFilterType
            wc -- omega_c, Cut-off frequency. unit:Hz
            lh -- low or high pass. 0=low pass, 1=high pass
*/
void Test(float freq, int type, float wc, int lh)
{
    GenSignal(freq);
    DigitalFilter dfilter;
    dfilter.InitPara(type, wc, kSmpFrq, lh);
    
    int loops = 1;
    double d1, d2;
    StopWatch (0, 1, "dfilter.SignalPass()");
    for (int m=0; m<loops; m++) {
        for (int i=0; i<kSimValNum; i++) {
            d1 = sv_buf_[i];
            sv_out_[i] = dfilter.SignalPass(d1);
        }
    }
    StopWatch (0, 0, NULL);

    FILE *fp = fopen("filter_out.csv", "w");
    if (!fp) {
        printf("open file error\n");
        return;
    }
    for (int i=0; i<kSimValNum; i++) {
        fprintf(fp, "%d,%d\n", sv_buf_[i], sv_out_[i]);
    }
    fclose(fp);
}

int main (int argc, char *argv[])
{
    if (argc < 5) {
        printf("Usage: %s frq ords wc lh\n", argv[0]);
        printf("    frq: The frequency of the simulated signal, float. unit:Hz\n");
        printf("    ords: filter orders. 1-3\n");
        printf("    w_c: cut-off frequency, float. unit:Hz\n");
        printf("    lh: 0=low-pass, 1=high-pass\n");
        exit(1);
    }

    float frq;
    int type;
    float wc;
    int lh;
    sscanf(argv[1], "%g", &frq);
    sscanf(argv[2], "%d", &type);
    sscanf(argv[3], "%g", &wc);
    sscanf(argv[4], "%d", &lh);
    
    Test(frq, type, wc, lh);

    return 0;
}



