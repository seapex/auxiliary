/*! \file ne_msg_dif.h
    \brief New Energy Message Data Interchange Format.
    Copyright (c) 2023  Xi'an Boyuu Electric, Inc.
*/
#ifndef _NE_MSG_DIF_H_
#define _NE_MSG_DIF_H_

#include <stdint.h>
#include <sys/time.h>

#include "param_phd.h"

/*! 10-minute record save file structure description
struct NETestSaveFile {
    NETestFileHead file_head;
    //file body
        //data block1
        NETestBlockHead blk_head1;
        NETestRecXXX data1[blk_head1.num];
        //data block2
        NETestBlockHead blk_head2;
        NETestRecXXX data2[blk_head2.num];
        //...
} //*/

struct NETestFileHead { // New energy test data save file head
    uint8_t ver;    // The version of this structure. =0
    uint8_t num;    // Number of data block
    uint8_t reserved;
    uint8_t compress;   // Compression algorithm for file body. 0=No compress, 1=zlib
    uint32_t len[2];    // Length of the file body(exclude size of the head). 
                        // [0-1]:length, compressed length.
};

struct NETestBlockHead { //data block head
    uint16_t type;  //data block type. 0=freq, 1=harmonic, 2=unbalance, 3=voltage deviation, 4=Pst
    uint16_t num;   //number of records in the data block
    uint32_t len;   //length of the data block(exclude size of the head)
    uint8_t reserved[4];
};

struct NETestDataFreq {  //frequency data
    timeval time;
    float freq; //unit:Hz
};

struct NETestDataHarm {  //harmonic data
    timeval time;
    float hru[3][kMaxHarmNum+1];    //voltage harmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-1 no use, 2-50 order.
    float ha[3][kMaxHarmNum+1];     //current harmonic. unit:A. [0-2]:A-C. [0-50]:0-50 order.
    float ihru[3][kMaxHarmNum+1];   //voltage interharmonic ratio. unit:%. [0-2]:A-C. [0-50]:0-50 order.
    float iha[3][kMaxHarmNum+1];    //current interharmonic. unit:A. [0-2]:A-C. [0-50]:0-50 order.
    float thd[3][2];                //total harmonic distortion. unit:%. [0-2]:A-C. [0-1]:voltage,current
};

struct NETestDataUnblc { //unbalance data
    timeval time;
    float seq[2][3];  //sequence component. [0-1]:voltage,current. unit:V,A; [0-2]:zero,positive,negative 
};

struct NETestDataUdev {  //voltage deviation data
    timeval time;
    float dev_u[3][2];  //deviation voltage. [0-2]:A-C. [0-1]:under, over. unit:V
};

struct NETestDataPst {   //Pst data
    time_t time;
    float pst[3];  //[0-2]:A-C
};

struct NERecTime {  //10-minute record time
    uint16_t year;  //e.g. 2023
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t reserved;
};

struct NETestStatus {   //Status of 10-minute records
    uint8_t ver;    //The version of this structure. =0
    uint8_t ldx;    //LD index. 0~
    uint16_t cnt;   //Count of status changes
    uint8_t rec_num[16];    //Number of 10-minute records. [0-10]:bgdata,0~10%,10~20%,...,90~100%. [11-15]:reserved
    NERecTime times[10][16];    //running record times(end time). [0-9]:0~10%,10~20%,...,90~100%.
    NERecTime bg_tms[255];  //background record times.
};

#endif // _NE_MSG_DIF_H_