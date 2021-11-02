/*! \file parse_optn_flicker.h
    \brief Parse command line option for flicker_tst.
*/
#ifndef _PARSE_OPTN_SCNET_H_
#define _PARSE_OPTN_SCNET_H_

#include <stdint.h>
#include "parse_option.h"

enum SubCmdType { kBatchSet=1, kCmdTypeEnd };
//enum MainCmdOptnType {  kSetMacAddr=2, kSetParam, kGetParam, kMacPackPing, kUpApp, kUpBoot };

class ParseOptnScnet:public ParseOption {
public:
    ParseOptnScnet();
    ~ParseOptnScnet(){};

    //Accessors
    const uint8_t *bset_mac() { return &bset_par_.mac[0][0]; }
    uint8_t dbgcmd() { return dbgcmd_; }
    const uint8_t *scnet() { return bset_par_.scnet; }
    const char *filename_cfg() { return pfile_cfg_; }
    uint8_t force() { return force_; }
    const uint8_t *mac(int idx) { return pmac_[idx]; }
    uint8_t mac_cmd() { return mac_cmd_; }
    const uint32_t *trns_rto() { return &bset_par_.trns_rto[0][0]; }
    const float *c1c2() { return bset_par_.c1c2[0]; }
    uint16_t rllc() { return bset_par_.rllc; }
    
    //Mutators
    void clr_bset_par() { memset(&bset_par_, 0, sizeof(bset_par_)); }
protected:
    int HandleMain(int opt, char *arg);
    int HandleSubcmd(int opt, char *arg);
    int HandleOther(char *arg);
private:
    int HandleBset(int opt);
        
    struct BatchSetParam {
        uint8_t scnet[4];
        uint32_t trns_rto[4][2];
        uint8_t mac[4][3];
        float c1c2[3][2];
        uint16_t rllc;
    } bset_par_;
    
    void InitParam();
    char filename_cfg_[128];  //configure file name
    const char *pfile_cfg_;
    uint8_t mac_[2][6]; //Mac address.
    uint8_t *pmac_[2];  //pointer to mac_.
    uint8_t mac_cmd_;   //MAC communication command
    uint8_t force_;     //Forced to upgrade. 1=force
    uint8_t dbgcmd_;    //debug command.
};


#endif //_PARSE_OPTN_SCNET_H_