#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
//#include<time.h>
//#include<fcntl.h>
#include <math.h>
#include <ctype.h>
#include "cyc10_save.h"
#include "cyc150_save.h"
#include "loopbuf_sort.h"
#include "generic.h"

/*!
Convert time_t to tm, it's thread-safe
    Input:  src -- source
    Output: des -- destination
*/
void time_t2tm(struct tm *des, const time_t *src)
{
    if (!src||!des) return;
    time_t tmt = *src;
    memcpy(des, gmtime(&tmt), sizeof(tm));
}

/*!
Show help information

    Input:  t -- 1=cycle10, 2=cycle150, 3=cycle10 harmonic
*/
const char *app_name_;
void ShowHelp(int t)
{
    switch (t) {
        case 1:
            printf(" Usage: %s filename index\n", app_name_);
            printf("        %s filename type [orders]\n", app_name_);
            printf("\ttype -- frq, pst, thd, harm orders\n");
            break;
        case 2:
            printf(" Usage: %s filename index [ac_dc]\n", app_name_);
            printf("\tac_dc -- 0=AC, 1=DC\n", app_name_);
            break;
        case 3:
            printf(" Usage: %s filename harm orders\n", app_name_);
            break;
        default:
            printf(" Usage: %s filename(*.150, *.ten or *.status)\n", app_name_);
            break;
    }
    printf("\n");
}

static const int kCyc10Max = 10;
void ShowStatus(char *filename)
{
    LoopBufSort<uint32_t> *status = new LoopBufSort<uint32_t>(kCyc10Max, CompareInt, filename);
    
    int num = status->DataNum();
    printf("num=%d, head=%d, tail=%d\n", num, status->head(), status->tail());
    printf("cust_data= %d, %d, %d, %d\n", status->cust_data(0), status->cust_data(1), status->cust_data(2), status->cust_data(3));
    status->Seek(0);
    uint32_t li;
    for (int i = 0; i < num; i++) {
        if (status->Read(&li) < 0) {
            printf("\nbuffer be empty!");
        } else {
            printf("%04d%02d%02d_%02d%02d\n", (li>>20)+1900, li>>16&0xf, li>>11&0x1f, li>>6&0x1f, li&0x3f);
        }
    }
    delete status;
}

/*!
show 10cycle record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            idx -- index of record
*/
bool ShowRec10(FILE *f_strm, CycxxFileHead *rcdhd, int idx)
{
    MeasVCyc10LD cyc10;
    if (rcdhd->version!=1) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }
    if (idx>rcdhd->count) idx = rcdhd->count;
    if (idx<1) idx = 1;
    fseek(f_strm, sizeof(MeasVCyc10LD)*(idx-1), SEEK_CUR);
    int i = fread(&cyc10, sizeof(cyc10), 1, f_strm);
    if (i != 1) {
        printf("read file error!\n");
        return false;
    }
    //SetTimeZone(8);
    struct tm tmx;
    time_t2tm(&tmx, &cyc10.time.tv_sec);
    printf("index:%d  time:%04d-%02d-%02d %02d:%02d:%02d.%03d\n", idx, tmx.tm_year+1900, tmx.tm_mon+1, tmx.tm_mday,
            tmx.tm_hour, tmx.tm_min, tmx.tm_sec, cyc10.time.tv_usec/1000);
    printf("rms_u = [%6g, %6g, %6g][%6g, %6g, %6g]\n", cyc10.rms_u[0][0], cyc10.rms_u[0][1], cyc10.rms_u[0][2],
                                    cyc10.rms_u[1][0], cyc10.rms_u[1][1], cyc10.rms_u[1][2]);
    printf("rms_i = %6g, %6g, %6g\n", cyc10.rms_i[0], cyc10.rms_i[1], cyc10.rms_i[2]);
    printf("seq   = [%7g, %7g, %7g][%7g, %7g, %7g]\n", cyc10.seq[0][0], cyc10.seq[0][1], cyc10.seq[0][2],
                                    cyc10.seq[1][0], cyc10.seq[1][1], cyc10.seq[1][2]);
    printf("hrm_amp_u =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.hrm_amp_u[0][j], cyc10.hrm_amp_u[1][j], cyc10.hrm_amp_u[2][j]);
    }
    printf("ihrm_amp_u =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.ihrm_amp_u[0][j], cyc10.ihrm_amp_u[1][j], cyc10.ihrm_amp_u[2][j]);
    }
    printf("thd = %7g%%, %7g%%, %7g%%\n", cyc10.thd[0], cyc10.thd[1], cyc10.thd[2]);
    printf("hrm_amp_i =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.hrm_amp_i[0][j], cyc10.hrm_amp_i[1][j], cyc10.hrm_amp_i[2][j]);
    }
    printf("ihrm_amp_i =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.ihrm_amp_i[0][j], cyc10.ihrm_amp_i[1][j], cyc10.ihrm_amp_i[2][j]);
    }
    printf("w   = %7g, %7g, %7g. %7g\n", cyc10.w[0], cyc10.w[1], cyc10.w[2], cyc10.w[3]);
    printf("var = %7g, %7g, %7g. %7g\n", cyc10.var[0], cyc10.var[1], cyc10.var[2], cyc10.var[3]);
    printf("pf  = %7g, %7g, %7g. %7g\n", cyc10.pf[0], cyc10.pf[1], cyc10.pf[2], cyc10.pf[3]);
    printf("frq  = %7g\n", cyc10.frq);
    printf("pst  = %7g, %7g, %7g\n", cyc10.pst[0], cyc10.pst[1], cyc10.pst[2]);
    return true;
}

