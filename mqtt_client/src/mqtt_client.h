/*! \file mqtt_client.h
    \brief MQTT Client handler.
    Copyright (c) 2022  Xi'an Boyuu Electric, Inc.
*/
#ifndef _MQTT_CLIENT_H_
#define _MQTT_CLIENT_H_

#include "socket_client.h"
#include "data_bufs.h"

class MQTTClient {
    void TxbufOverflow(int len);
    int AddProperty(CtrlPackType type, uint8_t *pbuf, int len);
    int AbortMqttS();
    int ConnectStatus(int show=1);
  
    SocketClient *sock_clnt_;
    int ass_idx_;   //associated object index
    class DataBuf *data_buf_;
    
    uint8_t flags_; //Connect flags. bit7:user name; bit6:passwd; bit5:Will Retain; 
                    //bit4-3:Will QoS; bit2:Will Flag; bit1:Clean Start
    uint16_t kp_alv_;   //Keep Alive. unit:s
    uint8_t *tx_buf_;   //transmit buffer
    int tbuf_sz_;       //size of tx_buf_ in bytes
    uint8_t fix_hd_[5]; //MQTT fixed header
   
    static const char *prtcl_name_;  //protocol name
    
public:
    MQTTClient(SocketClient *sock);
    ~MQTTClient();

    int AssociateMqttS();
    void Connect(uint8_t lvl, uint8_t flag, uint16_t alive=0);
    void Disconnect(uint8_t rscd=0);
    void Publish(uint8_t flgs, const char *topic, const uint8_t *msg, int sz);
    void KeepConnect();
    void Subscribe(const char * const *topics, uint8_t *optns, int cnt);
    void Unsubscribe(const char * const *topics, int cnt);

    //Accessors
    uint8_t session() { return data_buf_->ack_flg(); }  //0=session absent, 1=session present
    
};

#endif //_MQTT_CLIENT_H_
