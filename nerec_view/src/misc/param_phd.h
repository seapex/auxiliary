/*! \file param_phd.h
    \brief Physical device parameter.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PARAM_PHY_H_
#define _PARAM_PHY_H_

#include <stdint.h>

static const int kChannelTol = 4;   //Total number of channel
static const int kSGTol = 4;   //Total number of setting group
static const int kPstSmpNum = 320;  //sample data number for Pst per 0.2s(≈10cycle)
static const int kMaxHarmNum = 50;  //Max orders of harmonic
static const int kDefaultSmpFreq = 12800;  //Default sampling frequency
enum kSteadyAlmEna {kAlmHarmEna, kAlmFreqEna, kAlmPstEna, kAlmUnblcEna, kAlmVoltdvEna};

union LDChnnlInfo {
    uint8_t idx[2]; //channel index. [0-1]:voltage/current. range:1~kChannelTol, 0=nil
    uint16_t stat;  //channel state. 0=nil
};

struct ScheduleRecWv {   //Scheduled wave recording parameter
    uint8_t ena_ld; //bit0:timer enable. 0=disable, 1=enable; bit1-7:Logical device
    uint8_t repeat; //repeat interval. 0=no repeat, 1=day, 2=week, 3=month
    uint32_t time;  //trigger time
    uint8_t resv[2];   //reserved
};

struct SCNetParam {
    uint8_t mac[3];     //scnet mac address
    uint8_t svtyp;      //SV type. 0=primary, 1=secondary
    uint8_t ouit;       //OUI type. 0=boyuu, 1=NR Electric
    uint8_t oui[3];     //OUI mac
    uint8_t model;
    uint8_t resv[11];
};

enum NEDataType { kNEDataFreq, kNEDataHarm, kNEDataUnblc, kNEDataUdev, kNEDataPst };
struct NewEnergyParam {
    uint8_t ena;        //new energy enable. 0=disable, 1=enable
    uint8_t num[2];     //Number of 10-minute data stores. [0-1]:running, background
    uint8_t spc[5];     //save space. uint:s. 113=0.2s. [0-4]:freq, harm, unbalance, voltage deviation, reserved
    int32_t pwr;        //Rated power. unit:kW
};

enum PQParaType { kPQParaFreq, kPQParaHrm, kPQParaUnblc,
    kPQParaUdev,     //include Urms & Irms
    kPQParaEnd=7 };

struct ParamPhD {   //Physical Device parameter
    uint16_t device_model;  //4002=PQNet300D
    char device_id[16];     //PQNet300Dnnnnnn. nnnnnn:000001~999999
    uint16_t stts_spc[kPQParaEnd];   //statistic space. unit:s
    uint8_t ip[2][4];       //ip address. [0-1]:eth0,eth1
    uint16_t port[2];       //port number. [0-1]:eth0,eth1
    uint8_t mask[2][4];     //netmask. [0-1]:eth0,eth1
    uint8_t mac[2][4];      //mac address. [0-1]:eth0,eth1
    uint8_t gate[2][4];     //gateway. [0-1]:eth0,eth1
    uint8_t frqm_spc;       //frequency measure space. 0=1s, 1=3s, 2=10s
    uint8_t hrm_type;       //harmonic value type. 0=Subgroup, 1=Component, 2=Group
    uint8_t ihrm_type;      //interharmonic value type. 0=Subgroup, 1=Group, 2=1/2
    uint8_t man_rec_en;	    //manual record wave enable
    uint8_t evnt_tol;       //Rapid Change Event record total number.
    uint8_t cmtrd_sv_meth;  //Comtrade file save method. 0=COMTRADE, 1=<IED+LDName>/COMTRADE, 2=COMTRADE/<LDName>, 3=COMTRADE/<LDNum>, 4=Jangsu, 5=Shanghai
    uint8_t stdy_trgr_ena;  //Steady state index overrun event triggers recording wave. 0=disable,1=enable.
                            //bit0:total; bit1:freq; bit2:harm volt; bit3:harm current; 
                            //bit4:volt unbalance; bit5:current negtive; bit6:volt deviation
    uint8_t synctm_type;    //Synchronize time type. 0=gps+ntp, 1=ntp
    uint16_t evnt_max_dur;  //Transient event maximum record duration. unit:s
    uint16_t evnt_end_num;  //The number of cycles recorded after the event
    int8_t sys_zone;        //system timezone. ranges:[-12,12],unit:hours. 
    uint8_t gps_zone_t;     //gps timezone type. 0=utc,1=local
    uint8_t ntp_ip[4];      //ip address of ntp server.
    uint8_t commu_num[2];   //Serial port communication number. [0-1]:COM1, COM2
    uint32_t baud[2];       //Serial port baud rate. [0-1]:COM1, COM2
    uint8_t prtcl[2][2];    //Serial port protocol. [0-1]:COM1, COM2; [0-1]:phy protocol, app protocal
	uint8_t alarm_ena;		//steady data alarm enable. bit0-4:harm,freq,pst,unbalance,voltdev
	uint8_t cvt_ena;        //cvt function enable. 0=disable, 1=PQNet202CVT, 2=PQNet101CVT
	uint8_t cvt_cc;         //obsolete!! current channel index. 0=none, 1=channel1, 2=channel2...
	uint8_t cvt_chnl[3];    //CVT channel config. [0-2]:A-C, 0=none, 1=channel1, 2=channel2...
	uint8_t cvt_cnct[3][2]; //C1/C2 connect config. [0-2]:A-C; [0-1]:C1/C2, 0=A,1=B,2=C
	float cvt_c1c2[3][2];   //C1/C2 capacitance, unit:uF. [0-2]:A-C; [0-1]:C1/C2
    uint8_t intrplt_t[2];   //interpolate type. [0-1]:type,boundary. type:0=linear,1=quadratic,2=cubic; boundary:0=natural,1=clamped.
    uint8_t softver[3];     //dpqnet_mn version
    uint8_t cyc10_ena;      //10cycle measurement enable. 0=disable, 1=enable
    uint8_t cyc10_sz;       //10cycle measurement value savefile size. unit:minute
    uint8_t cyc150_ena;     //150cycle measurement enable. 0=disable, 1=enable
    uint8_t cyc150_sz;      //150cycle measurement value savefile size. unit:minute
    uint8_t sig_simu;       //signal simulation. 0=close, 1=steady, 2=steady+transient, 3=steady+RVC.
    uint8_t input0_thr[2];  //Zero input threshold. 0=voltage, 1=current. unit:V/mA, secondary
	float cvt_r1r2[3][2];   //C1/C2 resistance, unit:O. [0-2]:A-C; [0-1]:C1/C2
    uint16_t time_diff;     //Maximum time difference allowed for security check. unit:s
    uint8_t security_ena;   //Enforce remote access security checks. 0=no, 1=yes
    uint8_t pst_fltr;       //Pst prefilter. 0=off, 1=on
    uint8_t powfrq_fltr;    //Power frequency low-pass filter. 0=off, 1=2nd-order, 2=3rd-order
    uint8_t iover[3];       //io board firmware version
    uint32_t ver_time;      //dpqnet_mn version update time
	uint16_t audtlog_sz;    //Audit log record file size. unit:kByte
	char ver_61850[24];     //61850 service version information
	int16_t pr_dly_cnt;     //Fine-tune the time of the rising edge of the IRIG-B Pr code
	int8_t b_dbg_mode;      //IRIG-B output debug mode
    uint8_t rvc_wav_ena;    //RVC record wave enable. 0=disable, 1=enable
    uint8_t dc_evnt_val;    //DC transient event value type. 0=20ms rms, 1=20ms average, 2=10ms average
    uint8_t st_tm_alg;      //Statistic data timestamp alignment. 0=no, 1=yes
    ScheduleRecWv schdl_recwv[4];   //Scheduled wave recording parameter
    uint16_t cvt_prl_res;   //Ratio of resistor impedance to capacitor reactance at 50Hz in RC parallel circute.
    uint8_t cvt_dbg;        //Show CVT debug information. 0-2:A-C,3=Off
    uint8_t eth_speed;      //eth1 speed. 0=auto, 1=100M
    uint16_t debug;         //bit0-2:61850 realtime,ReadSV(),HandleSV(); bit3:synchronize parameter to scnet104. 0=off,1=on
    uint8_t hrm_ns;         //harmonic noise suppression. 0=off,1=on
    uint8_t res[69];        //reserved
    NewEnergyParam ne_par;  //New energy test parameter
    SCNetParam scnet_par[8];//SCNetxx parameter
};  //sizeof(ParamPhD) = 512

#endif //_PARAM_PHY_H_