/*!
show single type of 10cycle record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            type -- data type
            orders -- harmonic orders
*/
bool ShowRec10Sgl(FILE *f_strm, CycxxFileHead *rcdhd, char *type, int orders)
{
    MeasVCyc10LD cyc10;
    if (rcdhd->version!=1) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }
        
    for (int i=0; i<rcdhd->count; i++) {
        int k = fread(&cyc10, sizeof(cyc10), 1, f_strm);
        if (k != 1) {
            printf("read file error!\n");
            return false;
        }
        if (!strcmp(type, "frq")) {
            if (cyc10.frq>=0) printf("%04d frq  = %7g\n", i+1, cyc10.frq);
        } else if (!strcmp(type, "pst")) {
            if (cyc10.pst[0]>=0) printf("%04d pst  = %7g, %7g, %7g\n", i+1, cyc10.pst[0], cyc10.pst[1], cyc10.pst[2]);
        } else if (!strcmp(type, "thd")) {
            printf("%04d thd  = %7g, %7g, %7g\n", i+1, cyc10.thd[0], cyc10.thd[1], cyc10.thd[2]);
        } else if (!strcmp(type, "harm")) {
            if (orders<=0) {
                ShowHelp(3);
                break;
            }
            int j = orders;
            printf("%04d hrm_amp_u  = %7g, %7g, %7g\n", i+1, cyc10.hrm_amp_u[0][j], cyc10.hrm_amp_u[1][j], cyc10.hrm_amp_u[2][j]);
            printf("%04d ihrm_amp_u  = %7g, %7g, %7g\n", i+1, cyc10.ihrm_amp_u[0][j], cyc10.ihrm_amp_u[1][j], cyc10.ihrm_amp_u[2][j]);
            printf("%04d hrm_amp_i  = %7g, %7g, %7g\n", i+1, cyc10.hrm_amp_i[0][j], cyc10.hrm_amp_i[1][j], cyc10.hrm_amp_i[2][j]);
            printf("%04d ihrm_amp_i  = %7g, %7g, %7g\n", i+1, cyc10.ihrm_amp_i[0][j], cyc10.ihrm_amp_i[1][j], cyc10.ihrm_amp_i[2][j]);
        } else {
            printf("unknown data type!\n");
            return false;
        }
    }
    return true;
}

