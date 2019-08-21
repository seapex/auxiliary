/*! \rcl.h
    \brief RCL Resistor, Capacitor & Inductor.
    Copyright (c) 2019 Seapex
*/
#ifndef __RCL_H_
#define __RCL_H_

#include "math_ext.h"

class RCL
{
    struct {
        float x[2];   //variable x
        float y[2];   //variable y
    } cap_tmp_var_;    //Capacitor temp variable
    float cap_a0_;  //capacitor coefficient ak

    int smpl_frq_;  //sampling frequency. unit:Hz
    
public:
    RCL(int frq);
    ~RCL(){};
    
    float Capacitor(float sv);
    void SetCapPara(float cap);
    
    //mutators
};

#endif // __RCL_H_

