#include <stdio.h>

#include "thread_mng.h"
#include "socket_client.h"
#include "debug_info.h"

extern SocketClient *g_sock_mqttc;

/*!
socket for mqtt client
*/
void *ThrdSocketMqttC(void *myarg)
{
    CleanupNode *cnode = (CleanupNode *) myarg;

    printf("%d socket_mqttc thread run...\n", cnode->threadnum);
    for ( ; ; ) {
        g_thread_cnt[cnode->threadnum]++; //Increase this thread count
        g_sock_mqttc->Run(100);

        if (g_mainq.control.active == QUITCMD) break;
    }
    NoticeClrq(cnode);
    return NULL;
}
