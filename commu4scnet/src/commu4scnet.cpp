#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <zlib.h>
#include <fcntl.h> 

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "commu4scnet.h"
#include "misc.h"

static const uint8_t kGroupMac[6] = {0xB1, 0xE0, 0x14, 0x03, 0x08, 0x01};
static const uint8_t kBoyuuOUI[3] = {0xB0, 0xE0, 0x14};

enum DeviceModel {PQNet103D, PQNet204D, PQNet202CVT, PQNet202E1, PQNet202E2, PQNet202Ex, PQNetxxx, DM_NoCard};
enum ParamType {kFirmVer, kDevModel, kADCBkgrdDC, kCorrFactor, kPT_CT};
static const char *kParamName[] = {"FirmwareVer", "DeviceModel", "ADCBackgroudDC[4]", "CorrectFactor[4]", "PT_CT[2]"};
static const char *DeviceModelStr[] = {"PQNet103D", "PQNet204D", "PQNet202CVT", "PQNet202E1", "PQNet202E2", "PQNet202Ex", "PQNetxxx", "DM_NoCard"};

CommuForScnet::CommuForScnet()
{
    if (OpenSocket()<0) exit(-1);
    
    sock_ll_ = new sockaddr_ll;
  	memset(sock_ll_, 0, sizeof(sockaddr_ll));
    if (GetLocalMac(src_mac_, &sock_ll_->sll_ifindex, "eth1")<0) {    //enp0s3
        exit(-1);
    }
    eth_type_ = 0xB0E0;
    
    //printf("sizeof(Para4Scnet)=%d\n", sizeof(Para4Scnet));
    //printf("sizeof(CrtlMacFrame)=%d\n", sizeof(CrtlMacFrame));
}

CommuForScnet::~CommuForScnet()
{
    delete sock_ll_;
    if (socket_fd_>0) {
        close (socket_fd_);
    }
    
}

/*!
Get the local MAC address & interface index.

    Input:  name -- Network card name
    Output: mac -- source mac address
            idx -- network card interface index
    Return:  <0=failure
*/
int CommuForScnet::GetLocalMac(uint8_t *mac, int *idx, const char *name)
{
	struct ifreq ethreq;
    strncpy(ethreq.ifr_name, name, IFNAMSIZ);

	int ret;
	do {
    	ret = ioctl(socket_fd_, SIOCGIFHWADDR, &ethreq); //Get the MAC address of the network port
    	if (ret < 0) break;
        memcpy(mac, ethreq.ifr_hwaddr.sa_data, 6);

	    ret = ioctl(socket_fd_, SIOCGIFINDEX, &ethreq);  //Get the device index of the network port
    	if (ret < 0) break;
    	*idx = ethreq.ifr_ifindex;
	} while(0);
	if(ret < 0) {
		perror("ioctl");
	}
	return ret;
}

