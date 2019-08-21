/*! \elec_cmpnnt.h
    \brief electronic component simulate.
    Copyright (c) 2019 Seapex
*/
#ifndef __ELEC_CMPNNT_H_
#define __ELEC_CMPNNT_H_

#include "math_ext.h"

class ElecComponent
{
    struct {
        float x[2];   //variable x
        float y[2];   //variable y
    } cap_tmp_var_;    //Capacitor temp variable
    int smpl_frq_;
    float cap_a0_;  //capacitor coefficient ak
    
public:
    ElecComponent(int frq);
    ~ElecComponent(){};
    
    float Capacitor(float sv);
    void SetCapPara(float cap);
    
    //mutators
};

#endif // __ELEC_CMPNNT_H_

