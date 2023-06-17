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
    uint16_t buf[2];
    buf[0] = 7;
    buf[1] = 0xff;    //0=LD1,1=LD2..., 0xff=all LD
    memcpy(tbuf, buf, 4);
    return 4;
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
    NETestStatus ne_status;
    memset(&ne_status, 0, sizeof(ne_status));
    sprintf(topic+strlen(topic), "ne/status");

    for (int i=0; i<4; i++) {
        *(uint16_t *)tbuf = ne_status.cnt;
        tbuf += 2;
    }
    return 8;
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

    NERecTime rec_tm[] = {
        {2023, 6, 13, 15, 3, 15},
        {2023, 6, 13, 15, 33, 24},
        {2023, 6, 13, 16, 3, 33}
    };

    tbuf[0] = 1;    //0=LD1,1=LD2..., 0xff=all LD
    tbuf[1] = 2;    //0=bgdata, 1=0~10%, 2=10~20%, ..., 10=90~100%, 0xff=all range
    tbuf[2] = 0;    //number of record, ≤8. 0=all record
    for (int i=0; i<tbuf[2]; i++) {
        memcpy(&tbuf[6+i*8], &rec_tm[i], 8);
    }

    return 6+8*tbuf[2];
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
 
    char topic[32];
    uint8_t tbuf[72];
    memset(tbuf, 0, 2);
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
            len = GetNERec(topic, &tbuf[2]);
            break;
        default:
            break;
    }
    if (len > 0) mqttc->Publish(2, topic, tbuf, len+2);
}