/*! \file parse_optn_svx.h
    \brief Parse command line option for sv_rx_9-2.
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
    uint8_t clean() { return clean_; }
    uint8_t send(MsgType type) { return send_[type]; }
    uint8_t retain() { return retain_; }
    uint8_t qos() { return qos_; }
    const char * topic() { return topic_; }
    const char * msg() { return msg_; }

protected:
    int HandleMain(int opt, char *arg=NULL);
    int HandleSubcmd(int opt, char *arg=NULL);
    int HandleOther(char *arg) { return 0; }
private:
    void InitParam();

    int ver_;   //protocol versin. 4 or 5
    uint8_t clean_; //clean_start. 1=yes
    uint8_t send_[2];   //send packet? [0-1]:publish,subscribe
    uint8_t retain_;    //PUBLISH flag.RETAIN
    uint8_t qos_;       //PUBLISH flag.QoS
    char *topic_;
    char *msg_;
};



#endif //_PARSE_OPTN_MQTTC_H_