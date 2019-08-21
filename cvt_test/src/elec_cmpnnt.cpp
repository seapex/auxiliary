//#include <fastmath67x.h>
#include <string.h>
#include <stdio.h>
#include "elec_cmpnnt.h"

/*!
    Input:  frq -- sampling frequency. unit:Hz
*/
ElecComponent::ElecComponent(int frq)
{
    smpl_frq_ = frq;
    memset(&cap_tmp_var_, 0, sizeof(cap_tmp_var_));
}

/*!
Set capacitor parameter
    
    Input:  cap -- capacitance of capacitor. unit:uF
*/
void ElecComponent::SetCapPara(float cap)
{
    cap_a0_ = 1e6/(2*smpl_frq_*cap);  // T/(2C)
    printf("cap_a0_=%f\n", cap_a0_);
}

/*!
capacitor difference equation
    
    Input:  sv -- sampling value
*/
float ElecComponent::Capacitor(float sv)
{
    cap_tmp_var_.x[0] = cap_tmp_var_.x[1];
    cap_tmp_var_.x[1] = sv;
    cap_tmp_var_.y[0] = cap_tmp_var_.y[1];
    cap_tmp_var_.y[1] = cap_a0_*(cap_tmp_var_.x[1]+cap_tmp_var_.x[0]) + cap_tmp_var_.y[0];
    return cap_tmp_var_.y[1];
}

