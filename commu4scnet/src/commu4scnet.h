/*! \file commu4scnet.h
    \brief communication tool for SCNet1xx.
*/
#ifndef _COMMU4SCNET_H_
#define _COMMU4SCNET_H_

enum MacCmd { kSetMac=7, kSetPar, kGetPar, kMacPing, kUpApp, kUpBoot, kDebug };

class CommuForScnet {
    int socket_fd_; // Socket file description
  	struct sockaddr_ll *sock_ll_;   // Link-Layer socket descriptor
    uint8_t src_mac_[6];
    uint16_t eth_type_; //shall be 0xB0E0
    
    typedef struct {
        uint8_t dev_model;  //Device model
        uint8_t res1[3];
        int16_t adc_dc[4];  //Backgroud DC component of ADC. [0-3]:A-C, N(only for current)
        int32_t corr[4];    //Accuracy correction factor. [0-3]:A-C, N(only for current). unit:1/10000
        uint32_t trns_rto[2];   //transformer ratio. [0-1]:PT1,PT2 unit:V or CT1, CT2 unit:A
        uint32_t cvt_c1c2[2];   //C1/C2 capacitance, unit:uF. Note! The actual data type is float.
        uint16_t cvt_prl_res;   //Ratio of resistor impedance to capacitor reactance at 50Hz in RC parallel circuit.
        uint8_t svtyp;      //SV type. 0=primary, 1=secondary
        uint8_t reserve[41];
        int32_t dbg32[4];
        int16_t debug[8];
        uint8_t res2[2];
        uint8_t ver[2][3]; //firmware version. [0-1]:App,Bootloader
    } Para4Scnet;

    typedef struct {
        uint16_t idx;       //packet index. 0~65534, 0xffff=last packet
        uint16_t size;      //the size of buf in bytes
        uint8_t reserved[4];    //reserved
        uint8_t buf[1024];
    } UpgradePack;
    
    typedef struct {
        uint8_t  desMac[6]; // Destination MAC address
        uint8_t  srcMac[6]; // Source MAC address
        uint16_t ethertype; // This value shall be 0xB0E0
        uint16_t length;    // length of [cmd,data]
        uint16_t cmd;       // refer to MacCmd
        uint8_t  res[2];
        union {
            struct {
                uint8_t dev_mac[6];
                uint8_t reserve[34];
            } mac;
            Para4Scnet para;
            UpgradePack pack;
       } data;
       uint32_t crc;    //reserved for crc32
    } CrtlMacFrame;     //CrtlMacFrame tx_buf_;
    uint8_t rx_buf_[1520];
    Para4Scnet par4scnet_;

    int OpenSocket();
    int GetLocalMac(uint8_t *mac, int *idx, const char *name);
    int CheckMacAddr(const uint8_t *mac);
    void SaveParam(const char *filename, Para4Scnet *par);
    int LoadParam(Para4Scnet *par, const char *filename);
    FILE *OpenUpFile(uint8_t *ver, const char *filename, int vdx, uint8_t fc=0);
    void SwapBytes(Para4Scnet *par, int type);
    
  public:
    CommuForScnet();
    ~CommuForScnet();
    
    int SetMacAddr(const uint8_t *mac);
    int SetParam(const char *filename, const uint8_t *mac);
    int GetParam(const char *filename, const uint8_t *mac);
    int MacPing(const uint8_t *mac, uint8_t echo=0);
    int Upgrade(const char *filename, const uint8_t *mac, uint16_t cmd, uint8_t fc=0);
    int BatchSet(const uint8_t *chnl, const uint32_t *ratio, const uint8_t *mac, const float *c1c2, uint16_t rllc);
    int DebugCmd(uint8_t cmdn, const uint8_t *mac);
};


#endif //_COMMU4SCNET_H_
