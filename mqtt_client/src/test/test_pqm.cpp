#include "test_pqm.h"
#include "mqttc_para.h"
#include "mqtt_client.h"
#include "ne_msg_dif.h"
#include "time_cst.h"

/*!
generate clear new energy record mqtt payload.

    Input:  topic -- mqtt topic
    Output: topic --
            tbuf -- mqtt payload
    return: mqtt payload length
*/
int ClearNERec(char *topic, uint8_t *tbuf)
{
    sprintf(topic+strlen(topic), "ctrl");

    uint16_t ver = htons(1);
    memcpy(tbuf, &ver, 2);
    tbuf[3] = 7;
    tbuf[5] = 0xff;    //0=LD1,1=LD2..., 0xff=all LD
    return 6;
}

/*!
Get new energy record status.

    Input:  topic -- mqtt topic
    Output: topic --
            tbuf -- mqtt payload
    return: mqtt payload length
*/
int GetNERecStatus(char *topic, uint8_t *tbuf)
{
    uint16_t chg = 0;
    sprintf(topic+strlen(topic), "ne/status");

    uint16_t ver = htons(0);
    memcpy(tbuf, &ver, 2);    
    for (int i=0; i<4; i++) {
        tbuf += 2;
        *(uint16_t *)tbuf = htons(chg);
    }
    return 10;
}

/*!
Get new energy record.

    Input:  topic -- mqtt topic
    Output: topic --
            tbuf -- mqtt payload
    return: mqtt payload length
*/
int GetNERec(char *topic, uint8_t *tbuf)
{
    sprintf(topic+strlen(topic), "ne/rec");

    NERecTime rec_tm[][3] = {
        { {2023, 6, 13, 15, 3, 15}, {2023, 6, 13, 15, 33, 24}, {2023, 6, 13, 16, 3, 33} },
        { {2023, 6, 14, 11, 3, 15}, {2023, 6, 14, 11, 33, 24}, {2023, 6, 14, 12, 3, 33} }
    };

    uint16_t ver = htons(1);
    memcpy(tbuf, &ver, 2);
    int i = 2;
    tbuf[i++] = 0;  //0=LD1,1=LD2..., 0xff=all LD
    tbuf[i++] = 0;  //number of range, 0=all range
    tbuf[i++] = 0;  //0=bgdata, 1=0~10%, 2=10~20%, ..., 10=90~100%
    tbuf[i++] = 3;  //number of record(<16).  0=all record
    i += 2;
    for (int j=0; j<3; j++) {
        memcpy(&tbuf[i], &rec_tm[0][j], 8);
        i += 8;
    }
    tbuf[i++] = 2;  //0=bgdata, 1=0~10%, 2=10~20%, ..., 10=90~100%
    tbuf[i++] = 3;  //number of record(<16).  0=all record
    i+=2;
    for (int j=0; j<3; j++) {
        memcpy(&tbuf[i], &rec_tm[1][j], 8);
        i += 8;
    }

    return i;
}

/*!
Test pqm mqtt comunication.

    Input:  func -- function code
            mqttc -- mqtt client
*/
void TestPQM(int func, MQTTClient *mqttc)
{
    mqttc_para().SetSubTopic("pq/up/#", 5, 0);
    mqttc->Subscribe();
 
    char topic[64];
    uint8_t tbuf[256];
    int len = 0;
    sprintf(topic, "pq/dn/%s/", mqttc_para().dev_id());
    switch (func) {
        case 1: //clear new energy record
            len = ClearNERec(topic, &tbuf[2]);
            break;
        case 2: //get new energy record status
            len = GetNERecStatus(topic, &tbuf[2]);
            break;
        case 3: //get new energy record
            len = GetNERec(topic, tbuf);
            break;
        default:
            break;
    }
    if (len > 0) mqttc->Publish(2, topic, tbuf, len);
}