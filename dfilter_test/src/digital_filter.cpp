//#include <fastmath67x.h>
#include <string.h>
#include <stdio.h>
#include "digital_filter.h"

/*!
    Input:  frq -- sampling frequency. unit:Hz
*/
DigitalFilter::DigitalFilter()
{
    fltr_t_ = 0;
    omega_c_ = 1;
    smpl_rate_ = 12800;
    low_high_ = 0;

    memset(&tmp_var_, 0, sizeof(tmp_var_));
    tmpv_sz_ = sizeof(tmp_var_)/16;
}

/*!
Initialize filter parameter

    Input:  type -- kDFilterType
            wc -- omega_c, Cut-off frequency. unit:Hz
            rate -- T=1/rate, sampling rate. unit:Hz
            lh -- low or high pass. 0=low pass, 1=high pass
    Return: 0=success, 1=cancel, 2=type error, 3=rate error
*/
int DigitalFilter::InitPara(int type, float wc, int rate, int lh)
{
    if (rate<=0) return 3;

    int up = 0;
    if (fltr_t_ != type) up++;
    if (omega_c_ != wc) up++;
    if (smpl_rate_ != rate) up++;
    if (low_high_ != lh) up++;
    if (!up) return 1;

    fltr_t_ = type;
    omega_c_ = wc;
    smpl_rate_ = rate;
    low_high_ = lh;

    //memset(&tmp_var_, 0, sizeof(tmp_var_));
    struct {
        double a[8]; //coefficient a'0~
        double b[8]; //coefficient b'0~
    } tmpcf;    //temporary coefficient
    double Tw = 2.0*kM_PI*omega_c_/rate;  //T*omega_c
    switch (fltr_t_) {
        case kButter1st:
            if (lh) {
                tmpcf.a[0] = 2;
                tmpcf.a[1] = -2;
            } else {
                tmpcf.a[0] = tmpcf.a[1] = Tw;
            }
            tmpcf.b[0] = Tw + 2;
            tmpcf.b[1] = Tw - 2;
            break;
        case kButter2nd:
            if (lh) {
                tmpcf.a[0] = tmpcf.a[2] = 10000;
                tmpcf.a[1] = -20000;
            } else {
                tmpcf.a[0] = tmpcf.a[2] = 2500*Tw*Tw;
                tmpcf.a[1] = tmpcf.a[0]*2;
            }
            tmpcf.b[0] = 2500*Tw*Tw + 7071*Tw + 10000;
            tmpcf.b[1] = 5000*Tw*Tw - 20000;
            tmpcf.b[2] = 2500*Tw*Tw - 7071*Tw + 10000;
            break;
        case kButter3rd:
            if (lh) {
                tmpcf.a[0] = 8;
                tmpcf.a[1] = -24;
                tmpcf.a[2] = 24;
                tmpcf.a[3] = -8;
            } else {
                tmpcf.a[0] = tmpcf.a[3] = Tw*Tw*Tw;
                tmpcf.a[1] = tmpcf.a[2] = 3*tmpcf.a[0];
            }
            tmpcf.b[0] = Tw*Tw*Tw + 4*Tw*Tw + 8*Tw + 8;
            tmpcf.b[1] = 3*Tw*Tw*Tw + 4*Tw*Tw - 8*Tw - 24;
            tmpcf.b[2] = 3*Tw*Tw*Tw - 4*Tw*Tw - 8*Tw + 24;
            tmpcf.b[3] = Tw*Tw*Tw - 4*Tw*Tw + 8*Tw - 8;
            break;
        default:
            return 2;
    }
    //printf("fltr_t_=%d, lh=%d, wc=%g,smpl_rate_=%d\n", fltr_t_, lh, omega_c_, smpl_rate_);
    for (int i=0; i<=fltr_t_; i++) {
        cffcnt_.a[i] = tmpcf.a[i]/tmpcf.b[0];
        cffcnt_.b[i] = tmpcf.b[i]/tmpcf.b[0];
        //printf("a[%d]=%g, b[%d]=%g\n", i, cffcnt_.a[i], i, cffcnt_.b[i]);
    }

    return 0;
}