inline int CommuForScnet::CheckMacAddr(const uint8_t *mac)
{
    if (memcmp(mac, kBoyuuOUI, 3)) {
        printf("Invalid MAC address! %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return 1;
    } else {
        return 0;
    }
}

/*!
Set MAC address to scnet

    Input:  mac -- The MAC address of the scnet
    Return:  <0=failure
*/
int CommuForScnet::SetMacAddr(const uint8_t *mac)
{
    if (CheckMacAddr(mac)) return -1;

    CrtlMacFrame tbuf;
    memcpy(tbuf.desMac, kGroupMac, 6);
    memcpy(tbuf.srcMac, src_mac_, 6);
    tbuf.ethertype = htons(eth_type_);
    tbuf.length = htons(44);
    tbuf.cmd = htons(kSetMac);
    memcpy(tbuf.data.mac.dev_mac, mac, 6);
    uint32_t crc = crc32(0, Z_NULL, 0);
    uint8_t *pbuf = (uint8_t *)&tbuf;
    crc = crc32(crc, pbuf, 60);
    memcpy(&pbuf[60], &crc, 4);

    int i;
    for (i=0; i<3; i++) {
        sendto(socket_fd_, &tbuf, 64, 0, (struct sockaddr*)sock_ll_, sizeof(sockaddr_ll));
        msSleep(50);
        if (!MacPing(mac, 0)) {
            printf("Seting MAC %02X:%02X:%02X:%02X:%02X:%02X succeed\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            break;
        }
    }
    if(i>=3) {
        printf("Set MAC %02X:%02X:%02X:%02X:%02X:%02X failed!!!\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return -1;
    }
    return 0;
}

/*��
Save parameter to configuration file

    Input:  filename -- The configuration file used to store the parameters
            par -- parameter
*/
void CommuForScnet::SaveParam(const char *filename, Para4Scnet *par)
{
    printf("FirmwareVer:%d.%d.%d\n", par->ver[0][0], par->ver[0][1], par->ver[0][2]);
    FILE *fstrm = fopen(filename, "w");
    if (!fstrm) {
        printf("Cannot create %s!\n", filename);
        return;
    }
    fprintf(fstrm, "%s:%d.%d.%d\n", kParamName[kFirmVer], par->ver[0][0], par->ver[0][1], par->ver[0][2]);
    fprintf(fstrm, "%s=%s\n", kParamName[kDevModel], DeviceModelStr[par->dev_model]);
    fprintf(fstrm, "%s=%d,%d,%d,%d\n", kParamName[kADCBkgrdDC], par->adc_dc[0], par->adc_dc[1], par->adc_dc[2], par->adc_dc[3]);
    float fi[4];
    for (int i=0; i<4; i++) {
        fi[i] = par->corr[i];
        fi[i] /= 10000;
    }
    fprintf(fstrm, "%s=%6.4f,%6.4f,%6.4f,%6.4f\n", kParamName[kCorrFactor], fi[0], fi[1], fi[2], fi[3]);
    fprintf(fstrm, "%s=%d,%d\n", kParamName[kPT_CT], par->trns_rto[0], par->trns_rto[1]);

    fclose(fstrm);
}

/*!
Load parameter from configuration file

    Input:  filename -- The configuration file used to store the parameters
    Output: par -- parameter
    Return: 0=success, -1=error
*/
int CommuForScnet::LoadParam(Para4Scnet *par, const char *filename)
{
    FILE *fstrm = fopen(filename, "r");
    if (!fstrm) {
        printf("Cannot open %s!\n", filename);
        return -1;
    }
    char par_name[64], stri[128];
    int retv;
    float fi[4];
    for (int n=0; n<10; n++) {
        retv = fscanf(fstrm, "%[^:=]", par_name);
        if (retv==EOF) break;
        int i, j;
        for (i=0; i<=kPT_CT; i++) {
            if ( !strcmp(par_name, kParamName[i]) ) break;
        }
        //printf("%s i=%d\n", par_name, i);
        switch (i) {
            case kFirmVer:
                fscanf(fstrm, ":%d.%d.%d\n", &par->ver[0][0], &par->ver[0][1], &par->ver[0][2]);
                break;
            case kDevModel:
                fgets(stri, sizeof(stri), fstrm);
                sscanf(stri, "=%s", par_name);
                for (j=0; j<=PQNet202Ex; j++) {
                    if ( !strcmp(par_name, DeviceModelStr[j]) ) break;
                }
                if (j>PQNet202Ex) return -1;
                par->dev_model = j;
                break;
            case kADCBkgrdDC:
                fgets(stri, sizeof(stri), fstrm);
                sscanf(stri, "=%d,%d,%d,%d", &par->adc_dc[0], &par->adc_dc[1], &par->adc_dc[2], &par->adc_dc[3]);
                break;
            case kCorrFactor:
                fgets(stri, sizeof(stri), fstrm);
                sscanf(stri, "=%f,%f,%f,%f", &fi[0], &fi[1], &fi[2], &fi[3]);
                break;
            case kPT_CT:
                fgets(stri, sizeof(stri), fstrm);
                sscanf(stri, "=%d,%d", &par->trns_rto[0], &par->trns_rto[1]);
                break;
            default:
                break;
        }
    }
    fclose(fstrm);
    for (int i=0; i<4; i++) {
        par->corr[i] = fi[i]*10000;
    }

#if 0
    printf("%s:%d.%d.%d\n", kParamName[kFirmVer], par->ver[0][0], par->ver[0][1], par->ver[0][2]);
    printf("%s=%s\n", kParamName[kDevModel], DeviceModelStr[par->dev_model]);
    printf("%s=%d,%d,%d,%d\n", kParamName[kADCBkgrdDC], par->adc_dc[0], par->adc_dc[1], par->adc_dc[2], par->adc_dc[3]);
    printf("%s=%d,%d,%d,%d\n", kParamName[kCorrFactor], par->corr[0], par->corr[1], par->corr[2], par->corr[3]);
    printf("%s=%d,%d\n", kParamName[kPT_CT], par->trns_rto[0], par->trns_rto[1]);
#endif

    return 0;
}

/*��
Get parameter from scnet

    Input:  filename -- The configuration file used to store the parameters
            mac -- The MAC address of the scnet
    Return:  <0=failure
*/
int CommuForScnet::GetParam(const char *filename, const uint8_t *mac)
{
    if (CheckMacAddr(mac)) return -1;
    
    CrtlMacFrame tbuf;
    memcpy(tbuf.desMac, mac, 6);
    memcpy(tbuf.srcMac, src_mac_, 6);
    tbuf.ethertype = htons(eth_type_);
    tbuf.length = htons(44);
    tbuf.cmd = htons(kGetPar);
    uint32_t crc = crc32(0, Z_NULL, 0);
    uint8_t *pbuf = (uint8_t *)&tbuf;
    crc = crc32(crc, pbuf, 60);
    memcpy(&pbuf[60], &crc, 4);

    int retv = -1, i, cnt;
    CrtlMacFrame *rbuf = (CrtlMacFrame *)rx_buf_;
    for (i=0; i<3; i++) {
        sendto(socket_fd_, &tbuf, 64, 0, (struct sockaddr*)sock_ll_, sizeof(sockaddr_ll));
        StopWatch(0, 1, NULL);
        for (;;) {
            cnt = recvfrom(socket_fd_, rx_buf_, sizeof(rx_buf_), 0, NULL, NULL);
            if(cnt>0) {
                if (!memcmp(mac, rbuf->srcMac, 6) && ntohs(rbuf->cmd)==kGetPar) {
                    uint32_t fcs = *(uint32_t *)(&rx_buf_[cnt-4]);
                    crc = crc32(0, Z_NULL, 0);
                    if (fcs==crc32(crc, rx_buf_, cnt-4)) {
                        StopWatch(0, 0, NULL);
                        retv = 0;
                        memcpy(&par4scnet_, &rbuf->data.para, sizeof(par4scnet_));
                        SwapBytes(&par4scnet_, 1);
                        if (filename) {
                            printf("%d bytes frome %02X:%02X:%02X:%02X:%02X:%02X; time=%6.3fms\n", 
                                     cnt, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], stopwatch_dur(0)*1000);
                            SaveParam(filename, &par4scnet_);
                        }
                        break;
                    }
                }
            }
            StopWatch(0, 0, NULL);
            if (stopwatch_dur(0)>1) break;
        }
        if (!retv) break;
        else {
            if (filename) printf("Get parameter failed!\n");
        }
    }
    return retv;
}

/*!
Set parameter to scnet

    Input:  filename -- The configuration file used to load the parameters
            mac -- The MAC address of the scnet
    Return:  <0=failure
*/
int CommuForScnet::SetParam(const char *filename, const uint8_t *mac)
{
    if (CheckMacAddr(mac)) return -1;
    
    CrtlMacFrame tbuf;
    memcpy(tbuf.desMac, mac, 6);
    memcpy(tbuf.srcMac, src_mac_, 6);
    tbuf.ethertype = htons(eth_type_);
    tbuf.length = htons(sizeof(Para4Scnet)+4);
    tbuf.cmd = htons(kSetPar);
    if (filename) {
        if ( LoadParam(&tbuf.data.para, filename) ) {
            printf("Error loading parameters from file!\n");
            return -1;
        }
    } else {
        memcpy(&tbuf.data.para, &par4scnet_, sizeof(par4scnet_));
    }
    SwapBytes(&tbuf.data.para, 0);
    
    uint8_t *pbuf = (uint8_t *)&tbuf;
    uint32_t crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, pbuf, sizeof(Para4Scnet)+20);
    memcpy(&pbuf[sizeof(Para4Scnet)+20], &crc, 4);

    int retv = -1, i, cnt;
    CrtlMacFrame *rbuf = (CrtlMacFrame *)rx_buf_;
    for (i=0; i<3; i++) {
        sendto(socket_fd_, &tbuf, sizeof(Para4Scnet)+24, 0, (struct sockaddr*)sock_ll_, sizeof(sockaddr_ll));
        StopWatch(0, 1, NULL);
        for (;;) {
            msSleep(1);
            cnt = recvfrom(socket_fd_, rx_buf_, sizeof(rx_buf_), 0, NULL, NULL);
            if(cnt>0) {
                if (!memcmp(mac, rbuf->srcMac, 6) && ntohs(rbuf->cmd)==kSetPar) {
                    uint32_t fcs = *(uint32_t *)(&rx_buf_[cnt-4]);
                    crc = crc32(0, Z_NULL, 0);
                    if (fcs==crc32(crc, rx_buf_, cnt-4)) {
                        StopWatch(0, 0, NULL);
                        printf("Set parameters succeed >> %02X:%02X:%02X:%02X:%02X:%02X. time=%6.3fms\n", 
                                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], stopwatch_dur(0)*1000);
                        retv = 0;
                        break;
                    }
                }
            }
            StopWatch(0, 0, NULL);
            if (stopwatch_dur(0)>1) break;
        }
        if (!retv) break;
        else printf("Set parameters failed!\n");
    } 
    return retv;       
}

