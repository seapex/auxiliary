#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <zlib.h>

#include "ne_msg_dif.h"
#include "time_cst.h"

static const char *kNETypeName[] = {"freq", "harm", "unblc", "udev", "pst", "thd"};
static const char *kNEDisTypeName[] = {"Frequency", "Harmonic", "Unbalance", 
            "Voltage deviation", "Pst", "thd"};

/*!
Show help information
*/
const char *app_name_;
void ShowHelp()
{
    printf(" Usage: %s filename\n", app_name_);
    printf("        %s filename type [param]\n", app_name_);
    printf("\ttype -- %s, %s [orders], %s, %s, %s, %s\n", 
            kNETypeName[0], kNETypeName[1], kNETypeName[2], kNETypeName[3], 
            kNETypeName[4], kNETypeName[5]);
    printf("\n");
    exit(0);
}

/*static const int rec_sz_[5] = {sizeof(NETestDataFreq), sizeof(NETestDataHarm), sizeof(NETestDataUnblc), 
        sizeof(NETestDataUdev), sizeof(NETestDataPst)};
*/

/*!
Show frequency data

    Input:  buf -- buffer of frequency data
            num -- number of frequency data
*/
void ShowFreq(uint8_t *buf, int num)
{
    NETestDataFreq data;
    struct tm tmx;
    for (int i=0; i<num; i++) {
        memcpy(&data, buf, sizeof(data));
        time_t2tm(&tmx, &data.time.tv_sec, 1);
        if (num<1000) {
            printf("%03d %02d:%02d:%02d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        } else {
            printf("%04d %02d:%02d:%02d.%03d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec, data.time.tv_usec/1000);
        }
        printf("%.3f Hz\n", data.freq);
        buf += sizeof(data);
    }
}

/*!
Show unbalance data

    Input:  buf -- buffer of unbalance data
            num -- number of unbalance data
*/
void ShowUnblc(uint8_t *buf, int num)
{
    NETestDataUnblc data;
    struct tm tmx;
    for (int i=0; i<num; i++) {
        memcpy(&data, buf, sizeof(data));
        time_t2tm(&tmx, &data.time.tv_sec, 1);
        printf("%03d %02d:%02d:%02d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        printf("%.3f %.3f %.3f; %.3f %.3f %.3f\n", data.seq[0][0], data.seq[0][2], data.seq[0][1], 
                data.seq[1][0], data.seq[1][2], data.seq[1][1]);
        buf += sizeof(data);
    }
}

/*!
Show voltage deviation data

    Input:  buf -- buffer of voltage deviation data
            num -- number of voltage deviation data
*/
void ShowUdev(uint8_t *buf, int num)
{
    NETestDataUdev data;
    struct tm tmx;
    for (int i=0; i<num; i++) {
        memcpy(&data, buf, sizeof(data));
        time_t2tm(&tmx, &data.time.tv_sec, 1);
        if (num<1000) {
            printf("%03d %02d:%02d:%02d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        } else {
            printf("%04d %02d:%02d:%02d.%03d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec, data.time.tv_usec/1000);
        }
        printf("%.3f %.3f; %.3f %.3f; %.3f %.3f %%\n", data.u_dev[0][0], data.u_dev[0][1], 
                data.u_dev[1][0], data.u_dev[1][1], data.u_dev[2][0], data.u_dev[2][1]);
        buf += sizeof(data);
    }
}

/*!
Show Pst data

    Input:  buf -- buffer of Pst data
            num -- number of Pst data
*/
void ShowPst(uint8_t *buf, int num)
{
    NETestDataPst data;
    struct tm tmx;
    for (int i=0; i<num; i++) {
        memcpy(&data, buf, sizeof(data));
        time_t2tm(&tmx, &data.time, 1);
        printf("%d %02d:%02d:%02d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        printf("%.3f %.3f %.3f\n", data.pst[0], data.pst[1], data.pst[2]);
        buf += sizeof(data);
    }
}

/*!
Show Pst data

    Input:  buf -- buffer of Pst data
            num -- number of Pst data
            type -- data type. 1=harm, 5=thd
            ord -- harmonic orders. 0-50
*/
void ShowHarm(uint8_t *buf, int num, int type, int ord)
{
    NETestDataHarm data;
    struct tm tmx;
    for (int i=0; i<num; i++) {
        memcpy(&data, buf, sizeof(data));
        time_t2tm(&tmx, &data.time.tv_sec, 1);
        printf("%03d %02d:%02d:%02d ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        if (type==1) {  // harmonic
            printf("%.3f %.3f %.3f; %.2f %.2f %.2f; %.3f %.3f %.3f; %.3f %.3f %.3f\n", 
                    data.hru[0][ord], data.hru[1][ord], data.hru[2][ord],
                    data.ha[0][ord], data.ha[1][ord], data.ha[2][ord],
                    data.ihru[0][ord], data.ihru[1][ord], data.ihru[2][ord],
                    data.iha[0][ord], data.iha[1][ord], data.iha[2][ord]);
        } else {    // thd
            printf("%.3f %.3f; %.3f %.3f; %.3f %.3f\n", data.thd[0][0], data.thd[0][1], data.thd[1][0],
                    data.thd[1][1], data.thd[2][0], data.thd[2][1]);
        }
        buf += sizeof(data);
    }
}

/*!
Show a certain type of data

    Input:  buf -- data buffer
            num -- Number of data block
            type -- data type. 0=freq, 1=harm, 2=unblc, 3=udev, 4=pst, 5=thd
            ord -- harmonic orders. 0-50
*/
bool ShowXxxRec(uint8_t *buf, int num, int type, int ord)
{
    NETestBlockHead bhead;
    int t = type==5?1:type;
    int i;
    for (i=0; i<num; i++) {
        memcpy(&bhead, buf, sizeof(bhead));
        if (bhead.type==t) break;
        buf += sizeof(bhead) + bhead.len;
    }
    if (i>=num) {
        printf("No %s data\n", kNEDisTypeName[type]);
        return false;
    }

    printf("%s: %d. ", kNEDisTypeName[bhead.type], bhead.num);
    buf += sizeof(bhead);
    struct tm tmx;
    time_t timex;
    memcpy(&timex, buf, sizeof(timex));
    time_t2tm(&tmx, &timex, 1);
    printf("%04d-%02d-%02d\n", tmx.tm_year+1900, tmx.tm_mon+1, tmx.tm_mday);
    switch (type) {
        case kNEDataFreq:
            ShowFreq(buf, bhead.num);
            break;
        case kNEDataUnblc:
            ShowUnblc(buf, bhead.num);
            break;
        case kNEDataUdev:
            ShowUdev(buf, bhead.num);
            break;
        case kNEDataPst:
            ShowPst(buf, bhead.num);
            break;
        default:
            ShowHarm(buf, bhead.num, type, ord);
            break;
    }
    return true;
}

/*!
show new energy test brief record

    Input:  buf -- data buffer
            num -- Number of data block
*/
void ShowRecBrief(uint8_t *buf, int num)
{
    NETestBlockHead bhead;
    int i, j, k=0, idx;
    for (i=0; i<num; i++) {
        memcpy(&bhead, buf, sizeof(bhead));
        printf("%s: %d\n", kNEDisTypeName[bhead.type], bhead.num);
        buf += sizeof(bhead) + bhead.len;
    }
}

/*!
Read record from file

    Input:  filename
            type -- data type. 0=freq, 1=thd, 2=harm, 3=unblc, 4=udev, 5=pst
            ord -- harmonic orders. 0-50
*/
bool ReadRecord(char *filename, int type, int ord)
{
    NETestFileHead fhead;
    int i, j, k=0, idx;
    FILE *f_strm = fopen(filename, "rb");
    if (f_strm) {   //File be opened successfully
        i = fread(&fhead, sizeof(fhead), 1, f_strm);
        if (i != 1) return false;
        printf("Number of data block = %d, compress = %d, power = %d kW\n", fhead.num, fhead.compress, fhead.rated_pwr);
        printf("=============================================\n");
        uLongf li = fhead.len[0];
        uint8_t *pbuf = new uint8_t[li];
        do {
            if (fhead.compress==1) {
                uint8_t *zbuf = new uint8_t[fhead.len[1]];
                i = fread(zbuf, fhead.len[1], 1, f_strm);
                if (i != 1) return false;
                int k = uncompress(pbuf, &li, zbuf, fhead.len[1]);
                delete [] zbuf;
                if (k != Z_OK) break;
            } else {
                i = fread(pbuf, fhead.len[0], 1, f_strm);
                if (i != 1) break;
            }
            if (type==999) {
                ShowRecBrief(pbuf, fhead.num);
            } else {
                ShowXxxRec(pbuf, fhead.num, type, ord);
            }
        } while(0);
        delete [] pbuf;
        fclose(f_strm);
    } else {
        printf("Open %s failure\n", filename);
    }
    return true;
}

int main (int argc, char *argv[])
{
    app_name_ = argv[0];
    if (argc < 2) {
        ShowHelp();
        exit(1);
    }

    char filename[64], ext[8];
    sscanf(argv[1], "%[^.]%*[.]%s", filename, ext);
    int i = 100;
    sscanf(ext, "ne%d", &i);
    if (i<0 || i>10) {
        printf("Unknown data type\n");
        ShowHelp();
    }
    strcpy(filename, argv[1]);
    int type=999, ord=999;
    if (argv[2]) {
        int i;
        for (i=0; i<6; i++) {
            if (!strcmp(argv[2], kNETypeName[i])) break;
        }
        if (i>=6) {
            printf("Unknown data type\n");
            exit(1);
        }
        type = i;
        if (type==kNEDataHarm) {
            if (argv[3]) {
                sscanf(argv[3], "%d", &ord);
            } else {
                ord = 1;
            }
            if (ord>50) {
                printf("Invalid orders!\n");
                exit(1);
            }
        }
    }
    SetTimeZone(8);
    if (!ReadRecord(filename, type, ord)) printf("ShowRec() is failure\n");
}


