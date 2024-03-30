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
#include <math_ext.h>

enum DataType {kDTpRMS=1, kDTpSeq, kDTpTHD, kDTpHarm, kDTpFrq, kDTpPst, kDTpAvg, kDTpRpl, kDTpLfoRms};

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

    Input:  t -- 1=cycle10, 2=cycle150, 3=LFO RMS
*/
const char *app_name_;
void ShowHelp(int t)
{
    switch (t) {
        case 1:
            printf(" Usage: %s filename index [ac_dc]\n", app_name_);
            printf("        %s filename type [orders] [ac_dc]\n", app_name_);
            printf("\ttype -- rms, seq, thd, harm orders, frq, pst, lfo, avg, rpl\n");
            printf("\tac_dc -- ac, dc\n");
            break;
        case 2:
            printf(" Usage: %s filename index [ac_dc]\n", app_name_);
            printf("        %s filename type [orders] [ac_dc]\n", app_name_);
            printf("\ttype -- rms, seq, thd, harm [orders] avg, rpl\n");
            printf("\tac_dc -- ac, dc\n");
            break;
        default:
            printf(" Usage: %s filename(*.150, *.ten or *.lfo)\n", app_name_);
            break;
    }
    printf("\n");
}

void ShowStatus(char *filename)
{
    LoopBufSort<uint32_t> *status = new LoopBufSort<uint32_t>(8, CompareInt, filename);
    
    int num = status->data_num();
    printf("num=%d\n", num);
    status->set_offset(0);
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
            ac_dc -- 0=AC, 1=DC.
*/
bool ShowRec10(FILE *f_strm, CycxxFileHead *rcdhd, int idx, int ac_dc)
{
    if (rcdhd->version!=2) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }

    if (idx>rcdhd->count) idx = rcdhd->count;
    if (idx<1) idx = 1;
    fseek(f_strm, sizeof(MeasVCyc10LD)*(idx-1), SEEK_CUR);

    MeasVCyc10LD cyc10;
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
    printf("hrm_amp_u =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.hrm_amp_u[0][j], cyc10.hrm_amp_u[1][j], cyc10.hrm_amp_u[2][j]);
    }
    printf("ihrm_amp_u =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.ihrm_amp_u[0][j], cyc10.ihrm_amp_u[1][j], cyc10.ihrm_amp_u[2][j]);
    }
    printf("hrm_amp_i =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.hrm_amp_i[0][j], cyc10.hrm_amp_i[1][j], cyc10.hrm_amp_i[2][j]);
    }
    printf("ihrm_amp_i =\n");
    for (int j = 0; j <= kMaxHarmNum; j++) {
        printf("%2d. %7g, %7g, %7g\n", j, cyc10.ihrm_amp_i[0][j], cyc10.ihrm_amp_i[1][j], cyc10.ihrm_amp_i[2][j]);
    }
    printf("dev_u = [%6g, %6g] [%6g, %6g] [%6g, %6g]\n", cyc10.dev_u[0][0], cyc10.dev_u[0][1], 
                    cyc10.dev_u[1][0], cyc10.dev_u[1][1], cyc10.dev_u[2][0], cyc10.dev_u[2][1]);
    printf("pst  = %7g, %7g, %7g\n", cyc10.pst[0], cyc10.pst[1], cyc10.pst[2]);
    printf("lfo_rms  = %7g, %7g, %7g\n", cyc10.lfo_rms[0], cyc10.lfo_rms[1], cyc10.lfo_rms[2]);

    if (ac_dc) {    //DC
        printf("rms_u = %6g, %6g, %6g\n", cyc10.dc.rms_u[0], cyc10.dc.rms_u[1], cyc10.dc.rms_u[2]);
        printf("rms_i = %6g, %6g, %6g\n", cyc10.dc.rms_i[0], cyc10.dc.rms_i[1], cyc10.dc.rms_i[2]);
        printf("avg_u = %6g, %6g, %6g\n", cyc10.dc.avg_u[0], cyc10.dc.avg_u[1], cyc10.dc.avg_u[2]);
        printf("avg_i = %6g, %6g, %6g\n", cyc10.dc.avg_i[0], cyc10.dc.avg_i[1], cyc10.dc.avg_i[2]);
        printf("seq = %6g, %6g, %6g\n", cyc10.dc.seq[0], cyc10.dc.seq[1], cyc10.dc.seq[2]);
        printf("rpl = [%6g, %6g] [%6g, %6g] [%6g, %6g]\n", cyc10.dc.rpl[0][0], cyc10.dc.rpl[0][1], 
                        cyc10.dc.rpl[1][0], cyc10.dc.rpl[1][1], cyc10.dc.rpl[2][0], cyc10.dc.rpl[2][1]);
        printf("w = %6g, %6g, %6g\n", cyc10.dc.w[0], cyc10.dc.w[1], cyc10.dc.w[2]);
    } else {        //AC
        printf("thd = %7g%%, %7g%%, %7g%%\n", cyc10.ac.thd[0], cyc10.ac.thd[1], cyc10.ac.thd[2]);
        printf("rms_u = [%6g, %6g, %6g][%6g, %6g, %6g]\n", cyc10.ac.rms_u[0][0], cyc10.ac.rms_u[0][1], cyc10.ac.rms_u[0][2],
                                        cyc10.ac.rms_u[1][0], cyc10.ac.rms_u[1][1], cyc10.ac.rms_u[1][2]);
        printf("rms_i = %6g, %6g, %6g\n", cyc10.ac.rms_i[0], cyc10.ac.rms_i[1], cyc10.ac.rms_i[2]);
        printf("seq   = [%7g, %7g, %7g][%7g, %7g, %7g]\n", cyc10.ac.seq[0][0], cyc10.ac.seq[0][1], cyc10.ac.seq[0][2],
                                        cyc10.ac.seq[1][0], cyc10.ac.seq[1][1], cyc10.ac.seq[1][2]);
        printf("w   = %7g, %7g, %7g. %7g\n", cyc10.ac.w[0], cyc10.ac.w[1], cyc10.ac.w[2], cyc10.ac.w[3]);
        printf("var = %7g, %7g, %7g. %7g\n", cyc10.ac.var[0], cyc10.ac.var[1], cyc10.ac.var[2], cyc10.ac.var[3]);
        printf("pf  = %7g, %7g, %7g. %7g\n", cyc10.ac.pf[0], cyc10.ac.pf[1], cyc10.ac.pf[2], cyc10.ac.pf[3]);
        printf("frq  = %7g\n", cyc10.ac.freq);
    }

    return true;
}