/*!
ping MAC packet

    Input:  mac -- destination MAC address
            echo -- 0=no, 1=yes
    Return:  <0=failure
*/
int CommuForScnet::MacPing(const uint8_t *mac, uint8_t echo)
{
    if (CheckMacAddr(mac)) return -1;

    CrtlMacFrame tbuf;
    memcpy(tbuf.desMac, mac, 6);
    memcpy(tbuf.srcMac, src_mac_, 6);
    tbuf.ethertype = htons(eth_type_);
    tbuf.length = htons(44);
    tbuf.cmd = htons(kMacPing);
    uint32_t crc = crc32(0, Z_NULL, 0);
    uint8_t *pbuf = (uint8_t *)&tbuf;
    crc = crc32(crc, pbuf, 60);
    memcpy(&pbuf[60], &crc, 4);

    int retv = -1, i, cnt;
    CrtlMacFrame *rbuf = (CrtlMacFrame *)rx_buf_;
    for (i=0; i<3; i++) {
        sendto(socket_fd_, &tbuf, 64, 0, (struct sockaddr*)sock_ll_, sizeof(sockaddr_ll));
        StopWatch(0, 1, NULL);
        for (;;) {
            cnt = recvfrom(socket_fd_, rx_buf_, sizeof(rx_buf_), 0, NULL, NULL);
            if(cnt>0) {
                if (!memcmp(mac, rbuf->srcMac, 6) && ntohs(rbuf->cmd)==kMacPing) {
                    uint32_t fcs = *(uint32_t *)(&rx_buf_[cnt-4]);
                    crc = crc32(0, Z_NULL, 0);
                    if (fcs==crc32(crc, rx_buf_, cnt-4)) {
                        if (echo) {
                             StopWatch(0, 0, NULL);
                             printf("%d bytes frome %02X:%02X:%02X:%02X:%02X:%02X; time=%6.3fms\n", 
                                     cnt, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], stopwatch_dur(0)*1000);
                        }
                        retv = 0;
                        break;
                    }
                }
            }
            StopWatch(0, 0, NULL);
            if (stopwatch_dur(0)>1) break;
        }
        if (!retv) break;
        else if (echo) printf("Destination Host Unreachable!\n");
    }
    return retv;
}

