#include <stdio.h>
#include <string.h>

#include "parse_option.h"

ParseOption::ParseOption()
{
    debug_ = 0;
}

/*!
    Return: 0,1=normal, -1=exception, >1=continue
*/
int ParseOption::Parse(int argc, char *argv[])
{
    if (argc<2) return PrintHelp();
    if (strstr(argv[0], cmd_name_[kMainProg])) {
        if ( strcspn(argv[1], "-")==0 ) {   //begin with a hyphen delimiter '-'
            cmd_ = kMainProg;
        } else {
            argc--; argv++;
            if (argc>0) return Parse(argc, argv);
            return -1;
        }
    } else if (!strcmp(argv[0], "help")) {
        if (argc<2) return PrintHelp();
        return PrintHelp(argv[1]);
    } else {
        int idx = GetCmdIdx(argv[0]);
        if (idx<0) {
            return -1;
        } else {
            cmd_ = idx;
        }
    }
    
    const char *sopts = short_opts_[cmd_];
    const option *lopts = long_opts_[cmd_];
    int ret = 0;
    while (1) {
        int c = getopt_long(argc, argv, sopts, lopts, NULL);

        if (c == -1) break;

        if (cmd_==kMainProg) {
            ret = HandleMain(c);
        } else {
            ret = HandleSubcmd(c);
        }
        if (ret != 0) break;
    }
    return ret;
}

int ParseOption::PrintHelp(const char *cmd)
{
    if (cmd==NULL) {
        puts(help_info_[kMainProg]);
        return -1;
    }
    
    int idx = GetCmdIdx(cmd);
    if (idx<0) {
        puts("unknown command!\n");
        return -1;
    } else {
        puts(help_info_[idx]);
    }
    return 0;
}

/*!
Get the index number of the command

    Input:  arg -- name of command
    Variable: cmd_tol_, cmd_name_
    Return: index number of command
*/
int ParseOption::GetCmdIdx(const char *arg)
{
    int i;
    for (i=0; i<cmd_tol_; i++) {
        if (!strcmp(arg, cmd_name_[i])) break;
    }
    if (i<cmd_tol_) {
        return i;
    } else {
        return -1;
    }
}

