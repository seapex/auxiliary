#include <complex>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>

#include "commu4scnet.h"
#include "parse_optn_scnet.h"

bool g_doIt = true;

extern int g_test;
int g_test;

/*!
This function handles the ^c, and allows us to cleanup on exit
*/
void ctrlCfun (int i)
{
    printf("ctrlCfun is invoked!\n");
    g_doIt = false;
}

int main (int argc, char *argv[])
{
    ParseOptnScnet parse_opt;
    int ret = parse_opt.Parse(argc, argv);
    if (ret < 0) return ret;

    signal (SIGINT, ctrlCfun);
    CommuForScnet commu;
    int cmd;
    switch (parse_opt.cmd()) {
        case kMainProg:
            cmd = parse_opt.mac_cmd();
            switch (cmd) {
                case kSetMac:
                    commu.SetMacAddr(parse_opt.mac(0));
                    break;
                case kSetPar:
                    commu.SetParam(parse_opt.filename_cfg(), parse_opt.mac(1));
                    break;
                case kGetPar:
                    commu.GetParam(parse_opt.filename_cfg(), parse_opt.mac(1));
                    break;
                case kMacPing:
                    commu.MacPing(parse_opt.mac(1), 1);
                    break;
                case kUpApp:
                case kUpBoot:
                    commu.Upgrade(parse_opt.filename_cfg(), parse_opt.mac(1), cmd, parse_opt.force());
                    break;
                case kSniff:
                    commu.Sniff();
                    break;
                case kDebug:
                    commu.DebugCmd(parse_opt.dbgcmd(), parse_opt.mac(1));
                default:
                    break;
            }
            break;
        case kBatchSet:
            commu.BatchSet(parse_opt.scnet(), parse_opt.trns_rto(), parse_opt.bset_mac(), parse_opt.c1c2(), parse_opt.rllc());
            parse_opt.clr_bset_par();
            break;
        default:
            return -1;
            break;
    }
}


