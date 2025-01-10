/*! \file parse_optn_flicker.h
    \brief Parse command line option for flicker_tst.
*/
#ifndef _PARSE_OPTN_SCNET_H_
#define _PARSE_OPTN_SCNET_H_

#include <stdint.h>
#include "parse_option.h"

enum SubCmdType { kBatchSet=1, kCmdTypeEnd };
//enum MainCmdOptnType {  kSetMacAddr=2, kSetParam, kGetParam, kMacPackPing, kUpApp, kUpBoot };

struct BatchSetParam {
    uint8_t scnet[4];       //[0-3]:channel1-4. 0=needn't set, 1=need set
    uint32_t trns_rto[4][2];    //[0-3]:channel1-4; [0-1]:primary,secondary. unit:V/A. value=0:not set
    uint8_t mac[4][3];  //[0-3]:channel1-4; [0-2]:25-48bit of mac
    float c1c2[4][2];   //[0-2]:A-C; [0-1]:C1,C2. value=0:not set
    uint16_t rllc;      //Ratio of resistor impedance to capacitor reactance at 50Hz in RC parallel circute. value=0:not set
    uint8_t svtype[4];     //[0-3]:channel1-4. 0=primary, 1=secondary, other=not set
    uint16_t app_id;    //APPID. 0x4000~0x7FFF. 0x6B0E=boyuu 9-2, 0xCB0E=boyuu customized. value=0:not set
};

class ParseOptnScnet:public ParseOption {
public:
    ParseOptnScnet();
    ~ParseOptnScnet(){};

    //Accessors
    uint8_t dbgcmd() { return dbgcmd_; }
    const char *filename_cfg() { return pfile_cfg_; }
    uint8_t force() { return force_; }
    const uint8_t *mac(int idx) { return pmac_[idx]; }
    uint8_t mac_cmd() { return mac_cmd_; }
    const BatchSetParam *bset_par() { return &bset_par_; }
    
    //Mutators
    void clr_bset_par() { memset(&bset_par_, 0, sizeof(bset_par_)); }
protected:
    int HandleMain(int opt, char *arg);
    int HandleSubcmd(int opt, char *arg);
    int HandleOther(char *arg);
private:
    int HandleBset(int opt);        
    void InitParam();

    BatchSetParam bset_par_;    
    char filename_cfg_[128];  //configure file name
    const char *pfile_cfg_;
    uint8_t mac_[2][6]; //Mac address.
    uint8_t *pmac_[2];  //pointer to mac_.
    uint8_t mac_cmd_;   //MAC communication command
    uint8_t force_;     //Forced to upgrade. 1=force
    uint8_t dbgcmd_;    //debug command.
};


#endif //_PARSE_OPTN_SCNET_H_