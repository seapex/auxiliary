#include <stdlib.h>
#include <netinet/in.h>

#include "mqtt_client.h"
#include "param_cfg.h"
#include "app_prtcl.h"
#include "phy_prtcl.h"
#include "debug_info.h"
#include "format_convrt.h"
#include "messageq_mqttc.h"
#include "time_cst.h"

const char * MQTTClient::prtcl_name_ = "MQTT";

MQTTClient::MQTTClient(SocketClient *sock)
{
    sock_clnt_ = sock;
    ass_idx_ = sock_clnt_->RegistIdx();
    if (ass_idx_ < 0) {
        printf("Register communication object index(%d) failure!\n", ass_idx_);
        abort();
    }
    data_bufs().new_buf(ass_idx_);
    data_buf_ = data_bufs().buf(ass_idx_);
    if (!data_buf_) {
        PRINT_DBG(0)("data_buf_ allocation failure!\n")DBG_PRINT
        exit(1);
    }
    tbuf_sz_ = 4096;
    tx_buf_ = new uint8_t[tbuf_sz_];
}

MQTTClient::~MQTTClient()
{
    data_bufs().del_buf(ass_idx_);
    sock_clnt_->set_regist_idx(ass_idx_, 0);    //Deregister associated object index
}

/*!
Associate with MQTT Server/Broker, then connecting

    Return: 0=success, 1=failure
*/
int MQTTClient::AssociateMqttS()
{
    printf("Connect to %s %s\n",  param_cfg().hostname(), param_cfg().name()); fflush(stdout);
    int retv=1;
    for (int i=0; i<5; i++) {
        if (!data_buf_->status()) {   //Socket not connected
            if (sock_clnt_->Start(param_cfg().name(), param_cfg().hostname(), kPhyPrtclMqttC,
                                  kAppPrtclMqttC, ass_idx_+1) == 0) {
                retv = 0;
                break;
            }
        }
        msSleep(200);
    }
    return retv;
}

/*!
Abort the connect with MQTT Server -- Broker
*/
int MQTTClient::AbortMqttS()
{
    if (data_buf_->status()) { //socket connected
        sock_clnt_->end(ass_idx_+1);
    }
}

/*!
Get connection status

    Input:  show -- 0=not 1=yes
    Return: 0=Socket not connected, 1=Mqtt server not connected, 2=Mqtt server connected
*/
int MQTTClient::ConnectStatus(int show)
{
    int status = data_buf_->status();
    if (show) {
        if (status==1) {
            printf("Mqtt server not connected\n");
        } else if (!status) {
            printf("Socket not connected\n");
        }
    }
    return status;
}

/*!
Send CONNECT packet to MQTT Server -- Broker

    Input:  lvl -- protocol revision level. 4 or 5
            flag -- connect flags. bit1:clean start; bit2:will flag;...
            alive -- Keep alive
*/
void MQTTClient::Connect(uint8_t lvl, uint8_t flag, uint16_t alive)
{
    uint32_t u32 = 0xffff;
    data_buf_->SetProperty(kSExpiryInterval, &u32);
    
    int k, m;
    if (ConnectStatus(0)==1) { //mqtt server not connected
        data_buf_->set_rev(lvl);
        flags_ = flag;
        kp_alv_ = alive;
        //---- Variable header ----
        uint8_t *pbuf = &tx_buf_[5];
        
        int len = strlen(prtcl_name_);
        pbuf[0] = 0; pbuf[1] = len;
        memcpy(&pbuf[2], prtcl_name_, len);
        len += 2;
        pbuf[len++] = lvl;
        pbuf[len++] = flag;
        *(uint16_t*)(&pbuf[len]) = htons(alive); len += 2;
        len = AddProperty(kCONNECT, pbuf, len);
        //---- Payload ------------
        m = data_buf_->UpClientID();   //Client identifier
        memcpy(&pbuf[len], data_buf_->clnt_id(), m);
        len += m;
        /*if (lvl==5) {   //Will properties
            pbuf[len++] = 0;
        }*/
        //---- Fixed header -------
        fix_hd_[0] = kCONNECT<<4;
        k = EncodeVarBint(&fix_hd_[1], len);
        memcpy(&tx_buf_[4-k], fix_hd_, k+1);

        messageq_mqttc().Push(ass_idx_, kCONNECT, &tx_buf_[4-k], len+k+1);
        data_buf_->set_status(2);
    }
}

/*!
Send DISCONNECT packet to MQTT Server -- Broker

    Input:  rscd -- reason code
*/
void MQTTClient::Disconnect(uint8_t rscd)
{
    int k, m;
    int status = ConnectStatus(0);
    if (status==2) { //mqtt server connected
        //---- Variable header ----
        int len = 0;
        if (data_buf_->rev()==5) {
            uint8_t *pbuf = &tx_buf_[5];
            *pbuf++ = rscd;
            len = 0; //AddProperty(kDISCONNECT, pbuf, 1);
            if (rscd || len>1) {
                len += 1;
            } else {
                len = 0;
            }
        }
        //---- Fixed header -------
        fix_hd_[0] = kDISCONNECT<<4;
        k = EncodeVarBint(&fix_hd_[1], len);
        memcpy(&tx_buf_[4-k], fix_hd_, k+1);
        messageq_mqttc().Push(ass_idx_, kDISCONNECT, &tx_buf_[4-k], len+k+1);
        do {
            msSleep(500);
        } while (data_buf_->status()==2);
        printf("status=%d\n", data_buf_->status());
        msSleep(300);
        AbortMqttS();
    } else if (status) {    //socket connected
        AbortMqttS();
    }
}