/*!
show 150cycle record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            idx -- index of record
            ac_dc -- 0=AC, 1=DC.
*/
bool ShowRec150(FILE *f_strm, CycxxFileHead *rcdhd, int idx, int ac_dc)
{
    MeasVChnl3s cyc150;
    if (idx>rcdhd->count) idx = rcdhd->count;
    if (idx<1) idx = 1;
    fseek(f_strm, sizeof(MeasVChnl3s)*(idx-1), SEEK_CUR);
    int i = fread(&cyc150, sizeof(cyc150), 1, f_strm);
    if (i != 1) {
        printf("read file error!\n");
        return false;
    }
    //SetTimeZone(8);
    struct tm tmx;
    time_t2tm(&tmx, &cyc150.time);
    printf("index=%d  time:%04d-%02d-%02d %02d:%02d:%02d\n", idx, tmx.tm_year+1900, tmx.tm_mon+1, tmx.tm_mday,
            tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
    printf("rms = [%8.3f %8.3f %8.3f][%8.3f %8.3f %8.3f]\n", cyc150.rms[0][0], cyc150.rms[0][1], cyc150.rms[0][2],
                                    cyc150.rms[1][0], cyc150.rms[1][1], cyc150.rms[1][2]);
    if (!ac_dc) {
        printf("seq = %8.3f %8.3f %8.3\n", cyc150.ac.seq[0], cyc150.ac.seq[1], cyc150.ac.seq[2]);
        printf("thd = %7.3f %7.3f %7.3f\n", cyc150.ac.thd[0], cyc150.ac.thd[1], cyc150.ac.thd[2]);
    }
    
    printf("hrm_amp =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %8.3f %8.3f %8.3f\n", j, cyc150.hrm_amp[0][j], cyc150.hrm_amp[1][j], cyc150.hrm_amp[2][j]);
    }
    printf("hrm_ang =\n");
    float ang1 = 630 - cyc150.hrm_ang[0][1];
    int n;
    for (int j = 1; j <= kMaxHarmNum; j++) {
        printf("%2d.", j);
        for (int phs=0; phs<3; phs++) {
            float fi = cyc150.hrm_ang[phs][j] + 90 + ang1*j;
            int n = fi*10+0.5;
            fi = n % 3600;
            printf(" %7.1f", fi/10);
        }
        printf("\n");
    }
    printf("hr =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7.3f %7.3f %7.3f\n", j, cyc150.hr[0][j], cyc150.hr[1][j], cyc150.hr[2][j]);
    }

    return true;
}

/*!
Show record

    Input:  filename
            ext -- file extension
            arg -- command line arguments
*/
bool ShowRec(char *filename, char *ext, char *arg[])
{
    bool ret = false;
    CycxxFileHead rcdhd;
    int i, j, k, idx;
    FILE *f_strm = fopen(filename, "rb");
    if (f_strm) {   //File be opened successfully
        i = fread(&rcdhd, sizeof(rcdhd), 1, f_strm);
        if (i != 1) return false;
        printf("count = %d, compress = %d\n", rcdhd.count, rcdhd.compress);
        printf("=============================================\n");
        if (!strcmp(ext, "l50")) {
            if (IsInt(arg[2])) {
                int adc = 0;
                if(arg[3]) adc = atoi(arg[3]);
                ret = ShowRec150(f_strm, &rcdhd, atoi(arg[2]), adc);
            } else {
                ShowHelp(2);
            }
        } else if (!strcmp(ext, "ten")) {
            if (IsInt(arg[2])) {
                ret = ShowRec10(f_strm, &rcdhd, atoi(arg[2]));
            } else {
                k = 0;
                if (arg[3]) k = atoi(arg[3]);
                ret = ShowRec10Sgl(f_strm, &rcdhd, arg[2], k);
            }
        }
    } else {
        printf("Open %s failure\n", filename);
    }
    return ret;
}

int main (int argc, char *argv[])
{
    app_name_ = argv[0];
    if (argc < 2) {
        ShowHelp(0);
        exit(1);
    }

    int i, j;

    char filename[64];
    strcpy(filename, argv[1]);
    char name[48];
    char ext[8];
    char *pch = strrchr(argv[1], '.');
    if (!pch) {
        printf("File type unknown\n");
        return 1;
    }
    *(pch--) = ' ';
    sscanf(argv[1], "%s%s", name, ext);
    printf("Filename: %s.%s\n", name, ext);
    if (strstr(name, "status")) {
        ShowStatus(filename);
    } else {
        if (argc<3) {
            if (!strcmp(ext, "l50")) {
                ShowHelp(2);
            } else if (!strcmp(ext, "ten")) {
                ShowHelp(1);
            } else {
                printf("Unknown file type\n");
            }
        } else {
            if (!ShowRec(filename, ext, argv)) printf("ShowRec() is failure\n");
        }
    }
}


