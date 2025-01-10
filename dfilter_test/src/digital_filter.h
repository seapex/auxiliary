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
    void RefreshPara(int idx, int par);

    struct {
        double a[8]; //coefficient a0~
        double b[8]; //coefficient b0~
    } cffcnt_;  //filter coefficient
    
    struct {
        double x[8]; //variable x
        double y[8]; //variable y
    } tmp_var_; //filter temporary variable

    int fltr_t_;    //filter type. kDFilterType
    int freq_c_; //cut-off frequency. unit:0.01Hz
    int smpl_rate_; //sampling rate. unit:Hz
    int low_high_;  //low or high pass. 0=low pass, 1=high pass

    int tmpv_sz_;    //size of type of tmp_var_. 
public:
    DigitalFilter();
    ~DigitalFilter(){};
    
    int InitPara(int type, int wc, int rate, int lh);
    float SignalPass(float sv);
    
    //Accessors
    //void print_para() { printf("%d,%g,%d,%d\n", fltr_t_, freq_c_, smpl_rate_, low_high_); printf("a[0-1]=%g,%g; b[1]=%g\n", cffcnt_.a[0], cffcnt_.a[1], cffcnt_.b[1]); }
    //Mutators
    void set_fltr_t(int val) { if (val==fltr_t_) return; RefreshPara(0, val); }
    void set_freq_c(int val) { if (val==freq_c_) return; RefreshPara(1, val); }
    void set_smpl_rate(int val) { if (val==smpl_rate_) return; RefreshPara(2, val); }
    void set_low_high(int val) { if (val==low_high_) return; RefreshPara(3, val); }
};

#endif // __DIGITAL_FILTER_H_

