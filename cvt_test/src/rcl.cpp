//#include <fastmath67x.h>
#include <string.h>
#include <stdio.h>
#include "rcl.h"

/*!
    Input:  frq -- sampling frequency. unit:Hz
*/
RCL::RCL(int frq)
{
    smpl_frq_ = frq;
    memset(&cap_tmp_var_, 0, sizeof(cap_tmp_var_));
}

/*!
Set capacitor parameter
    
    Input:  cap -- capacitance of capacitor. unit:uF
*/
void RCL::SetCapPara(float cap)
{
    cap_a0_ = 1e6/(2*smpl_frq_*cap);  // T/(2C)
    //printf("cap_a0_=%f\n", cap_a0_);
}

/*!
capacitor difference equation
    
    Input:  sv -- sampling value
*/
float RCL::Capacitor(float sv)
{
/*    cap_tmp_var_.x[0] = cap_tmp_var_.x[1];
    cap_tmp_var_.x[1] = sv;
    cap_tmp_var_.y[0] = cap_tmp_var_.y[1];
    cap_tmp_var_.y[1] = cap_a0_*(cap_tmp_var_.x[1]+cap_tmp_var_.x[0]) + cap_tmp_var_.y[0];
    return cap_tmp_var_.y[1];
*/
    float d = cap_a0_*(sv+cap_tmp_var_.x[0]) + cap_tmp_var_.y[0];
    cap_tmp_var_.x[0] = sv; //cap_tmp_var_.x[0] = cap_tmp_var_.x[1]
    cap_tmp_var_.y[0] = d;  //cap_tmp_var_.y[0] = cap_tmp_var_.y[1];
    return d;
}