int CommuForScnet::OpenSocket()
{

    socket_fd_ = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(socket_fd_ < 0) {
        perror("socket");
	    return -1;
    }
  	fcntl(socket_fd_, F_SETFL, O_NONBLOCK);     //Set read & write to no block
    return 0;
}

/*!
Open upgrade file

    Input:  filename -- upgrade filename
            vdx -- version index.
            fc -- force. 1=yes
    Output: ver -- version of upgrade file. uint8_t[3]
*/
const char *verstr[2] = {"kFirmVerA", "kFirmVerB"};
FILE *CommuForScnet::OpenUpFile(uint8_t *ver, const char *filename, int vdx, uint8_t fc)
{
    FILE *fstrm = fopen(filename, "rb");
    if (!fstrm) {
        printf("Cannot open %s!\n", filename);
        return NULL;
    }
    uint8_t buf[128];
    int sz = fread(buf, 1, 128, fstrm);
    for (;;) {
        if (!sz) break;
        int i;
        for (i=0; i<=sz-12; i+=4) {
            if (!memcmp(&buf[i], verstr[vdx], 9)) break;
        }
        if (i<=sz-12) {
            memcpy(ver, &buf[i+9], 3);
            break;
        }
        memmove(buf, &buf[116], 12);
        sz = fread(&buf[12], 1, 116, fstrm);
    }
    int retv = 0;
    if (!sz) {
        printf("Invalid upgrade file\n");
        retv = -1;
    } else {
        printf("%d.%d.%d-->%d.%d.%d\n", par4scnet_.ver[vdx][0], par4scnet_.ver[vdx][1], par4scnet_.ver[vdx][2], ver[0], ver[1], ver[2]);
        if (memcmp(ver, par4scnet_.ver[vdx], 3)<=0 && !fc) {
            printf("Already the latest version, no need to upgrade!\n");
            retv = -1;
        }
    }
    if (retv) {
        fclose(fstrm);
        fstrm = NULL;
    }
    return fstrm;
}

