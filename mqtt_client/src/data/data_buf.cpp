#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <zlib.h>
#include <netinet/in.h>

#include "data_buf.h"
#include "format_convrt.h"
#include "thread_mng.h"
#include "param_cfg.h"
#include "debug_info.h"
//#include "time_cst.h"
//#include "gui_msg_queue.h"
//#include "md5.h"

DataBuf::DataBuf()
{
    pthread_mutex_init (&mutex_, NULL);
    clnt_id_ = NULL;
    status_ = 0;
    memset(Property01_, 0, sizeof(Property01_));
    memset(&property_val_, 0, sizeof(property_val_));
    memset(&property_rev_, 0, sizeof(property_rev_));

    pbuf_sz_ = 4096;
    pry_buf_ = new uint8_t[pbuf_sz_];
    ack_flg_ = 0;
}

DataBuf::~DataBuf()
{
    pthread_mutex_destroy(&mutex_);
}

/*!
Dynamic assign value to property

    Input:  val -- property value
            len -- length of val
    Output: pry
*/
inline void DataBuf::DynAssignVal(uint8_t **pry, const void *val, int len)
{
    uint8_t *pval = *pry;
    if (pval) delete [] pval;
    pval = new uint8_t[len+3];
    pval[len+2] = 0;    //For debug display only
    
    *(uint16_t *)pval = htons(len);
    memcpy(&pval[2], val, len);

    *pry = pval;
}

