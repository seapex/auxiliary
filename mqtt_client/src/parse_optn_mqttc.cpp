/*! \file parse_option.cpp
    \brief Parse command line option.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_optn_mqttc.h"
#include "config.h"

#define MAIN_PROG "mqtt_client"

const char *kCmdName[kCmdTypeEnd] = {
    MAIN_PROG,
    };

//main command
static const char * main_sopts = "hV:d::crq:t:m:";
static const option main_lopts[] = {
    { "help",    0, 0, 'h' },
    { "debug",   2, 0, 'd' },
    { "pub",     0, 0, 0x80 },
    { "sub",     0, 0, 0x81 },
    { NULL,      0, 0, 0 },
};
static const char * main_help =
    "Usage: "MAIN_PROG" [option]\n"
    "       -h, --help      Print help information\n"
    "       -V protocol-version\n"
    "       -dn --debug=n   Show debug information\n"
    "       -c              clean start\n"
    "       -r              RETAIN=1\n"
    "       -q n            QoS. n=0,1; default to 1\n"
    "       --pub           send PUBLISH\n"
    "       --sub           send SUBSCRIBE\n"
    "       -t topic        PUBLISH topic\n"
    "       -m message      PUBLISH message\n"
;

ParseOptnMqttC::ParseOptnMqttC()
{
    InitParam();
}

ParseOptnMqttC::~ParseOptnMqttC()
{
    if (topic_) delete [] topic_;
    if (msg_) delete [] msg_;
}

void ParseOptnMqttC::InitParam()
{
    cmd_tol_ = kCmdTypeEnd;
    cmd_name_ = kCmdName;
    
    short_opts_[kMainProg] = main_sopts;
    long_opts_[kMainProg] = main_lopts;
    help_info_[kMainProg] = main_help;

    debug_ = 0;
    ver_ = 4;
    clean_ = 0;
    memset(send_, 0, sizeof(send_));
    retain_ = 0;
    qos_ = 1;

    const char *stri = "seapex/test1";
    int n = strlen(stri);
    topic_ = new char[n+1];
    strcpy(topic_, stri);
    stri = "Hello Wednesday(Mercury)!";
    n = strlen(stri);
    msg_ = new char[n+1];
    strcpy(msg_, stri);
}

/*!
Handle main command

    Input:  opt -- option character 
*/
int ParseOptnMqttC::HandleMain(int opt, char *arg)
{
    switch (opt) {
        case 'V':
            if (atoi(optarg)==5) ver_ = 5;
            break;
        case 'd':
            if (optarg){
                debug_ = atoi(optarg);
            } else {
                debug_ = 1;
            }
            //return 2;
            break;
        case 'c':
            clean_ = 1;
            break;
        case 'q':
            if (optarg){
                qos_ = atoi(optarg);
            } else {
                qos_ = 1;
            }
            break;
        case 'r':
            retain_ = 1;
            break;
        case 0x80:
        case 0x81:
            send_[opt-0x80] = 1;
            break;
        case 't':
            if (topic_) delete [] topic_;
            topic_ = new char[strlen(optarg)+1];
            strcpy(topic_, optarg);
            break;
        case 'm':
            if (msg_) delete [] msg_;
            msg_ = new char[strlen(optarg)+1];
            strcpy(msg_, optarg);
            break;
        default:
            return PrintHelp();
    }
    return 0;
}

/*!
Handle subcommand

    Input:  opt -- option character 
*/

int ParseOptnMqttC::HandleSubcmd(int opt, char *arg)
{
    int ret;
    switch (cmd_) {
        default:
           ret = PrintHelp();
    }
    return ret;
}

