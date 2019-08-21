/*! \file parse_option.cpp
    \brief Parse command line option.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_optn_flicker.h"
#include "config.h"

#define MAIN_PROG "flicker_tst"

const char *kCmdName[kCmdTypeEnd] = {
    MAIN_PROG};

//main command
static const char * main_sopts = "hVsa:t";
static const option main_lopts[] = {
    { "help",    0, 0, 'h' },
    { "version", 0, 0, 'V' },
    { "speed", 0, 0, 's' },
    { "accuracy", 1, 0, 'a' },
    { "statis", 0, 0, 't' },
    { NULL, 0, 0, 0 },
};
static const char * main_help =
    "Usage: "MAIN_PROG" [-V,--version] [-h,--help]\n"
    "       -s, --speed       test pst process speed\n"
    "       -a, --accuracy <num> test pst accuracy\n"
    "       -t, --statis      test pst statistic speed\n";

ParseOptnFlicker::ParseOptnFlicker()
{
    InitParam();
}

void ParseOptnFlicker::InitParam()
{
    cmd_tol_ = kCmdTypeEnd;
    cmd_name_ = kCmdName;
    
    short_opts_[kMainProg] = main_sopts;
    long_opts_[kMainProg] = main_lopts;
    help_info_[kMainProg] = main_help;
}

/*!
Handle main command

    Input:  opt -- option character 
*/
int ParseOptnFlicker::HandleMain(int opt)
{
    switch (opt) {
        case 'V':
            printf(MAIN_PROG" version %d.%d.%d\n", _VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);
            return 1;
        case 's':
            return kSpeedTst;
            break;
        case 'a':
            a_num_ = atoi(optarg);
            return kAccuracyTst;
            break;
        case 't':
            return kStatisTst;
            break;
        default:
            return PrintHelp();
    }
}

/*!
Handle subcommand

    Input:  opt -- option character 
*/

int ParseOptnFlicker::HandleSubcmd(int opt)
{
    int ret;
    switch (cmd_) {

        default:
           ret = PrintHelp();
    }
    return ret;
}


