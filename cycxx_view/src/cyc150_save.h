/*! 
文件保存路径: save/cyc10/LDx.  x 为逻辑设备编号, 1,2,...
文件名: LDx_yyyyMMdd_hhmm.ten.  e.g. LD1_20191013_0907.ten
文件格式:
    head:CycxxFileHead
    record1:MeasVCyc10LD
    record2:MeasVCyc10LD
    ...

字节序: Little endian
每个文件保存时长:1-60分钟可设
*/
#ifndef _CYC150_SAVE_H_
#define _CYC150_SAVE_H_

#include <stdint.h>

//static const int kMaxHarmNum = 50;  //Max orders of harmonic
struct MeasVChnl3s {    //3s(150cycles) interval measure value for channel
    uint32_t index; //Channel index
    time_t time;
    
    float hrm_amp[3][kMaxHarmNum+1];   //harmonic amplitude. [0-2]:A-C. [0-50]. unit:V/A
    float hrm_ang[3][kMaxHarmNum+1];   //harmonic phase angle. unit:degree. [0-2]:A-C. [1-50]:1-50 order.
    float ihrm_amp[3][kMaxHarmNum+1];  //interharmonic amplitude. [0-2]:A-C. [0-50]. unit:V/A
    float hr[3][kMaxHarmNum+1];    //harmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-1 no use, 2-50 order. 
    float ihr[3][kMaxHarmNum+1];   //interharmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-50 order.
    float rms[2][3];    //[0-2]:A-C or +o,-o,+- for DC. unit:V/A. [0-1]:phase-neutral, phase-phase; [1] not use for DC.
    union {
        struct {
            float u_dev[3][2];  //voltage deviation. [0-2]:A-C. [0-1]:Urms-under, Urms-over. unit:V. [0-2]:+o,-o,+- for DC.
            float seq[3];   //sequence component. [0-2]:zero,positive,negative; unit:V/A. [0-2]:U_b,U_u,ε; unit:V,V,% for DC voltage.
            float thd[3];       //THD. unit:%. [0-2]:A-C.
            float thdodd[3];    //odd THD. unit:%. [0-2]:A-C.
            float thdevn[3];    //even THD. unit:%. [0-2]:A-C.
        } ac;
        struct {
            float avg[3];       //average. [0-2]:+o,-o,+-; unit:V/A.
            float u_dev[3][2];  //voltage deviation. [0-2]:+o,-o,+-. [0-1]:Urms-under, Urms-over. unit:V.
            float seq[3];       //unbalance. [0-2]:U_b,U_u,ε; unit:V,V,%.
            float rpl[3][2];    //ripple. [0-2]:+o,-o,+-; [0-1]:coefficient,ratio. unit:%
        } dc;
    };
};

#endif //_CYC150_SAVE_H_
