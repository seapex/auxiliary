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
#include "guic_bufs.h"
#include "phy_prtcl.h"
#include "app_prtcl.h"
//#include "data_buf.h"

/*!
Send system control command

    Input:  type -- command type. 1=quit, 2=cold reboot, 3=update
*/
void SystemCtrl(int type)
{
    const int CNNCT_MAX = 2;
    messageq_guic().InitQueue(CNNCT_MAX);
    guic_bufs().Initialize(CNNCT_MAX);
    SocketClient *sock = new SocketClient(CNNCT_MAX);
    
    int ass_idx = sock->RegistIdx();
    if (ass_idx < 0) {
        printf("Register communication object index(%d) failure!\n", ass_idx);
        return;
    }
    guic_bufs().new_buf(ass_idx);
    if (sock->Start("/tmp/sockgui", NULL, kPhyPrtclPqbC, kAppPrtclGuiC, ass_idx+1) < 0) {
        printf("socket start /tmp/sockgui failed!\n");
        return;
    }
    GuicBuf *data_buf = guic_bufs().buf(ass_idx);
    data_buf->set_sysctl_rsp(251, kCtrlSystemCtl);
    messageq_guic().PushCtrlSig(ass_idx, kCtrlSystemCtl, type);
    int i;
    for (i=0; i<50; i++) {
        sock->Run(100);
        if (data_buf->sysctl_rsp(kCtrlSystemCtl)!=251) break; 
    }
    if (i==50) printf("Command sending failed!!\n");
    delete sock;
    msSleep(1000);
}

enum CmdType {kDefault, kMnQuit=1, kMnColdboot, kUpdate, kMnReboot, kWTDClose, kForceBoot, kUnknown};
int main(int argc, char *argv[])
{
    int cmd=kDefault;
    if (argc<2) {
        printf("version:%d.%d.%d\n", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);
        printf("Usage: %s [ wtd_close | quit | reboot | coldboot | forceboot | update]\n", argv[0]);
        return 0;
    }
    if (strcmp(argv[1], "wtd_close")==0) {
        cmd = kWTDClose;
    } else if (strcmp(argv[1], "quit")==0) {
        cmd = kMnQuit;
    } else if (strcmp(argv[1], "coldboot")==0) {
        cmd = kMnColdboot;
    } else if (strcmp(argv[1], "reboot")==0) {
        cmd = kMnReboot;
    } else if (strcmp(argv[1], "forceboot")==0) {
        cmd = kForceBoot;
    } else if (strcmp(argv[1], "update")==0) {
        cmd = kUpdate;
    } else {
        printf("Unknown command!!\n");
        return 0;
    }

    TimeCstInit();
    switch (cmd) {
        case kMnQuit:
        case kMnColdboot:
            SystemCtrl(cmd);
            break;
        case kUpdate:
            SystemCtrl(cmd);
            msSleep(3000);
            watchdog().Enable(7);
            break;
        case kMnReboot:
            SystemCtrl(1);
            msSleep(2000);
            system("reboot");
            break;
        case kWTDClose:
            watchdog().Disable();
            break;
        case kForceBoot:
            watchdog().Enable(5);
        default:
            break;
    }
    TimeCstEnd();

    switch (cmd) {
        case kForceBoot:
            printf("Force booting...\n");
            watchdog().Feed();
            while(1);
        default:
            break;
    }    
}


