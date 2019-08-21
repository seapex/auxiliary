//#include <fastmath67x.h>
#include <string.h>
#include <stdio.h>
#include "digital_filter.h"

/*!
    Input:  frq -- sampling frequency. unit:Hz
*/
DigitalFilter::DigitalFilter()
{
    IniLowPassPara(kButter2nd, 75);
    IniLowPassPara(kButter3rd, 75);
}

/*!
Initialize low pass filter parameter

    Input:  type -- kLowPassType
            fc -- Cut-off frequency. unit:Hz
    Return: 0=success, 1=type error, 2=fc error
*/
int DigitalFilter::IniLowPassPara(int type, int fc)
{
    switch (type) {
        case kButter2nd:
            memset(&butter2_tmp_var_, 0, sizeof(butter2_tmp_var_));
            switch (fc) {
                case 75:
                    butter2_const_.a[0] = butter2_const_.a[2] = 0.00033014;
                    butter2_const_.a[1] = 0.00066028;
                    butter2_const_.b[1] = -1.948;
                    butter2_const_.b[2] = 0.9493;
                    break;
                default:
                    return 2;
            }
            break;
        case kButter3rd:
            memset(&butter3_tmp_var_, 0, sizeof(butter3_tmp_var_));
            switch (fc) {
                case 75:
                    butter3_const_.a[0] = butter3_const_.a[3] = 0.00000601195;
                    butter3_const_.a[1] = butter3_const_.a[2] = 0.000018036;
                    butter3_const_.b[1] = -2.92638;
                    butter3_const_.b[2] = 2.85545;
                    butter3_const_.b[3] = -0.929018;
                    break;
                default:
                    return 2;
            }
            break;
        default:
            return 1;
    }
    return 0;
}

/*!
Low pass filter
    
    Input:  sv -- sampling value
            type -- kLowPassType
*/
float DigitalFilter::LowPass(float sv, int type)
{
    float val;
    switch (type) {
        case kButter2nd:
            val = Butter2(sv);
            break;
        case kButter3rd:
            val = Butter3(sv);
            break;
        default:
            val = sv;
            break;
    }
    return val;
}

/*!
Butterworth 2nd-order filter
    
    Input:  sv -- sampling value
*/
float DigitalFilter::Butter2(float sv)
{
    butter2_tmp_var_.x[0] = butter2_tmp_var_.x[1];
    butter2_tmp_var_.x[1] = butter2_tmp_var_.x[2];
    butter2_tmp_var_.x[2] = sv;
    butter2_tmp_var_.y[0] = butter2_tmp_var_.y[1];
    butter2_tmp_var_.y[1] = butter2_tmp_var_.y[2];
    butter2_tmp_var_.y[2] = butter2_const_.a[0]*butter2_tmp_var_.x[2] 
                          + butter2_const_.a[1]*butter2_tmp_var_.x[1] 
                          + butter2_const_.a[2]*butter2_tmp_var_.x[0]
                          - butter2_const_.b[1]*butter2_tmp_var_.y[1]
                          - butter2_const_.b[2]*butter2_tmp_var_.y[0];
    return butter2_tmp_var_.y[2];
}

/*!
Butterworth 3rd-order filter
    
    Input:  sv -- sampling value
*/
float DigitalFilter::Butter3(float sv)
{
    butter3_tmp_var_.x[0] = butter3_tmp_var_.x[1];
    butter3_tmp_var_.x[1] = butter3_tmp_var_.x[2];
    butter3_tmp_var_.x[2] = butter3_tmp_var_.x[3];
    butter3_tmp_var_.x[3] = sv;
    butter3_tmp_var_.y[0] = butter3_tmp_var_.y[1];
    butter3_tmp_var_.y[1] = butter3_tmp_var_.y[2];
    butter3_tmp_var_.y[2] = butter3_tmp_var_.y[3];
    butter3_tmp_var_.y[3] = butter3_const_.a[0]*butter3_tmp_var_.x[3] 
                          + butter3_const_.a[1]*butter3_tmp_var_.x[2] 
                          + butter3_const_.a[2]*butter3_tmp_var_.x[1]
                          + butter3_const_.a[3]*butter3_tmp_var_.x[0]
                          - butter3_const_.b[1]*butter3_tmp_var_.y[2]
                          - butter3_const_.b[2]*butter3_tmp_var_.y[1]
                          - butter3_const_.b[3]*butter3_tmp_var_.y[0];
    return butter3_tmp_var_.y[3];
}
