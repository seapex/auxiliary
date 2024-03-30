/*! \digital_filter.h
    \brief digital filter.
    Copyright (c) 2019 Seapex
*/
#ifndef __DIGITAL_FILTER_H_
#define __DIGITAL_FILTER_H_

#include "math_ext.h"

enum kDFilterType {
    kButter1st=1, kButter2nd, kButter3rd, kButter4th, kButter5th
};

class DigitalFilter
{
    void RefreshPara(int idx, int pari, float parf);

    struct {
        double a[8]; //coefficient a0~
        double b[8]; //coefficient b0~
    } cffcnt_;  //filter coefficient
    
    struct {
        double x[8]; //variable x
        double y[8]; //variable y
    } tmp_var_; //filter temporary variable

    int fltr_t_;    //filter type. kDFilterType
    float omega_c_; //cut-off angular frequency. unit:Hz
    int smpl_rate_; //sampling rate. unit:Hz
    int low_high_;  //low or high pass. 0=low pass, 1=high pass

    int tmpv_sz_;    //size of type of tmp_var_. 
public:
    DigitalFilter();
    ~DigitalFilter(){};
    
    int InitPara(int type, float wc, int rate, int lh);
    float SignalPass(float sv);
    
    //Accessors
    //void print_para() { printf("%d,%g,%d,%d\n", fltr_t_, omega_c_, smpl_rate_, low_high_); printf("a[0-1]=%g,%g; b[1]=%g\n", cffcnt_.a[0], cffcnt_.a[1], cffcnt_.b[1]); }
    //Mutators
    void set_fltr_t(int val) { if (val==fltr_t_) return; RefreshPara(0, val, 0); }
    void set_omega_c(float val) { if (val==omega_c_) return; RefreshPara(1, 0, val); }
    void set_smpl_rate(int val) { if (val==smpl_rate_) return; RefreshPara(2, val, 0); }
    void set_low_high(int val) { if (val==low_high_) return; RefreshPara(3, val, 0); }
};

#endif // __DIGITAL_FILTER_H_

