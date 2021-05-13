/*! \file parse_optn_flicker.h
    \brief Parse command line option for flicker_tst.
*/
#ifndef _PARSE_OPTN_FLICKER_H_
#define _PARSE_OPTN_FLICKER_H_

#include <stdint.h>
#include "parse_option.h"

enum SubCmdType { kCmdTypeEnd=1 };
enum MainCmdOptnType {  kSpeedTst=2, kAccuracyTst, kStatisTst };

class ParseOptnFlicker:public ParseOption {
public:
    ParseOptnFlicker();
    ~ParseOptnFlicker(){};

    //Accessors
    int a_num() { return a_num_; }
protected:
    int HandleMain(int opt);
    int HandleSubcmd(int opt);
private:
    void InitParam();
    int a_num_; //accuracy test times
};


#endif //_PARSE_OPTN_FLICKER_H_