/*!
show single type of 10cycle record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            type -- data type
            orders -- harmonic orders
            ac_dc -- 0=AC, 1=DC.
*/
bool ShowRec10Sgl(FILE *f_strm, CycxxFileHead *rcdhd, char *type, int orders, int ac_dc)
{
    if (rcdhd->version!=2) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }

    int tpi;
    printf("SN,  Time,  ");
    if (!strcmp(type, "rms")) {
        if (ac_dc) {    //DC
            printf("U+, U-, U+-; I1, I2, I3i\n");
        } else {        //AC
            printf("Ua, Ub, Uc; Uab, Ubc, Uca; Ia, Ib, Ic\n");
        }
        tpi = kDTpRMS;
    } else if (!strcmp(type, "seq")) {
        if (ac_dc) {    //DC
            printf("U_b, U_u, u%\n");
        } else {        //AC
            printf("Uzero, Upos, Uneg; Izero, Ipos, Ineg\n");
        }
        tpi = kDTpSeq;
    } else if (!strcmp(type, "thd")) {
        if (ac_dc) return false;
        printf("A, B, C\n");
        tpi = kDTpTHD;
    } else if (!strcmp(type, "hrm")) {
        if (ac_dc) return false;
        if (orders<=0) {
            ShowHelp(1);
            return false;
        }
        printf("Ua, Ub, Uc, Ia, Ib, Ic\n");
        tpi = kDTpHarm;
    } else if (!strcmp(type, "frq")) {
        if (ac_dc) return false;
        printf("frequency\n");
        tpi = kDTpFrq;
    } else if (!strcmp(type, "pst")) {
        printf("A, B, C\n");
        tpi = kDTpPst;
    } else if (!strcmp(type, "avg")) {
        if (!ac_dc) return false;
        printf("[U+, U-, U+-]; [I1, I2, I3i]\n");
        tpi = kDTpAvg;
    } else if (!strcmp(type, "rpl")) {
        if (!ac_dc) return false;
        printf("U+[a,r], U-[a,r], U+-[a,r]\n");
        tpi = kDTpRpl;
    } else if (!strcmp(type, "lfo")) {
        if (ac_dc) {
            printf("U+, U-\n");
        } else {
            printf("Ua, Ub, Uc\n");
        }
        tpi = kDTpLfoRms;
    } else {
        printf("unknown data type!\n");
        return false;
    }
    
    MeasVCyc10LD cyc10;
    struct tm tmx;
    for (int i=0; i<rcdhd->count; i++) {
        int k = fread(&cyc10, sizeof(cyc10), 1, f_strm);
        if (k != 1) {
            printf("read file error!\n");
            return false;
        }
        time_t2tm(&tmx, &cyc10.time.tv_sec);
        printf("%04d, %02d:%02d:%02d.%03d, ", i+1,  tmx.tm_hour, tmx.tm_min, tmx.tm_sec, cyc10.time.tv_usec/1000);
        switch (tpi) {
            case kDTpRMS:
                if (ac_dc) {
                    printf("%9g, %9g, %9g; %9g, %9g, %9g\n", cyc10.dc.rms_u[0], cyc10.dc.rms_u[1], cyc10.dc.rms_u[2], 
                                                             cyc10.dc.rms_i[0], cyc10.dc.rms_i[1], cyc10.dc.rms_i[2]);
                } else {
                    printf("%9g, %9g, %9g; %9g, %9g, %9g; %9g, %9g, %9g\n", cyc10.ac.rms_u[0][0], cyc10.ac.rms_u[0][1], cyc10.ac.rms_u[0][2], 
                            cyc10.ac.rms_u[1][0], cyc10.ac.rms_u[1][1], cyc10.ac.rms_u[1][2], cyc10.ac.rms_i[0], cyc10.ac.rms_i[1], cyc10.ac.rms_i[2]);
                }
                break;
            case kDTpSeq:
                if (ac_dc) {
                    printf("%9g, %9g, %9g; %9g, %9g, %9g\n", cyc10.dc.seq[0], cyc10.dc.seq[1], cyc10.dc.seq[2]);
                } else {
                    printf("%9g, %9g, %9g; %9g, %9g, %9g\n", cyc10.ac.seq[0][0], cyc10.ac.seq[0][1], cyc10.ac.seq[0][2], 
                                                             cyc10.ac.seq[1][0], cyc10.ac.seq[1][1], cyc10.ac.seq[1][2]);
                }
                break;
            case kDTpTHD:
                printf("%9g, %9g, %9g\n", cyc10.ac.thd[0], cyc10.ac.thd[1], cyc10.ac.thd[2]);
                break;
            case kDTpHarm:
                printf("%9g, %9g, %9g, %9g, %9g, %9g\n", cyc10.hrm_amp_u[0][orders], cyc10.hrm_amp_u[1][orders], cyc10.hrm_amp_u[2][orders], cyc10.hrm_amp_i[0][orders], cyc10.hrm_amp_i[1][orders], cyc10.hrm_amp_i[2][orders]);
                break;
            case kDTpFrq:
                if (cyc10.ac.freq>=0) printf("%9g\n", cyc10.ac.freq);
                break;
            case kDTpPst:
                if (cyc10.pst[0]>=0) printf("%9g, %9g, %9g\n", cyc10.pst[0], cyc10.pst[1], cyc10.pst[2]);
                break;
            case kDTpAvg:
                printf("[%9g, %9g, %9g]; [%9g, %9g, %9g]\n", cyc10.dc.avg_u[0], cyc10.dc.avg_u[1], cyc10.dc.avg_u[2], 
                                                         cyc10.dc.avg_i[0], cyc10.dc.avg_i[1], cyc10.dc.avg_i[2]);
                break;
            case kDTpRpl:
                printf("[%9g, %9g], [%9g, %9g], [%9g, %9g]\n", cyc10.dc.rpl[0][0], cyc10.dc.rpl[0][1], 
                                    cyc10.dc.rpl[1][0], cyc10.dc.rpl[1][1], cyc10.dc.rpl[2][0], cyc10.dc.rpl[2][1]);
                break;
            case kDTpLfoRms:
                printf("%9g, %9g, %9g\n", cyc10.lfo_rms[0], cyc10.lfo_rms[1], cyc10.lfo_rms[2]);
                break;
            default:
                break;
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
    if (rcdhd->version!=1) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }

    if (idx>rcdhd->count) idx = rcdhd->count;
    if (idx<1) idx = 1;
    fseek(f_strm, sizeof(MeasVChnl3s)*(idx-1), SEEK_CUR);

    MeasVChnl3s cyc150;
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
    printf("dev_u = [%8.3f %8.3f],[%8.3f %8.3f],[%8.3f %8.3f]\n", cyc150.dev_u[0][0], cyc150.dev_u[0][1], 
                            cyc150.dev_u[1][0], cyc150.dev_u[1][1], cyc150.dev_u[2][0], cyc150.dev_u[2][1]);
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

    if (ac_dc) {    //DC
        printf("seq = %8.3f %8.3f %8.3f\n", cyc150.dc.seq[0], cyc150.dc.seq[1], cyc150.dc.seq[2]);
        printf("avg = %7.3f %7.3f %7.3f\n", cyc150.dc.avg[0], cyc150.dc.avg[1], cyc150.dc.avg[2]);
        printf("rpl = [%7.3f %7.3f], [%7.3f %7.3f], [%7.3f %7.3f]\n", cyc150.dc.rpl[0][0], cyc150.dc.rpl[0][1],
                                cyc150.dc.rpl[1][0], cyc150.dc.rpl[1][1], cyc150.dc.rpl[2][0], cyc150.dc.rpl[2][1]);
    } else {        //AC
        printf("seq = %8.3f %8.3f %8.3f\n", cyc150.ac.seq[0], cyc150.ac.seq[1], cyc150.ac.seq[2]);
        printf("thd = %7.3f %7.3f %7.3f\n", cyc150.ac.thd[0], cyc150.ac.thd[1], cyc150.ac.thd[2]);
        printf("thdodd = %7.3f %7.3f %7.3f\n", cyc150.ac.thdodd[0], cyc150.ac.thdodd[1], cyc150.ac.thdodd[2]);
        printf("thdevn = %7.3f %7.3f %7.3f\n", cyc150.ac.thdevn[0], cyc150.ac.thdevn[1], cyc150.ac.thdevn[2]);
    }

    return true;
}