/*!
keep-alive connection
*/
void MQTTClient::KeepConnect()
{
    if (!data_buf_->status()) {
        AssociateMqttS();
    } 
    
    if (data_buf_->status()==1) {
        Connect(data_buf_->rev(), flags_, kp_alv_);
    }
}

/*!
Send PUBLISH packet to MQTT Server -- Broker

    Input:  flgs -- fixed header flags. bit3:duplicate, bit2-1:QoS, bit0:retain
            topic -- topic name
            msg -- application message
            sz -- size of msg in bytes
*/
void MQTTClient::Publish(uint8_t flgs, const char *topic, const uint8_t *msg, int sz)
{
    int k, m;
    if (ConnectStatus()==2) { //mqtt server connected
        //---- Variable header ----
        uint8_t *pbuf = &tx_buf_[5];
        int len = strlen(topic);
        pbuf[0] = len>>8;
        pbuf[1] = len&0xff;
        memcpy(&pbuf[2], topic, len);
        len += 2;
        if (flgs&0x6) { //QoS>0
            messageq_mqttc().inc_pk_id();
            uint16_t pkg_id = htons(messageq_mqttc().pk_id());
            memcpy(&pbuf[len], &pkg_id, 2);
            len += 2;
        }
        len = AddProperty(kPUBLISH, pbuf, len);
        //---- Payload ------------
        memcpy(&pbuf[len], msg, sz);
        len += sz;
        //---- Fixed header -------
        fix_hd_[0] = kPUBLISH<<4;
        fix_hd_[0] |= flgs;
        k = EncodeVarBint(&fix_hd_[1], len);
        memcpy(&tx_buf_[4-k], fix_hd_, k+1);
        messageq_mqttc().Push(ass_idx_, kPUBLISH, &tx_buf_[4-k], len+k+1);
    }
}

/*!
Add property

    Input:  type -- MQTT Control Packet Type. 
            pbuf -- tx buffer
            len -- length in bytes before calling this function
    Return: length in bytes after called this function
*/
int MQTTClient::AddProperty(CtrlPackType type, uint8_t *pbuf, int len)
{
    if (data_buf_->rev()==5) {
        int k = data_buf_->GetProperty(&pbuf[len], type);
        if (k) {
            len += k;
            TxbufOverflow(len);
        }
    }
    return len;
}

/*!
Send SUBSCRIBE packet to MQTT Server -- Broker

    Input:  topics -- topic filters
            optns -- options of topic filter
            cnt -- count of topic filters
*/
void MQTTClient::Subscribe(const char * const *topics, uint8_t *optns, int cnt)
{
    int k, m;
    if (ConnectStatus()==2) { //mqtt server connected
        //---- Variable header ----
        uint8_t *pbuf = &tx_buf_[5];
        messageq_mqttc().inc_pk_id();
        uint16_t pkg_id = htons(messageq_mqttc().pk_id());
        memcpy(pbuf, &pkg_id, 2);
        int len = 2;
        len = AddProperty(kSUBSCRIBE, pbuf, len);
        //---- Payload ------------
        for (int i=0; i<cnt; i++) {
            k = strlen(topics[i]);
            *((uint16_t *)&pbuf[len]) = htons(k);
            len += 2;
            memcpy(&pbuf[len], topics[i], k);
            len += k;
            if (data_buf_->rev()<5) {
                optns[i] &=3;
            }
            pbuf[len] = optns[i];
            len++;
        }
        //---- Fixed header -------
        fix_hd_[0] = kSUBSCRIBE<<4 | 0x2;
        k = EncodeVarBint(&fix_hd_[1], len);
        memcpy(&tx_buf_[4-k], fix_hd_, k+1);
        messageq_mqttc().Push(ass_idx_, kSUBSCRIBE, &tx_buf_[4-k], len+k+1);
    }
}

/*!
Send UNSUBSCRIBE packet to MQTT Server -- Broker

    Input:  topics -- topic filters
            cnt -- count of topic filters
*/
void MQTTClient::Unsubscribe(const char * const *topics, int cnt)
{
    int k, m;
    if (ConnectStatus()==2) { //mqtt server connected
        //---- Variable header ----
        uint8_t *pbuf = &tx_buf_[5];
        messageq_mqttc().inc_pk_id();
        uint16_t pkg_id = htons(messageq_mqttc().pk_id());
        memcpy(pbuf, &pkg_id, 2);
        int len = 2;
        len = AddProperty(kUNSUBSCRIBE, pbuf, len);
        //---- Payload ------------
        for (int i=0; i<cnt; i++) {
            k = strlen(topics[i]);
            *((uint16_t *)&pbuf[len]) = htons(k);
            len += 2;
            memcpy(&pbuf[len], topics[i], k);
            len += k;
        }
        //---- Fixed header -------
        fix_hd_[0] = kUNSUBSCRIBE<<4 | 0x2;
        k = EncodeVarBint(&fix_hd_[1], len);
        memcpy(&tx_buf_[4-k], fix_hd_, k+1);
        messageq_mqttc().Push(ass_idx_, kUNSUBSCRIBE, &tx_buf_[4-k], len+k+1);
    }
}

/*!
Check if the tx_buf_ is overflowing, and reallocate memory if it is

    Input:  len
*/
inline void MQTTClient::TxbufOverflow(int len)
{
    if (len > tbuf_sz_-2048) {
        tbuf_sz_ *= 2;
        uint8_t *nbuf = new uint8_t[tbuf_sz_];
        memcpy(nbuf, tx_buf_, len);
        delete [] tx_buf_;
        tx_buf_ = nbuf;
    }
}

