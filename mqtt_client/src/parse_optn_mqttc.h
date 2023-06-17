/*! \file parse_optn_mqttc.h
    \brief Parse command line option for mqtt_client.
    Copyright (c) 2022  Xi'an Boyuu Electric, Inc.
*/
#ifndef _PARSE_OPTN_MQTTC_H_
#define _PARSE_OPTN_MQTTC_H_

#include <stdint.h>
//#include <getopt.h>
#include "parse_option.h"

enum SubCmdType {  
    kCmdTypeEnd=1 };
enum MsgType {
    kMsgPublish, kMsgSubscribe
};
class ParseOptnMqttC:public ParseOption {
public:
    ParseOptnMqttC();
    ~ParseOptnMqttC();

    //Accessors
    int ver() { return ver_; }
    const char * msg() { return msg_; }
    uint8_t qos() { return qos_; }
    uint8_t retain() { return retain_; }
    uint8_t send(MsgType type) { return send_[type]; }
    const char * topic() { return topic_; }
    uint8_t tstpqm() { return tstpqm_; }

protected:
    int HandleMain(int opt, char *arg=NULL);
    int HandleSubcmd(int opt, char *arg=NULL);
    int HandleOther(char *arg) { return 0; }
private:
    void InitParam();

    int ver_;   //protocol versin. 4 or 5
    uint8_t send_[2];   //send packet? 1=y,0=n. [0-1]:publish,subscribe
    uint8_t retain_;    //PUBLISH flag.RETAIN
    uint8_t qos_;       //PUBLISH flag.QoS
    char *topic_;
    char *msg_;     //Application Message
    uint8_t tstpqm_;    //test pqm function. 0=disable, 1,2,3...
};



#endif //_PARSE_OPTN_MQTTC_H_