/*! \file parse_option.cpp
    \brief Parse command line option.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_optn_scnet.h"
#include "config.h"
#include "commu4scnet.h"

#define MAIN_PROG "commu4scnet"

const char *kCmdName[kCmdTypeEnd] = {
    MAIN_PROG, "batchset"};

//main command
static const char * main_sopts = "hVm:s:g:pu:b:f";
static const option main_lopts[] = {
    { "help",       0, 0, 'h' },
    { "version",    0, 0, 'V' },
    { "mac",        1, 0, 'm' },
    { "setpar",     1, 0, 's' },
    { "getpar",     1, 0, 'g' },
    { "ping",       0, 0, 'p' },
    { "upgrade",    1, 0, 'u' },
    { "upboot",     1, 0, 'b' },
    { "force",      1, 0, 'f' },
    { NULL,         0, 0, 0 },
};
static const char * main_help =
    "Usage: "MAIN_PROG" [option] [mac address]\n"
    "                   [command] [args] [mac address]\n"
    "       -h, --help      Print help information\n"
    "       -V, --version   Print version information\n"
    "       -m MACADDRESS, --mac=MACADDRESS\n"
    "                       Set the MAC address of the device to MACADDRESS\n"
    "       -s CFGFILE, --setpar=CFGFILE\n"
    "                       Set the device parameter that read from CFGFILE\n"
    "       -g CFGFILE, --getpar=CFGFILE\n"
    "                       Get the device parameter, then write to CFGFILE\n"
    "       -p, --ping      MAC packet ping\n"
    "       -u UP_FILE, --upgrade=UP_FILE\n"
    "                       Upgrade app firmware\n"
    "       -b UP_FILE, --upboot=UP_FILE\n"
    "                       Upgrade bootloader firmware\n"
    "       -f, --force     Forced to upgrade\n"
    "\nThe "MAIN_PROG" commands are:\n"
    "   batchset     Batch set the parameters of several devices\n"
    "\nSee '"MAIN_PROG" help <command>' for help information on a specific command.\n";

//batchset command
static const char *bset_sopts = "c:r:m:";
static const option bset_lopts[] = {
    { "chnnlx",     1, 0, 'c' },
    { "ratio",      1, 0, 'r' },
    { "mac",        1, 0, 'm' },
    { NULL,         0, 0, 0 },
};
static const char * bset_help =
    "Usage: "MAIN_PROG" batchset [args] [mac address]\n"
    "       -c, --chnnlx=hexnum\n"
    "                       Channels to be set. bit0-3:channel1-4. 0=not, 1=set\n"
    "       -r, --ratiox=\"p,s p,s p,s p,s\"\n"
    "                       PT or CT of channel1-4\n"
    "       -m, --mac=\"mac1 mac2 mac3 mac4\"\n"
    "                      Lower 24 bits of the MAC address of channel1-4\n";
    
ParseOptnScnet::ParseOptnScnet()
{
    InitParam();
}

void ParseOptnScnet::InitParam()
{
    cmd_tol_ = kCmdTypeEnd;
    cmd_name_ = kCmdName;
    
    short_opts_[kMainProg] = main_sopts;
    long_opts_[kMainProg] = main_lopts;
    help_info_[kMainProg] = main_help;

    short_opts_[kBatchSet] = bset_sopts;
    long_opts_[kBatchSet] = bset_lopts;
    help_info_[kBatchSet] = bset_help;

    pfile_cfg_ = NULL;
    mac_cmd_ = 0;
    force_ = 0;
}

/*!
Handle main command

    Input:  opt -- option character 
*/
int ParseOptnScnet::HandleMain(int opt, char *arg)
{
    uint8_t *mac;
    switch (opt) {
        case 'V':
            printf(MAIN_PROG" version %d.%d.%d\n", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);
            break;
        case 'm':
            mac = mac_[0];
            sscanf(arg, "%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
            mac_cmd_ = kSetMac;
            break;
        case 's':
            strcpy(filename_cfg_, arg);
            pfile_cfg_ = filename_cfg_;
            mac_cmd_ = kSetPar;
            break;
        case 'g':
            strcpy(filename_cfg_, arg);
            pfile_cfg_ = filename_cfg_;
            mac_cmd_ = kGetPar;
            break;
        case 'p':
            mac_cmd_ = kMacPing;
            break;
        case 'u':
            strcpy(filename_cfg_, arg);
            pfile_cfg_ = filename_cfg_;
            mac_cmd_ = kUpApp;
            break;
        case 'b':
            strcpy(filename_cfg_, arg);
            pfile_cfg_ = filename_cfg_;
            mac_cmd_ = kUpBoot;
            break;
        case 'f':
            force_ = 1;
            break;
        default:
            return PrintHelp();
    }
    return 0;
}

/*!
not -x or -xxx argument

    Input:  arg
*/
int ParseOptnScnet::HandleOther(char *arg)
{
    uint8_t *mac = mac_[1];
    sscanf(arg, "%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx%*[:-]%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    
    return 0;
}

/*!
Handle subcommand

    Input:  opt -- option character 
*/
int ParseOptnScnet::HandleSubcmd(int opt, char *arg)
{
    int ret;
    switch (cmd_) {
        case kBatchSet:
            ret = HandleBset(opt);
            break;
        default:
           ret = PrintHelp();
    }
    return ret;
}

int ParseOptnScnet::HandleBset(int opt)
{
    uint8_t ui, i;
    switch (opt) {
        case 'c':
            sscanf(optarg, "%hhx", &ui);
            memset(bset_par_.chnl, 0, sizeof(bset_par_.chnl));
            for (i=0; i<4; i++) {
                if (ui>>i&1) bset_par_.chnl[i] = 1;
            }
            break;
        case 'r':
            sscanf(optarg, "%d,%d %d,%d %d,%d %d,%d", 
                   &bset_par_.trns_rto[0][0], &bset_par_.trns_rto[0][1], &bset_par_.trns_rto[1][0], &bset_par_.trns_rto[1][1],
                   &bset_par_.trns_rto[2][0], &bset_par_.trns_rto[2][1], &bset_par_.trns_rto[3][0], &bset_par_.trns_rto[3][1]);
            break;
        case 'm':
            sscanf(optarg, "%hhx%*[:-]%hhx%*[:-]%hhx %hhx%*[:-]%hhx%*[:-]%hhx %hhx%*[:-]%hhx%*[:-]%hhx %hhx%*[:-]%hhx%*[:-]%hhx", 
                   &bset_par_.mac[0][0], &bset_par_.mac[0][1], &bset_par_.mac[0][2], &bset_par_.mac[1][0], &bset_par_.mac[1][1], &bset_par_.mac[1][2],
                   &bset_par_.mac[2][0], &bset_par_.mac[2][1], &bset_par_.mac[2][2], &bset_par_.mac[3][0], &bset_par_.mac[3][1], &bset_par_.mac[3][2]);
            break;
        default:
            return PrintHelp(kCmdName[kBatchSet]);
    }
    return 0;
}

