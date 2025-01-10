//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <math.h>

#include "flicker_filter.h"
#include "flicker_statis.h"
#include "time_cst.h"
#include "math_ext.h"
#include "parse_optn_flicker.h"

enum CycNumPer2Min { kCycle1, kCycle2, kCycle7,
    kkCycle39, kkCycle110, kCycle1620, kCycle4000
};

struct SquarePara {
    float freq; //square wave frequency. unit:Hz
    float amp;  //square wave amplitude. unit:%
};

const SquarePara sqr_par_[] = {
    {0.00833333, 2.724},    // 1/2min, 2.724
    {0.01666667, 2.211},    // 2/2min
    {0.05833333, 1.459},    // 7/2min
    {0.325,      0.906},    // 39/2min
    {0.91666667, 0.725},    // 110/2min
    {13.5,       0.402},    // 1620/2min
    {33.3333333, 2.4  },    // 4000/2min
};
int sqr_type_ = 0;
int smpl_rate_;
uint32_t pst_x_[4][3];   //sampling point count.[0-3]:4channel, [0-2]:A-C phase

FlickerStatis *flkr_statis_ = NULL;
/*!
    Input:  srt -- sampling rate type. 
*/
void Initialize(int srt)
{
    IniFilterPar(srt);
    switch (srt) {
        case PstSR400Hz:
            smpl_rate_ = 400;
            break;
        case PstSR800Hz:
            smpl_rate_ = 800;
            break;
        case PstSR1600Hz:
            smpl_rate_ = 1600;
            break;
        case PstSR2560Hz:
            smpl_rate_ = 2560;
            break;
        default:
            break;
    }
    int n = smpl_rate_/avg_num_flicker() * 600;
    if (flkr_statis_) delete flkr_statis_;
    flkr_statis_ = new FlickerStatis(n);
    memset(pst_x_, 0, sizeof(pst_x_));
}

/*!
Simulative Pst data wave generator

    Input:  num -- number of sampling point will be generated
            chl -- channel. 0-3
            phs -- phase. 0-2:A-C
            amp -- amplitude. unit:0.01V
            dc -- DC component. unit:0.01V
    Output: pbuf
*/
void PstWaveGen(int32_t *pbuf, int num, int chl, int phs, float amp, float dc)
{
    double pow_freq = 50;
    double m = sqr_par_[sqr_type_].amp*1/200;
    double f = sqr_par_[sqr_type_].freq;
    double smpl_t = smpl_rate_;
    smpl_t = 1/smpl_t;
    
    int x = pst_x_[chl][phs];
    for (int i=0; i<num; i++) {
        //pbuf[i] = amp*(1+m*SquareWave(2*f*x*smpl_t))*cos(2*kM_PI*pow_freq*x*smpl_t - 2*kM_PI*phs/3) + dc;
        pbuf[i] = (1+m*SquareWave(2*f*x*smpl_t+0.5))*(amp*cos(2*kM_PI*pow_freq*x*smpl_t - 2*kM_PI*phs/3) + dc);
        x++;
    }
    pst_x_[chl][phs] = x;
}

float pst_[10];

void TestStatis(int val)
{
    int n = flkr_statis_->tol_avrg();
    float *buf = new float[n];

    float avg[2], max[2], min[2];
    for (int j=0; j<2; j++) {
        if (j) {
            for (int i=0; i<n; i++) {
                buf[i] = n-i;
            }
        } else {
            for (int i=0; i<n; i++) {
                buf[i] = i;
            }
        }
        flkr_statis_->SetAvrgIns(buf, n, 0);

        avg[j]=0, max[j]=0, min[j]=9999999999;
        int loops = 10;
        for (int m=0; m<loops; m++) {
            StopWatch (0, 1, "pst statis:");
            for (int i=0; i<val; i++) {
                flkr_statis_->GetPst(0);
            }
            StopWatch (0, 0, NULL);
            float dur = stopwatch_dur(0);
            if (dur>max[j]) max[j] = dur;
            if (dur<min[j]) min[j] = dur;
            avg[j] += dur;
            msSleep(100);
        }
        printf("avg=%6.5f, max=%6.5f, min=%6.5f\n", avg[j]/loops, max[j], min[j]);
    }
    printf("avg=%6.5f, max=%6.5f, min=%6.5f\n", (avg[0]+avg[1])/2, (max[0]+max[1])/2, (min[0]+min[1])/2);
}

void TestSpeed(int val, int phs, int type)
{
    int nums = smpl_rate_ / 5;  //Number of sampling points processed per round.
    int rounds = 60 * 5;    //Number of rounds per minute
    sqr_type_ = type;
    
    int32_t *wave = new int32_t[nums];
    float *data = new float[nums];
    float *buf = new float[nums];
    PstWaveGen(wave, nums, 0, phs, 100000, 0);
    StopWatch (0, 1, "test speed:");
    for (int i=0; i<val; i++) {
        for (int j=0; j<rounds; j++) {
            for (int k=0; k<nums; k++) {
                data[k] = wave[k];  //0;
                data[k] /= 100;
            }
            int n = FlickerFilter(buf, wave, nums, 0, phs, 50);
            flkr_statis_->SetAvrgIns(buf, n, phs);
        }
        pst_[i%10] = flkr_statis_->GetPst(phs);
    }
    StopWatch (0, 0, NULL);
    delete [] wave;
    delete [] buf;
    delete [] data;
}

void TestAccuracy(int cnt, int phs, int type)
{
    int nums = smpl_rate_ / 5;  //Number of sampling points processed per round.
    int rounds = 60 * 5;    //Number of rounds per minute
    sqr_type_ = type;
    printf("nums=%d\n", nums);
    int32_t *wave = new int32_t[nums];
    //float *data = new float[nums];
    float *buf = new float[nums];
    StopWatch (0, 1, "accuracy:");
    set_block2(0);
    for (int i=0; i<cnt; i++) {
        if (i%10==0) pst_x_[0][phs] = 0;
        for (int j=0; j<rounds; j++) {
            PstWaveGen(wave, nums, 0, phs, 10000, 0000);
            /*for (int k=0; k<nums; k++) {
                data[k] = wave[k];
                data[k] /= 100;
            }*/
            int n = FlickerFilter(buf, wave, nums, 0, phs, 50);
            flkr_statis_->SetAvrgIns(buf, n, phs);
        }
        pst_[i%10] = flkr_statis_->GetPst(phs);
        printf("%6.3f \n", pst_[i%10]);
    }
    StopWatch (0, 0, NULL);
    delete [] wave;
    delete [] buf;
    //delete [] data;
}

int main (int argc, char *argv[])
{
    ParseOptnFlicker parse_opt;
    int cmd = parse_opt.Parse(argc, argv);
    if (cmd<kSpeedTst) return 0;
        
    Initialize(PstSR1600Hz);
    if (cmd==kSpeedTst) {
        for (int i=0; i<7; i++) {
            TestSpeed(12, 0, i);   //4channel * 3phase = 12
        }
    } else if (cmd==kAccuracyTst) {
        int cnt;
        for (int i=0; i<7; i++) {
        //for (int i=0; i<1; i++) {
            cnt = parse_opt.a_num();
            if (i==0) cnt += 10;
            TestAccuracy(cnt, 0, i);
        }
    } else if (cmd==kStatisTst) {
        TestStatis(12);
    }
}



