#ifndef _PARAM_CFG_H_
#define _PARAM_CFG_H_
#include <stdint.h>

typedef struct {
    uint8_t par1;
    uint16_t par2;
    float par3;
    char par4[32];
    uint16_t par5;
} MultiMixPar;

typedef struct {
    char *topic;
    uint8_t option;
} SubPayload;

const int kMaxSubPayNum = 5;
typedef struct {
    char hostname[64];  //hostname or IP
    char name[64];      //port number or local filename
    char dev_id[24];    //device ID. //PQNet300Dnnnnnn. nnnnnn:000001~999999
    SubPayload sbpay[kMaxSubPayNum];
    char mltstr[4][32]; //cannot contain null value
    MultiMixPar mmx1;   //cannot contain null value
    MultiMixPar mmx2;   //can contains null values
} ConfigPara;

class ParamCfg {
    void DefaultPara();
    void SaveParam();
    void Show4Debug();
    void CleanCfgpar();

    ConfigPara cfg_para_;
    
public:
    ParamCfg();
    ~ParamCfg();

    void LoadParam();
    int SubPayload(char **topics, uint8_t *options);
    
    //Accessors
    char *hostname() { return cfg_para_.hostname; }
    char *name() { return cfg_para_.name; }
    const char *dev_id() { return cfg_para_.dev_id; }
};

ParamCfg & param_cfg();

#endif //_PARAM_CFG_H_