/*��
Upgrade firmware

    Input:  filename -- firmware bin file
            mac -- The MAC address of the scnet
            cmd -- kUpApp or kUpBoot
            fc -- force. 1=yes
    Return:  <0=failure
*/
int CommuForScnet::Upgrade(const char *filename, const uint8_t *mac, uint16_t cmd, uint8_t fc)
{
    if (GetParam(NULL, mac)<0) {
        printf("Destination Host Unreachable!\n");
        return -1;
    }

    uint8_t ver[3]={0,0,0};
    uint8_t vdx = cmd-kUpApp;
    FILE *fstrm = OpenUpFile(ver, filename, vdx, fc);
    if (!fstrm) return -2;
    
    fseek(fstrm, 0, SEEK_SET);
    CrtlMacFrame tbuf;
    memcpy(tbuf.desMac, mac, 6);
    memcpy(tbuf.srcMac, src_mac_, 6);
    tbuf.ethertype = htons(eth_type_);
    tbuf.length = htons(sizeof(UpgradePack)+4);
    tbuf.cmd = htons(cmd);
    tbuf.data.pack.idx = 0;
    uint8_t *pbuf = (uint8_t *)&tbuf;
    printf("Upload file .");
    fflush(stdout);
    int retv;
    for (;;) {
        int sz = fread(tbuf.data.pack.buf, 1, 1024, fstrm);
        tbuf.data.pack.size = sz;
        if (sz<1024) {
            tbuf.data.pack.idx = 0xFFFF;
        }
        uint32_t crc = crc32(0, Z_NULL, 0);
        crc = crc32(crc, pbuf, sizeof(UpgradePack)+20);
        memcpy(&pbuf[sizeof(UpgradePack)+20], &crc, 4);
        
        int i, cnt;
        retv = -1;
        CrtlMacFrame *rbuf = (CrtlMacFrame *)rx_buf_;
        for (i=0; i<3; i++) {
            sendto(socket_fd_, &tbuf, sizeof(UpgradePack)+24, 0, (struct sockaddr*)sock_ll_, sizeof(sockaddr_ll));
            StopWatch(0, 1, NULL);
            for (;;) {
                cnt = recvfrom(socket_fd_, rx_buf_, sizeof(rx_buf_), 0, NULL, NULL);
                if(cnt>0) {
                    if (!memcmp(mac, rbuf->srcMac, 6) && ntohs(rbuf->cmd)==cmd) {
                        uint32_t fcs = *(uint32_t *)(&rx_buf_[cnt-4]);
                        crc = crc32(0, Z_NULL, 0);
                        if (fcs==crc32(crc, rx_buf_, cnt-4)) {
                            StopWatch(0, 0, NULL);
                            printf("."); fflush(stdout);
                            retv = 0;
                            break;
                        }
                    }
                }
                StopWatch(0, 0, NULL);
                if (stopwatch_dur(0)>3) break;
            }
            if (!retv) break;
        }
        if (retv || sz<1024) {
            break;
        }
        tbuf.data.pack.idx++;
    }
    fclose(fstrm);
    
    if (!retv) {
        printf("ok\nStart update ."); fflush(stdout);
        int i;
        msSleep(500);
        for (i=0; i<15; i++) {
            printf("."); fflush(stdout);
            if (!GetParam(NULL, mac)) {
               if (memcmp(ver, par4scnet_.ver[vdx], sizeof(ver))==0) {
                    printf("ok\nUpdate succeed!\n");
                } else i = 30;
                break;
            }
        }
        if (i>=15) {
            printf("failed!\nUpdate failed!\n");
            return -3;
        }
    } else {
        printf("failed!\n");
        return -3;
    }
    return 0;
}

