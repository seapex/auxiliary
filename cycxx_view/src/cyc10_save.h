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
#ifndef _CYC10_SAVE_H_
#define _CYC10_SAVE_H_

#include <stdint.h>
#include <sys/time.h>

struct CycxxFileHead {  //10cycle measurement value save file head
    uint16_t count;     //number of records
    uint8_t compress;   //Compression algorithm. 0=No compression, 1=zlib
    uint8_t version;
    uint8_t reserved[4];
};

static const int kMaxHarmNum = 50;  //Max orders of harmonic

struct MeasVCyc10LD {    //10cycles(200ms) interval measurement value for LD
    timeval time;

    float hrm_amp_u[3][kMaxHarmNum+1];  //Voltage harmonic amplitude. [0-2]:A-C. [0-50]:0-50. unit:V
    float ihrm_amp_u[3][kMaxHarmNum+1]; //Voltage interharmonic amplitude. [0-2]:A-C. [0-50]:0-50. unit:V
    float hrm_amp_i[3][kMaxHarmNum+1];  //Current harmonic amplitude. [0-2]:A-C. [0-50]:0-50. unit:A
    float ihrm_amp_i[3][kMaxHarmNum+1]; //Current interharmonic amplitude. [0-2]:A-C. [0-50]:0-50. unit:A
    float dev_u[3][2];  //deviation voltage. [0-2]:A-C/U+,U-,U+-. [0-1]:Uunder, Uover. unit:V.
    float pst[3];   //[0-2]:A-C/U+,U-,U+-. <0=invalid data
    float lfo_rms[3];//voltage low frequency oscillation rms. [0-2]:A-C/U+,U-. unit:V
    
    union {
        struct {
            float thd[3];       //Voltage THD. unit:%. [0-2]:A-C.
            float rms_u[2][3];  //Voltage rms. [0-1]:phase-neutral, phase-phase; [0-2]:A-C. unit:V
            float rms_i[3];     //Current rms. [0-2]:A-C. unit:A
            float seq[2][3];    //Sequence component. [0-1]:voltage,current. [0-2]:zero,positive,negative. unit:V/A
            float w[4];     //Active power. A-C,all. unit:kW
            float var[4];   //Reactive power. A-C,all. unit:kvar
            float pf[4];    //Power Factor. A-C,all.
            float freq;      //unit:Hz.
        } ac;
        struct {
            float rms_u[3];  //Voltage rms. [0-2]:U+,U-,U+-. unit:V
            float rms_i[3];  //Current rms. [0-2]:1,2,3i. unit:A
            float avg_u[3];  //Voltage average. [0-2]:U+,U-,U+-. unit:V
            float avg_i[3];  //Current average. [0-2]:1,2,3i. unit:A
            float seq[3];    //Voltage unbalance. [0-2]:U_b,U_u,ε; unit:V,V,%.
            float rpl[3][2]; //voltage ripple. [0-2]:U+,U-,U+-; [0-1]:coefficient, ratio. unit:%
            float w[3];      //Active power. 1,2,3i. unit:kW
        } dc;
    };
};
#endif //_CYC10_SAVE_H_
