/*! \file buffer_pool.h
    \brief display data buffer pool.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DATA_BUF_H_
#define _DATA_BUF_H_

#include <stdio.h>
#include "loop_buffer.h"
#include "message_queue.h"

enum CtrlPackType { kCONNECT=1, kCONNACK, kPUBLISH, kPUBACK, kPUBREC, kPUBREL, kPUBCOMP, 
        kSUBSCRIBE, kSUBACK, kUNSUBSCRIBE, kUNSUBACK, kPINGREQ, kPINGACK, kDISCONNECT, kAUTH };

enum MQTTProperty { kPayloadFormat=1, kMExpiryInterval/*Message Expiry Interval*/,
    kContentType, kResponseTopic=8, kCorrelationData,
    kSubscriptionID=11, kSExpiryInterval=17/*Session Expiry Interval*/,
    kAsgClientID/*Assigned Client Identifier*/, kSvrKeepAlive,
    kAuthenticMethod=21, kAuthenticData, kReqProblemInfo, kWillDelayIntrv,
    kReqRespInfo, kRespInfo, kSvrReference=28, kReasonStr=31, kReceiveMax=33,
    kTopicAliasMax, kTopicAlias, kMaxQoS, kRetainAvailable, kUserPropety, 
    kMaxPackSize, kWildcardSubsAvlb, kSubsIDAvlb, KSharedSubsAvlb,
    kEndMQTTProperty };

struct PropertyValue {
    uint8_t payld_frmt;     //Payload Format Indicator. 0=unspecified bytes, 1=UTF-8 Encoded Character Data
    uint32_t mexp_intrvl;   //Message Expiry Interval. lifetime of the Application Message in seconds.If absent, the Application Message does not expire
    uint8_t *cont_type;     //Content Type. UTF-8 String describing the content of the Application Message.
    uint8_t *resp_top;      //Response Topic. UTF-8 String
    uint8_t *crltn_data;    //Correlation Data. Binary data
    uint32_t sub_id;        //Subscription Identifier. Variable Byte Integer
    uint32_t sexp_intrvl;   //Session Expiry Interval. unit:s. 0 or absent, Session ends when the Connection is closed. 0xFFFFFFFF the Session does not expire
    uint8_t *asg_clnt_id;   //Assigned Client Identifier. UTF-8 String
    uint16_t svr_kp_alv;    //Server Keep Alive. uint:s
    uint8_t *authent_mtd;   //Authentication Method. UTF-8 String
    uint8_t *authent_data;  //Authentication Data. Binary
    uint8_t req_prblm_inf;  //Request Problem Information.
    uint32_t wl_dly_intrvl; //Will Delay Interval
    uint8_t req_resp_inf;   //Request Response Information
    uint8_t *resp_inf;      //Response Information. UTF-8 String
    uint8_t *svr_ref;       //Server reference. UTF-8 String
    uint8_t *reasn_str;     //Reason String. UTF-8 String
    uint16_t rec_max;       //Receive Maximum
    uint16_t top_ala_max;   //Topic Alias Maximum
    uint16_t top_ala;       //Topic Alias
    uint8_t max_qos;        //Maximum QoS
    uint8_t retn_avlb;      //Retain Available
    uint8_t *usr_prpty;     //User Property. UTF-8 String
    uint32_t max_pack_sz;   //Maximum Packet Size
    uint8_t wld_sub_avlb;   //Wildcard Subscription Available
    uint8_t sub_id_avlb;    //Subscription Identifiers Available
    uint8_t shar_sub_avlb;  //Shared Subscription Available
};

class DataBuf
{
    void DynAssignVal(uint8_t **pry, const void *val, int len);

    pthread_mutex_t mutex_;
    PropertyValue property_val_;
    PropertyValue property_rev_;  //Received properties
    uint8_t *clnt_id_;  //MQTT Client identifier
    uint8_t status_;  //0=no use, 1=in use, 2=mqtt server connected
    uint8_t rev_;     //MQTT protocol revision level
    uint8_t Property01_[kEndMQTTProperty];  //Property on off. 0=off, 1=on
    uint8_t *pry_buf_;  //property buffer
    int pbuf_sz_;       //size of pry_buf_ in bytes
	uint8_t ack_flg_;   //Connect acknowledge flags. 0=session absent, 1=session present

  public:
  	DataBuf();
	~DataBuf();

    int ExtractProperty(const uint8_t *rbuf);
    int GetProperty(uint8_t *obuf, CtrlPackType type);
    void SetProperty(MQTTProperty id, void *val, int len=0);
    int UpClientID();

    //Accessors
    uint8_t ack_flg() { return ack_flg_; } 
    const uint8_t *clnt_id() { return clnt_id_; }
    const uint8_t *pry_buf() { return pry_buf_; }
    uint8_t rev() { return rev_; }
    uint8_t status() { return status_; }

    //Mutators
    void set_ack_flg(uint8_t val) { ack_flg_ = val; }
    void set_rev(uint8_t val) { rev_ = val; }
    void set_status(uint8_t val) { status_ = val; }

  protected:
   
};

#endif // _DATA_BUF_H_
