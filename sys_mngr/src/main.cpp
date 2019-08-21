#include <stdio.h>
#include <string.h>
//#include <sys/ioctl.h>
//#include <unistd.h>
//#include <fcntl.h>
#include "watchdog.h"
#include "messageq_guic.h"
#include "socket_client.h"
#include "time_cst.h"
#include "config.h"
//#include "data_buf.h"

/*!
Send system control command

    Input:  type -- command type. 1=quit, 2=cold reboot.
*/
void SystemCtrl(int type)
{
    const int CNNCT_MAX = 2;
    TimeCstInit();
    messageq_guic().InitQGui(CNNCT_MAX);
    g_data_buf = new DataBuf*[CNNCT_MAX];
    memset(g_data_buf, 0, sizeof(DataBuf*)*CNNCT_MAX);
    
    SocketClient *sock = new SocketClient(CNNCT_MAX);
    int ass_idx = sock->sock_idx();
    
    for (int i=0; ; i++) {
        int idx = sock->Start("/tmp/sockgui", NULL, 0, 3, ass_idx);
        if (idx>=0) {
            ass_idx = idx;
            //data_buf_ = g_data_buf[ass_idx];
            break;
        }
        if (i>=2) {
            printf("Connect to service failure!\n");
            return;
        }
        msSleep(2000);
    }
    messageq_guic().PushCtrlSig(ass_idx, kCtrlSystemCtl, type);
    for (int i=0; i<10; i++) {
        if (sock->Run(100)) break;
    }
    delete sock;
    delete [] g_data_buf;
}

enum CmdType {kDefault, kMnQuit=1, kMnColdboot, kMnReboot, kWTDClose, kForceBoot, kUnknown};
int main(int argc, char *argv[])
{
    int cmd=kDefault;
    if (argc<2) {
        printf("version:%d.%d.%d\n", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);
        printf("Usage: %s [ wtd_close | quit | reboot | coldboot | forceboot ]\n", argv[0]);
        return 0;
    }
    if (strcmp(argv[1], "wtd_close")==0) {
        cmd = kWTDClose;
    } else if (strcmp(argv[1], "quit")==0) {
        cmd = kMnQuit;
    } else if (strcmp(argv[1], "coldboot")==0) {
        cmd = kMnReboot;
    } else if (strcmp(argv[1], "reboot")==0) {
        cmd = kMnReboot;
    } else if (strcmp(argv[1], "forceboot")==0) {
        cmd = kMnReboot;
    } else {
        printf("Unknown command!!\n");
        return 0;
    }

    switch (cmd) {
        case kWTDClose:
            watchdog().Disable();
            break;
        case kMnQuit:
        case kMnColdboot:
            SystemCtrl(cmd);
            break;
        case kMnReboot:
            SystemCtrl(1);
            msSleep(2000);
            system("reboot");
            break;
        case kForceBoot:
            watchdog().Enable(1);
            break;
        default:
            break;
    }
}