/*!
Batch set the parameters of several devices

    Input:  chnl -- uint8_t[4]. [0-3]:channel1-4. 0=needn't set, 1=need set
            ratio -- uint32_t[4][2]. [0-3]:channel1-4, [0-1]:primary,secondary. unit:V/A
    Retrun: <0=failure
*/
int CommuForScnet::BatchSet(const uint8_t *chnl, const uint32_t *ratio, const uint8_t *mac)
{
    uint8_t scnet_mac[6] = {0xB0, 0xE0, 0x14};
    int retv = 0;
    for (int i=0; i<4; i++) {
        //printf("%d; %d:%d; %x-%x-%x\n", chnl[i], ratio[2*i], ratio[2*i+1], mac[3*i], mac[3*i+1], mac[3*i+2]);
        int up = 0;
        if (chnl[i]) {
            memcpy(&scnet_mac[3], &mac[3*i], 3);
            retv = GetParam(NULL, scnet_mac);
            if (retv<0) continue;
            //printf("trns_rto=%d,%d\n", par4scnet_.trns_rto[0], par4scnet_.trns_rto[1]);
            if ( memcmp(par4scnet_.trns_rto, &ratio[2*i], 8) ) {
                memcpy(par4scnet_.trns_rto, &ratio[2*i], 8);
                //printf("trns_rto=%d,%d\n", par4scnet_.trns_rto[0], par4scnet_.trns_rto[1]);
                up++;
            }
            if (up) {
                retv = SetParam(NULL, scnet_mac);
            }
        }
    }
    return retv;
}

/*!
Swap bytes order -- endianness

    Input:  type -- 0=Host2Net, 1=Net2Host
    Output: par --
*/
inline void CommuForScnet::SwapBytes(Para4Scnet *par, int type)
{
    if (type) {
        for (int i=0; i<4; i++) {
            par->adc_dc[i] = ntohs(par->adc_dc[i]);
            par->corr[i] = ntohl(par->corr[i]);
            par->adc_dc[i] = ntohs(par->adc_dc[i]);
        }
        for (int i=0; i<2; i++) {
            par->trns_rto[i] = ntohl(par->trns_rto[i]);
        }
    } else {
        for (int i=0; i<4; i++) {
            par->adc_dc[i] = htons(par->adc_dc[i]);
            par->corr[i] = htonl(par->corr[i]);
            par->adc_dc[i] = htons(par->adc_dc[i]);
        }
        for (int i=0; i<2; i++) {
            par->trns_rto[i] = htonl(par->trns_rto[i]);
        }
    }
}