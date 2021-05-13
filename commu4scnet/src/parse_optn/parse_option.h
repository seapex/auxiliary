/*! \file parse_option.h
    \brief Parse command line option.
*/
#ifndef _PARSE_OPTION_H_
#define _PARSE_OPTION_H_

#include <stdint.h>
#include <getopt.h>

const int kMainProg = 0;
const int kMaxNumCmd = 10;

class ParseOption
{
public:
    ParseOption();
    ~ParseOption(){};
    
    int Parse(int argc, char *argv[]);

    //Accessors
    int cmd() { return cmd_; };
    int debug() { return debug_; };

protected:
    int PrintHelp(const char *cmd=0);
    int GetCmdIdx(const char *arg);
    virtual int HandleMain(int opt, char *arg) = 0;
    virtual int HandleSubcmd(int opt, char *arg) = 0;
    virtual int HandleOther(char *arg) = 0;
    
    const char *short_opts_[kMaxNumCmd];
    const option *long_opts_[kMaxNumCmd];
    const char * help_info_[kMaxNumCmd];

    int cmd_tol_;   //total number of command
    const char **cmd_name_; //command name
    int debug_; //0=no debug, 1=debug
    int cmd_;
        
private:
       
};


#endif //_PARSE_OPTION_H_