/*!
show single type of 150cycle record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            type -- data type
            orders -- harmonic orders
            ac_dc -- 0=AC, 1=DC.
*/
bool ShowRec150Sgl(FILE *f_strm, CycxxFileHead *rcdhd, char *type, int orders, int ac_dc)
{
    if (rcdhd->version!=1) {
        printf("record version mismatch:%d\n", rcdhd->version);
        return false;
    }

    int tpi;
    printf("SN,Time,");
    if (!strcmp(type, "rms")) {
        if (ac_dc) {    //DC
            printf("U+/I1, U-/I2, U+-/I3i\n");
        } else {        //AC
            printf("[AN, BN, CN]; [AB, BC, CA]\n");
        }
        tpi = kDTpRMS;
    } else if (!strcmp(type, "seq")) {
        if (ac_dc) {    //DC
            printf("U_b, U_u, u%\n");
        } else {        //AC
            printf("zero, pos, neg\n");
        }
        tpi = kDTpSeq;
    } else if (!strcmp(type, "thd")) {
        if (ac_dc) return false;
        printf("A, B, C\n");
        tpi = kDTpTHD;
    } else if (!strcmp(type, "hrm")) {
        if (ac_dc) return false;
        if (orders<=0) {
            ShowHelp(2);
            return false;
        }
        if (orders==1) printf("ampA,ampB,ampC; angA,angB,angC\n");
        else printf("hrA,hrB,hrC; ampA,ampB,ampC; angA,angB,angC\n");
        tpi = kDTpHarm;
    } else if (!strcmp(type, "avg")) {
        if (!ac_dc) return false;
        printf("U+/I1, U-/I2, U+-/I3i\n");
        tpi = kDTpAvg;
    } else if (!strcmp(type, "rpl")) {
        if (!ac_dc) return false;
        printf("U+[a,r], U-[a,r], U+-[a,r]\n");
        tpi = kDTpRpl;
    } else {
        printf("unknown data type!\n");
        return false;
    }

    MeasVChnl3s cyc150;
    struct tm tmx;
    for (int i=0; i<rcdhd->count; i++) {
        int k = fread(&cyc150, sizeof(cyc150), 1, f_strm);
        if (k != 1) {
            printf("read file error!\n");
            return false;
        }
        time_t2tm(&tmx, &cyc150.time);
        printf("%04d, %02d:%02d:%02d, ", i+1, tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        switch (tpi) {
            case kDTpRMS:
                printf("[%9g, %9g, %9g]; [%9g, %9g, %9g]\n", cyc150.rms[0][0], cyc150.rms[0][1], cyc150.rms[0][2], cyc150.rms[1][0], cyc150.rms[1][1], cyc150.rms[1][2]);
                break;
            case kDTpSeq:
                printf("%9g, %9g, %9g\n", cyc150.ac.seq[0], cyc150.ac.seq[1], cyc150.ac.seq[2]);
                break;
            case kDTpTHD:
                printf("%9g, %9g, %9g\n", cyc150.ac.thd[0], cyc150.ac.thd[1], cyc150.ac.thd[2]);
                break;
            case kDTpHarm:
                if (orders==1) printf("%9g, %9g, %9g, %9g, %9g, %9g\n", cyc150.hrm_amp[0][orders], cyc150.hrm_amp[1][orders], cyc150.hrm_amp[2][orders], cyc150.hrm_ang[0][orders], cyc150.hrm_ang[1][orders], cyc150.hrm_ang[2][orders]);
                else printf("%9g, %9g, %9g, %9g, %9g, %9g, %9g, %9g, %9g\n", cyc150.hr[0][orders], cyc150.hr[1][orders], cyc150.hr[2][orders], cyc150.hrm_amp[0][orders], cyc150.hrm_amp[1][orders], cyc150.hrm_amp[2][orders], cyc150.hrm_ang[0][orders], cyc150.hrm_ang[1][orders], cyc150.hrm_ang[2][orders]);
                break;
            case kDTpAvg:
                printf("%9g, %9g, %9g\n", cyc150.dc.avg[0], cyc150.dc.avg[1], cyc150.dc.avg[2]);
                break;
            case kDTpRpl:
                printf("[%9g, %9g], [%9g, %9g], [%9g, %9g]\n", cyc150.dc.rpl[0][0], cyc150.dc.rpl[0][1], 
                        cyc150.dc.rpl[1][0], cyc150.dc.rpl[1][1], cyc150.dc.rpl[2][0], cyc150.dc.rpl[2][1]);
                break;
            default:
                break;
        }
    }
    return true;
}

/*!
show lfo rms record

    Input:  f_strm -- record file stream be opened
            rcdhd -- record file head
            type -- data type
            orders -- harmonic orders
            ac_dc -- 0=AC, 1=DC.
*/
bool ShowRecLfo(FILE *f_strm, CycxxFileHead *rcdhd, int ac_dc)
{
    printf("SN,  Time,  ");

    if (ac_dc) {
        printf("U+, U-\n");
    } else {
        printf("Ua, Ub, Uc\n");
    }
    
    LfoRms3s lfo;
    struct tm tmx;
    for (int i=0; i<rcdhd->count; i++) {
        int k = fread(&lfo, sizeof(lfo), 1, f_strm);
        if (k != 1) {
            printf("read file error!\n");
            return false;
        }
        time_t2tm(&tmx, &lfo.time);
        printf("%04d, %02d:%02d:%02d, ", i+1,  tmx.tm_hour, tmx.tm_min, tmx.tm_sec);
        printf("%9g, %9g, %9g\n", lfo.rms[0], lfo.rms[1], lfo.rms[2]);
    }
    return true;
}

/*!
Show record

    Input:  filename
            ftype -- 1=cycle10, 2=cycle150, 3=LFO rms
            arg -- command line arguments
*/
bool ShowRec(char *filename, int ftype, char *arg[])
{
    bool ret = false;
    CycxxFileHead rcdhd;
    int i, j, k=0, idx;
    FILE *f_strm = fopen(filename, "rb");
    if (f_strm) {   //File be opened successfully
        i = fread(&rcdhd, sizeof(rcdhd), 1, f_strm);
        if (i != 1) return false;
        printf("count = %d, compress = %d\n", rcdhd.count, rcdhd.compress);
        printf("=============================================\n");
        if (ftype==3) {
            int ac_dc = 0;
            if (arg[2]) {
                if(!strcmp(arg[2], "dc")) ac_dc = 1;
            }
            ret = ShowRecLfo(f_strm, &rcdhd, ac_dc);
        } else {
            int ac_dc = 0;
            if (IsInt(arg[2])) {
                if (arg[3]) {
                    if(!strcmp(arg[3], "dc")) ac_dc = 1;
                }
                if (ftype==2) {
                    ret = ShowRec150(f_strm, &rcdhd, atoi(arg[2]), ac_dc);
                } else {
                    ret = ShowRec10(f_strm, &rcdhd, atoi(arg[2]), ac_dc);
                }
            } else {
                if (arg[3]) {
                    if (IsInt(arg[3])) {
                        k = atoi(arg[3]);
                        if (arg[4] && !strcmp(arg[4], "dc")) ac_dc = 1;
                    } else {
                        if(!strcmp(arg[3], "dc")) ac_dc = 1;
                    }
                }
                if (ftype==2) {
                    ret = ShowRec150Sgl(f_strm, &rcdhd, arg[2], k, ac_dc);
                } else {
                    ret = ShowRec10Sgl(f_strm, &rcdhd, arg[2], k, ac_dc);
                }
            }
        }
        fclose(f_strm);
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
        int ftype = 0;
        if (!strcmp(ext, "l50")) {
            ftype = 2;
        } else if (!strcmp(ext, "ten")) {
            ftype = 1;
        } else if (!strcmp(ext, "lfo")) {
            ftype = 3;
        } else {
            printf("Unknown file type\n");
            return 1;
        }

        if (ftype!=3 && argc<3) {
            ShowHelp(ftype);
        } else {
            if (!ShowRec(filename, ftype, argv)) printf("ShowRec() is failure\n");
        }
    }
}