/*!
Refresh filter parameter

    Input:  idx -- parameter index. 0-3:fltr_t_, omega_c_, smpl_rate_, low_high_
            pari -- integer parameter
            parf -- float parameter
*/
void DigitalFilter::RefreshPara(int idx, int pari, float parf)
{
    int type = fltr_t_;
    float wc = omega_c_;
    int rate = smpl_rate_;
    int lh = low_high_;
    switch (idx) {
        case 0:
            type = pari;
            break;
        case 1:
            wc = parf;
            break;
        case 2:
            rate = pari;
            break;
        case 3:
            lh = pari;
            break;
    }
    InitPara(type, wc, rate, lh);
}

/*!
Filtering of passed signals. 
    
    Input:  sv -- sampling value
*/
float DigitalFilter::SignalPass(float sv)
{
    memmove(&tmp_var_.x[0], &tmp_var_.x[1], tmpv_sz_*fltr_t_);
    tmp_var_.x[fltr_t_] = sv;
    memmove(&tmp_var_.y[0], &tmp_var_.y[1], tmpv_sz_*fltr_t_);

    switch (fltr_t_) {
        case kButter1st:
            tmp_var_.y[1] = cffcnt_.a[0]*tmp_var_.x[1] 
                          + cffcnt_.a[1]*tmp_var_.x[0]
                          - cffcnt_.b[1]*tmp_var_.y[0];
            break;
        case kButter2nd:
            tmp_var_.y[2] = cffcnt_.a[0]*tmp_var_.x[2] 
                          + cffcnt_.a[1]*tmp_var_.x[1] 
                          + cffcnt_.a[2]*tmp_var_.x[0]
                          - cffcnt_.b[1]*tmp_var_.y[1]
                          - cffcnt_.b[2]*tmp_var_.y[0];
            break;
        case kButter3rd:
            tmp_var_.y[3] = cffcnt_.a[0]*tmp_var_.x[3]
                          + cffcnt_.a[1]*tmp_var_.x[2]
                          + cffcnt_.a[2]*tmp_var_.x[1]
                          + cffcnt_.a[3]*tmp_var_.x[0]
                          - cffcnt_.b[1]*tmp_var_.y[2]
                          - cffcnt_.b[2]*tmp_var_.y[1]
                          - cffcnt_.b[3]*tmp_var_.y[0];
            break;
        case kButter4th:
            tmp_var_.y[4] = cffcnt_.a[0]*tmp_var_.x[4]
                          + cffcnt_.a[1]*tmp_var_.x[3]
                          + cffcnt_.a[2]*tmp_var_.x[2]
                          + cffcnt_.a[3]*tmp_var_.x[1]
                          + cffcnt_.a[4]*tmp_var_.x[0]
                          - cffcnt_.b[1]*tmp_var_.y[3]
                          - cffcnt_.b[2]*tmp_var_.y[2]
                          - cffcnt_.b[3]*tmp_var_.y[1]
                          - cffcnt_.b[4]*tmp_var_.y[0];
            break;
        case kButter5th:
            tmp_var_.y[5] = cffcnt_.a[0]*tmp_var_.x[5]
                          + cffcnt_.a[1]*tmp_var_.x[4]
                          + cffcnt_.a[2]*tmp_var_.x[3]
                          + cffcnt_.a[3]*tmp_var_.x[2]
                          + cffcnt_.a[4]*tmp_var_.x[1]
                          + cffcnt_.a[5]*tmp_var_.x[0]
                          - cffcnt_.b[1]*tmp_var_.y[4]
                          - cffcnt_.b[2]*tmp_var_.y[3]
                          - cffcnt_.b[3]*tmp_var_.y[2]
                          - cffcnt_.b[4]*tmp_var_.y[1]
                          - cffcnt_.b[5]*tmp_var_.y[0];
            break;
        default:
            tmp_var_.y[fltr_t_] = sv;
            break;
    }
    return tmp_var_.y[fltr_t_];
}

