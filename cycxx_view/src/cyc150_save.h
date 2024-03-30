/*! 
文件保存路径: save/cyc150/Chnlx.  x 为通道编号, 1,2,...
文件名: Chnlx_yyyyMMdd_hhmm.150.  e.g. Chnl1_20191013_0907.150
文件格式:
    head:CycxxFileHead
    record1:MeasVChnl3s
    record2:MeasVChnl3s
    ...

字节序: Little endian
每个文件保存时长:1-60分钟可设
*/

#ifndef _CYC150_SAVE_H_
#define _CYC150_SAVE_H_

#include <stdint.h>

//static const int kMaxHarmNum = 50;  //Max orders of harmonic

struct MeasVChnl3s {    //3s(150cycles) interval measure value for channels
    uint32_t index; //Channels index
    time_t time;
    
    float hrm_amp[3][kMaxHarmNum+1];   //harmonic amplitude. [0-2]:A-C. [0-50]. unit:V/A
    float hrm_ang[3][kMaxHarmNum+1];   //harmonic phase angle. unit:degree. [0-2]:A-C. [1-50]:1-50 order.
    float ihrm_amp[3][kMaxHarmNum+1];  //interharmonic amplitude. [0-2]:A-C. [0-50]. unit:V/A
    float hr[3][kMaxHarmNum+1];    //harmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-1 no use, 2-50 order. 
    float ihr[3][kMaxHarmNum+1];   //interharmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-50 order.
    float rms[2][3];    //[0-2]:A-C or +LM/+L,-LM/-L,+L-L/M for DC. unit:V/A. [0-1]:phase-neutral, phase-phase; [1] not use for DC&current.
    float dev_u[3][2];  //deviation voltage,unit:V. [0-2]:A-C or U+,U-,U+- for DC.[0-1]:Urms-under, Urms-over. 

    union {
        struct {
            float seq[3];       //sequence component. [0-2]:zero,positive,negative; unit:V/A. [0-2]:U_b,U_u,ε; unit:V,V,% for DC voltage.
            float thd[3];       //THD. unit:%. [0-2]:A-C.
            float thdodd[3];    //odd THD. unit:%. [0-2]:A-C.
            float thdevn[3];    //even THD. unit:%. [0-2]:A-C.
        } ac;
        struct {
            float seq[3];       //unbalance. [0-2]:U_b,U_u,ε; unit:V,V,%. only for bipolar.
            float avg[3];       //average. [0-2]:+LM/+L,-LM/-L,+L-L/M; unit:V/A. 
            float rpl[3][2];    //ripple. [0-2]:+LM,-LM,+L-L; [0-1]:coefficient,ratio. unit:%
        } dc;
    };
};

struct LfoRms3s {   //3s(150cycles) interval LFO value for channels
    time_t time;
    float rms[3];   //Voltage low frequency oscillation rms. [0-2]:A-C/U+,U-. unit:V
};

#endif //_CYC150_SAVE_H_