/*!
Extract properties from rbuf

    Input:  rbuf --
    Return: total length of property, include property length
*/
int DataBuf::ExtractProperty(const uint8_t *rbuf)
{
    if (rev_<=4) return 0;
    int k, m, len;
    k = DecodeVarBint(&m, rbuf);
    len = k + m;
    rbuf += k;
    uint16_t si;
    while(m>0) {
        uint8_t id = *rbuf;
        rbuf++;
        m--;
        switch (id) {
            case kPayloadFormat:
                property_rev_.payld_frmt = *rbuf;
                PRINT_DBG(1)("payld_frmt=%d\n", property_rev_.payld_frmt)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kMExpiryInterval:
                property_rev_.mexp_intrvl = ntohl(*(uint32_t*)rbuf);
                PRINT_DBG(1)("mexp_intrvl=%d\n", property_rev_.mexp_intrvl)DBG_PRINT
                rbuf += 4;
                m -= 4;
                break;
            case kContentType:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.cont_type, rbuf, si);
                PRINT_DBG(1)("cont_type=%s\n", &property_rev_.cont_type[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kResponseTopic:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.resp_top, rbuf, si);
                PRINT_DBG(1)("resp_top=%s\n", &property_rev_.resp_top[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kCorrelationData:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.crltn_data, rbuf, si);
                PRINT_DBG(1)("crltn_data=%s\n", &property_rev_.crltn_data[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kSubscriptionID:
                property_rev_.sub_id = ntohl(*(uint32_t*)rbuf);
                PRINT_DBG(1)("sub_id=%d\n", property_rev_.sub_id)DBG_PRINT
                rbuf += 4;
                m -= 4;
                break;
            case kSExpiryInterval:
                property_rev_.sexp_intrvl = ntohl(*(uint32_t*)rbuf);
                PRINT_DBG(1)("sexp_intrvl=%d\n", property_rev_.sexp_intrvl)DBG_PRINT
                rbuf += 4;
                m -= 4;
                break;
            case kAsgClientID:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.asg_clnt_id, rbuf, si);
                PRINT_DBG(1)("asg_clnt_id=%s\n", &property_rev_.asg_clnt_id[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kSvrKeepAlive:
                property_rev_.svr_kp_alv = ntohl(*(uint16_t*)rbuf);
                PRINT_DBG(1)("svr_kp_alv=%d\n", property_rev_.svr_kp_alv)DBG_PRINT
                rbuf += 2;
                m -= 2;
                break;
            case kAuthenticMethod:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.authent_mtd, rbuf, si);
                PRINT_DBG(1)("authent_mtd=%s\n", &property_rev_.authent_mtd[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kAuthenticData:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.authent_data, rbuf, si);
                PRINT_DBG(1)("authent_data=%s\n", &property_rev_.authent_data[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kReqProblemInfo:
                property_rev_.req_prblm_inf = *rbuf;
                PRINT_DBG(1)("req_rereq_prblm_infsp_inf=%d\n", property_rev_.req_prblm_inf)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kWillDelayIntrv:
                property_rev_.wl_dly_intrvl = ntohl(*(uint32_t*)rbuf);
                PRINT_DBG(1)("wl_dly_intrvl=%d\n", property_rev_.wl_dly_intrvl)DBG_PRINT
                rbuf += 4;
                m -= 4;
                break;
            case kReqRespInfo:
                property_rev_.req_resp_inf = *rbuf;
                PRINT_DBG(1)("req_resp_inf=%d\n", property_rev_.req_resp_inf)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kRespInfo:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.resp_inf, rbuf, si);
                PRINT_DBG(1)("resp_inf=%s\n", &property_rev_.resp_inf[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kSvrReference:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.svr_ref, rbuf, si);
                PRINT_DBG(1)("svr_ref=%s\n", &property_rev_.svr_ref[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kReasonStr:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.reasn_str, rbuf, si);
                PRINT_DBG(1)("reasn_str=%s\n", &property_rev_.reasn_str[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kReceiveMax:
                property_rev_.rec_max = ntohl(*(uint16_t*)rbuf);
                PRINT_DBG(1)("rec_max=%d\n", property_rev_.rec_max)DBG_PRINT
                rbuf += 2;
                m -= 2;
                break;
            case kTopicAliasMax:
                property_rev_.top_ala_max = ntohl(*(uint16_t*)rbuf);
                PRINT_DBG(1)("top_ala_max=%d\n", property_rev_.top_ala_max)DBG_PRINT
                rbuf += 2;
                m -= 2;
                break;
            case kTopicAlias:
                property_rev_.top_ala = ntohl(*(uint16_t*)rbuf);
                PRINT_DBG(1)("top_ala=%d\n", property_rev_.top_ala)DBG_PRINT
                rbuf += 2;
                m -= 2;
                break;
            case kMaxQoS:
                property_rev_.max_qos = *rbuf;
                PRINT_DBG(1)("max_qos=%d\n", property_rev_.max_qos)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kRetainAvailable:
                property_rev_.retn_avlb = *rbuf;
                PRINT_DBG(1)("retn_avlb=%d\n", property_rev_.retn_avlb)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kUserPropety:
                si = ntohl(*(uint16_t*)rbuf);
                rbuf += 2;
                DynAssignVal(&property_rev_.usr_prpty, rbuf, si);
                PRINT_DBG(1)("usr_prpty=%s\n", &property_rev_.usr_prpty[2])DBG_PRINT
                rbuf += si;
                m -= si+2;
                break;
            case kMaxPackSize:
                property_rev_.max_pack_sz = ntohl(*(uint32_t*)rbuf);
                PRINT_DBG(1)("max_pack_sz=%d\n", property_rev_.max_pack_sz)DBG_PRINT
                rbuf += 4;
                m -= 4;
                break;
            case kWildcardSubsAvlb:
                property_rev_.wld_sub_avlb = *rbuf;
                PRINT_DBG(1)("wld_sub_avlb=%d\n", property_rev_.wld_sub_avlb)DBG_PRINT
                rbuf++;
                m--;
                break;
            case kSubsIDAvlb:
                property_rev_.sub_id_avlb = *rbuf;
                PRINT_DBG(1)("sub_id_avlb=%d\n", property_rev_.sub_id_avlb)DBG_PRINT
                rbuf++;
                m--;
                break;
            case KSharedSubsAvlb:
                property_rev_.shar_sub_avlb = *rbuf;
                PRINT_DBG(1)("shar_sub_avlb=%d\n", property_rev_.shar_sub_avlb)DBG_PRINT
                rbuf++;
                m--;
                break;
            default:
                break;
        }   
    }
    return len;
}

static const uint8_t kCnnctProperty[] = {kSExpiryInterval, kReceiveMax, kMaxPackSize, kTopicAliasMax, 
                        kReqRespInfo, kReqProblemInfo, kUserPropety, kAuthenticMethod, kAuthenticData}; 
static const uint8_t kPblshProperty[] = {kPayloadFormat, kMExpiryInterval, kContentType, kResponseTopic,
                        kCorrelationData, kSubscriptionID, kTopicAlias, kUserPropety}; 
static const uint8_t kDiscnnctProperty[] = {kSExpiryInterval, kSvrReference, kReasonStr, kUserPropety}; 
static const uint8_t kSubscrbProperty[] = {kSubscriptionID, kUserPropety}; 
static const uint8_t kUnsubscrbProperty[] = {kUserPropety}; 
/*!
Get property

    Input:  type -- MQTT Control Packet Type. 
    Output: obuf --
    Return: length of property in bytes
*/
int DataBuf::GetProperty(uint8_t *obuf, CtrlPackType type)
{
    const uint8_t *p;
    uint8_t cnt;
    switch (type) {
        case kCONNECT:
            p = kCnnctProperty;
            cnt = sizeof(kCnnctProperty);
            break;
        case kPUBLISH:
            p = kPblshProperty;
            cnt = sizeof(kPblshProperty);
            break;
        case kDISCONNECT:
            p = kDiscnnctProperty;
            cnt = sizeof(kDiscnnctProperty);
            break;
        case kSUBSCRIBE:
            p = kSubscrbProperty;
            cnt = sizeof(kSubscrbProperty);
            break;
        case kUNSUBSCRIBE:
            p = kUnsubscrbProperty;
            cnt = sizeof(kUnsubscrbProperty);
            break;
        default:
            return 0;
    }
    int len = 0;
    uint16_t si;
    for (int i=0; i<cnt; i++) {
        if (!Property01_[p[i]]) continue;
        pry_buf_[len++] = p[i];
        switch (p[i]) {
            case kPayloadFormat:
                pry_buf_[len] = property_val_.payld_frmt;
                len++;
                break;
            case kMExpiryInterval:
                memcpy(&pry_buf_[len], &property_val_.mexp_intrvl, 4);
                len += 4;
                break;
            case kContentType:
                si = ntohs(*(uint16_t*)property_val_.cont_type);
                memcpy(&pry_buf_[len], property_val_.cont_type, si+2);
                len += si+2;
                break;
            case kResponseTopic:
                si = ntohs(*(uint16_t*)property_val_.resp_top);
                memcpy(&pry_buf_[len], property_val_.resp_top, si+2);
                len += si+2;
                break;
            case kCorrelationData:
                si = ntohs(*(uint16_t*)property_val_.crltn_data);
                memcpy(&pry_buf_[len], property_val_.crltn_data, si+2);
                len += si+2;
                break;
            case kSubscriptionID:
                memcpy(&pry_buf_[len], &property_val_.sub_id, 4);
                len += 4;
                break;
            case kSExpiryInterval:
                memcpy(&pry_buf_[len], &property_val_.sexp_intrvl, 4);
                len += 4;
                break;
            case kAsgClientID:
                si = ntohs(*(uint16_t*)property_val_.asg_clnt_id);
                memcpy(&pry_buf_[len], property_val_.asg_clnt_id, si+2);
                len += si+2;
                break;
            case kSvrKeepAlive:
                memcpy(&pry_buf_[len], &property_val_.svr_kp_alv, 2);
                len += 2;
                break;
            case kAuthenticMethod:
                si = ntohs(*(uint16_t*)property_val_.authent_mtd);
                memcpy(&pry_buf_[len], property_val_.authent_mtd, si+2);
                len += si+2;
                break;
            case kAuthenticData:
                si = ntohs(*(uint16_t*)property_val_.authent_data);
                memcpy(&pry_buf_[len], &property_val_.authent_data, si+2);
                len += si+2;
                break;
            case kReqProblemInfo:
                pry_buf_[len++] = property_val_.req_prblm_inf;
                break;
            case kWillDelayIntrv:
                memcpy(&pry_buf_[len], &property_val_.wl_dly_intrvl, 4);
                len += 4;
                break;
            case kReqRespInfo:
                pry_buf_[len++] = property_val_.req_resp_inf;
                break;
            case kRespInfo:
                si = ntohs(*(uint16_t*)property_val_.resp_inf);
                memcpy(&pry_buf_[len], property_val_.resp_inf, si+2);
                len += si+2;
                break;
            case kSvrReference:
                si = ntohs(*(uint16_t*)property_val_.svr_ref);
                memcpy(&pry_buf_[len], property_val_.svr_ref, si+2);
                len += si+2;
                break;
            case kReasonStr:
                si = ntohs(*(uint16_t*)property_val_.reasn_str);
                memcpy(&pry_buf_[len], property_val_.reasn_str, si+2);
                len += si+2;
                break;
            case kReceiveMax:
                memcpy(&pry_buf_[len], &property_val_.rec_max, 2);
                len += 2;
                break;
            case kTopicAliasMax:
                memcpy(&pry_buf_[len], &property_val_.top_ala_max, 2);
                len += 2;
                break;
            case kTopicAlias:
                memcpy(&pry_buf_[len], &property_val_.top_ala, 2);
                len += 2;
                break;
            case kMaxQoS:
                pry_buf_[len] = property_val_.max_qos;
                len++;
                break;
            case kRetainAvailable:
                pry_buf_[len] = property_val_.retn_avlb;
                len++;
                break;
            case kUserPropety:
                si = ntohs(*(uint16_t*)property_val_.usr_prpty);
                memcpy(&pry_buf_[len], &property_val_.usr_prpty, si+2);
                len += si+2;
                break;
            case kMaxPackSize:
                memcpy(&pry_buf_[len], &property_val_.max_pack_sz, 4);
                len += 4;
                break;
            case kWildcardSubsAvlb:
                pry_buf_[len] = property_val_.wld_sub_avlb;
                len++;
                break;
            case kSubsIDAvlb:
                pry_buf_[len] = property_val_.sub_id_avlb;
                len++;
                break;
            case KSharedSubsAvlb:
                pry_buf_[len] = property_val_.shar_sub_avlb;
                len++;
                break;
            default:
                break;
        }
        if (len > pbuf_sz_-2048) {
            pbuf_sz_ *= 2;
            uint8_t *nbuf = new uint8_t[pbuf_sz_];
            memcpy(nbuf, pry_buf_, len);
            delete [] pry_buf_;
            pry_buf_ = nbuf;
        }
    }
    int m = EncodeVarBint(obuf, len);
    if (len) {
        memcpy(&obuf[m], pry_buf_, len);
    }
    return len+m;
}

/*!
Set property

    Input:  id -- Identifier of property
            val -- property value
            len -- length of val
    Return: 0=socket not connected, 1=mqtt server not connected, 2=mqtt server connected
*/
void DataBuf::SetProperty(MQTTProperty id, void *val, int len)
{
    switch (id) {
        case kPayloadFormat:
            property_val_.payld_frmt = *(uint8_t*)val;
            break;
        case kMExpiryInterval:
            property_val_.mexp_intrvl = htonl(*(uint32_t*)val);
            break;
        case kContentType:
            DynAssignVal(&property_val_.cont_type, val, len);
            break;
        case kResponseTopic:
            DynAssignVal(&property_val_.resp_top, val, len);
            break;
        case kCorrelationData:
            DynAssignVal(&property_val_.crltn_data, val, len);
            break;
        case kSubscriptionID:
            property_val_.sub_id = htonl(*(uint32_t*)val);
            break;
        case kSExpiryInterval:
            property_val_.sexp_intrvl = htonl(*(uint32_t*)val);
            break;
        case kAsgClientID:
            DynAssignVal(&property_val_.asg_clnt_id, val, len);
            break;
        case kSvrKeepAlive:
            property_val_.svr_kp_alv = htons(*(uint16_t*)val);
            break;
        case kAuthenticMethod:
            DynAssignVal(&property_val_.authent_mtd, val, len);
            break;
        case kAuthenticData:
            DynAssignVal(&property_val_.authent_data, val, len);
            break;
        case kReqProblemInfo:
            property_val_.req_prblm_inf = *(uint8_t*)val;
            break;
        case kWillDelayIntrv:
            property_val_.wl_dly_intrvl = htonl(*(uint32_t*)val);
            break;
        case kReqRespInfo:
            property_val_.req_resp_inf = *(uint8_t*)val;
            break;
        case kRespInfo:
            DynAssignVal(&property_val_.resp_inf, val, len);
            break;
        case kSvrReference:
            DynAssignVal(&property_val_.svr_ref, val, len);
            break;
        case kReasonStr:
            DynAssignVal(&property_val_.reasn_str, val, len);
            break;
        case kReceiveMax:
            property_val_.rec_max = htons(*(uint16_t*)val);
            break;
        case kTopicAliasMax:
            property_val_.top_ala_max = htons(*(uint16_t*)val);
            break;
        case kTopicAlias:
            property_val_.top_ala = htons(*(uint16_t*)val);
            break;
        case kMaxQoS:
            property_val_.max_qos = *(uint8_t*)val;
            break;
        case kRetainAvailable:
            property_val_.retn_avlb = *(uint8_t*)val;
            break;
        case kUserPropety:
            DynAssignVal(&property_val_.usr_prpty, val, len);
            break;
        case kMaxPackSize:
            property_val_.max_pack_sz = htonl(*(uint32_t*)val);
            break;
        case kWildcardSubsAvlb:
            property_val_.wld_sub_avlb = *(uint8_t*)val;
            break;
        case kSubsIDAvlb:
            property_val_.sub_id_avlb = *(uint8_t*)val;
            break;
        case KSharedSubsAvlb:
            property_val_.shar_sub_avlb = *(uint8_t*)val;
            break;
        default:
            break;
    }
    Property01_[id] = 1;
}

/*!
Update client identifer into clnt_id_

    Return: length of clnt_id_
*/
int DataBuf::UpClientID()
{
    const char *devid = param_cfg().dev_id();
    int sz = strlen(devid);
    //printf("devid=%s sz=%d\n", devid, sz);
    DynAssignVal(&clnt_id_, devid, sz);
    return sz + 2;
}


