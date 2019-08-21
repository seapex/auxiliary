/*! \digital_filter.h
    \brief digital filter.
    Copyright (c) 2019 Seapex
*/
#ifndef __DIGITAL_FILTER_H_
#define __DIGITAL_FILTER_H_

#include "math_ext.h"

enum kLowPassType {
    kButter2nd=1, kButter3rd
};
class DigitalFilter
{
    struct {
        float a[3];   //coefficient a
        float b[3];   //coefficient b
    } butter2_const_;    //2nd-order lowpass butterworth filter constant coefficient
    struct {
        float x[3];   //variable x
        float y[3];   //variable y
    } butter2_tmp_var_;  //2nd-order lowpass butterworth filter temp variable
    
    struct {
        float a[4];   //coefficient a
        float b[4];   //coefficient b
    } butter3_const_;    //3rd-order lowpass butterworth filter constant coefficient
    struct {
        float x[4];   //variable x
        float y[4];   //variable y
    } butter3_tmp_var_;  //3rdd-order lowpass butterworth filter temp variable

    inline float Butter2(float sv);
    inline float Butter3(float sv);
public:
    DigitalFilter();
    ~DigitalFilter(){};
    
    int IniLowPassPara(int type, int fc);
    float LowPass(float sv, int type=0);
    
    //mutators
};
#endif // __DIGITAL_FILTER_H_

