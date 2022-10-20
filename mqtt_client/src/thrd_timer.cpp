/*! \file thrd_timer.h
    \brief Thread for dpqnet_gui timer.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
//#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
//#include <time.h>
//using namespace std;

#include "thread_mng.h"
#include "time_cst.h"
#include "messageq_mqttc.h"
#include "debug_info.h"

void *ThrdTimer(void *myarg)
{
    CleanupNode *cnode = (CleanupNode *) myarg;

    int cnt_dot1s = 0;  //0.1s per one
    printf("%d timer thread run...\n", cnode->threadnum);
    for (;;) {
        msSleep(100);
        if (g_mainq.control.active == QUITCMD) break;

        cnt_dot1s++;
        int minor_type = 0;
        if (cnt_dot1s%600 == 0) { //1minute
            minor_type |= kOneMinute;
            cnt_dot1s = 0;
        }
        if (cnt_dot1s%100 == 0) { //10s
            minor_type |= kTenSecond;
        }
        if (cnt_dot1s%50 == 0) { //5s
            minor_type |= kFiveSecond;
        }
        if (cnt_dot1s%30 == 0) { //3s
            minor_type |= kThreeSecond;
            g_thread_cnt[cnode->threadnum]++; //Increase this thread count
        }
        if (cnt_dot1s%10 == 0) { //1s
            minor_type |= kOneSecond;
            messageq_mqttc().IncreaseCount();
        }
        if (cnt_dot1s%5 == 0) { //0.5s
            minor_type |= kDot5Second;
            if (cnt_dot1s&1) {
                NoticePthread(kTTMain, kPTimerInfo, minor_type, NULL);
            }
        }
        if (cnt_dot1s%2 == 0) { //0.2s
            minor_type |= kDot2Second;
            NoticePthread(kTTMain, kPTimerInfo, minor_type, NULL);
        }
    }
    NoticeClrq(cnode);
    return NULL;
}
