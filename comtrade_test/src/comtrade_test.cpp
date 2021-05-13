//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "loop_buffer.h"
#include "comtrade_test.h"

typedef struct {    //param of Analog channel
    long An;        //sn
    char ch_id[65]; //channel id
    char ph[3];     //phase
    char ccbm[65];  //Circuit component by monitor
    char uu[33];    //units
    float a,b;      //ax+b
    float skew;     //the channel time skew(in ¦Ìs) from start of sample period
    short min, max;
    float primary, secondary;
    char PS;        //p=primary, s=secondary
} SAChannelPara;

typedef struct {    //param of Analog channel
    long Dn;        //sn
    char ch_id[65]; //channel id
    char ph[3];     //phase
    char ccbm[65];  //Circuit component by monitor
    char y;         //normal state of status channel
} SDChannelPara;

typedef struct {    //param in cfg file
    char station_name[65]; char rec_dev_id[65]; char rev_year[5];
    short TT; short nA; short nD;
    SAChannelPara * achannel_param;
    SDChannelPara * dchannel_param;
    float lf;       //line frequecy
    short nrates; float samp; long endsamp;
    char starttm[32];   //start record wave time(1st point)
    char trigtm[32];    //trigger time
    char ft[7];         //file type
    float timemult;     //time multiplication factor
    char tz_data[6];    //timezone of starttm&trigtm. e.g. 0, +8, -10h30
    char tz_recorder[6];    //timezone of recorder.  e.g. 0, +8, -10h30, x
    char tmq, leap;     //time quality, leap second
} SCfgParam;

/*!
Extract analog channel information

    Input:  p -- analog channel information
    Output: par -- analog channel parameter
*/
void ExtractAChnl(SAChannelPara *par, char *p)
{
        memset(par, 0, sizeof(SAChannelPara));
        char buf[65];
        int n = 0;
        int len = strlen(p);
        for (int i=0; i<13&&n<len; i++) {
            int sz = strcspn(p, ",");
            if (sz) {
                memcpy(buf, p, sz);
                buf[sz] = 0;
                switch(i) {
                    case 0:  sscanf(buf, "%d", &par->An);         break;
                    case 1:  sscanf(buf, "%s", par->ch_id);       break;
                    case 2:  sscanf(buf, "%s", par->ph);          break;
                    case 3:  sscanf(buf, "%s", par->ccbm);        break;
                    case 4:  sscanf(buf, "%s", par->uu);          break;
                    case 5:  sscanf(buf, "%f", &par->a);          break;
                    case 6:  sscanf(buf, "%f", &par->b);          break;
                    case 7:  sscanf(buf, "%f", &par->skew);       break;
                    case 8:  sscanf(buf, "%d", &par->min);        break;
                    case 9:  sscanf(buf, "%d", &par->max);        break;
                    case 10: sscanf(buf, "%f", &par->primary);    break;
                    case 11: sscanf(buf, "%f", &par->secondary);  break;
                    case 12: sscanf(buf, "%c", &par->PS);         break;
                }
            }
            p += sz+1;
            n += sz+1;
        }
        printf("%d,%s,%s,%s,%s,%f,%f,%f,%d,%d,%f,%f,%c\n", par->An, par->ch_id, par->ph, par->ccbm, par->uu, par->a, par->b,
                par->skew, par->min, par->max, par->primary, par->secondary, par->PS);
}

/*!
Extract digital channel information

    Input:  p -- digital channel information
    Output: par -- digital channel parameter
*/
void ExtractDChnl(SDChannelPara *par, char *p)
{
        memset(par, 0, sizeof(SDChannelPara));
        char buf[65];
        int n = 0;
        int len = strlen(p);
        for (int i=0; i<5&&n<len; i++) {
            int sz = strcspn(p, ",");
            if (sz) {
                memcpy(buf, p, sz);
                buf[sz] = 0;
                switch(i) {
                    case 0:  sscanf(buf, "%d", &par->Dn);         break;
                    case 1:  sscanf(buf, "%s", par->ch_id);       break;
                    case 2:  sscanf(buf, "%s", par->ph);          break;
                    case 3:  sscanf(buf, "%s", par->ccbm);        break;
                    case 4:  sscanf(buf, "%c", &par->y);          break;
                }
            }
            p += sz+1;
            n += sz+1;
        }
        //printf("%d,%s,%s,%s,%c\n", par->Dn, par->ch_id, par->ph, par->ccbm, par->y);
}

/*!
Read configuration from .cfg file

    Input:  fname -- filename, exclude extension
    Output: cfg --
    Return: 0=success, 1=failure
*/
int ReadConfig(SCfgParam *cfg, char *fname)
{
    char stri[256];
    sprintf(stri, "%s.cfg", fname);
    FILE *fstrm = fopen(stri, "r");
    if (!fstrm) {
        printf("Cannot open %s\n", stri);
        return 1;
    }

    memset(cfg, 0, sizeof(SCfgParam));
    fscanf(fstrm, "%[^,],%[^,],%[^,\r\n]", cfg->station_name, cfg->rec_dev_id, cfg->rev_year);
    fscanf(fstrm, "%d,%dA,%dD\n", &cfg->TT, &cfg->nA, &cfg->nD);
    
    if (cfg->nA) {
        cfg->achannel_param = new SAChannelPara[cfg->nA];
        for (int i=0; i<cfg->nA; i++) {
            fgets(stri, sizeof(stri), fstrm);
            ExtractAChnl(&cfg->achannel_param[i], stri);
        }
    }
    if (cfg->nD) {
        cfg->dchannel_param = new SDChannelPara[cfg->nD];
        for (int i=0; i<cfg->nD; i++) {
            fgets(stri, sizeof(stri), fstrm);
            ExtractDChnl(&cfg->dchannel_param[i], stri);
        }
    }
    fscanf(fstrm, "%f\n", &cfg->lf);
    fscanf(fstrm, "%d\n", &cfg->nrates);
    fscanf(fstrm, "%f,%d\n", &cfg->samp, &cfg->endsamp);
    fscanf(fstrm, "%s\n", &cfg->starttm);
    fscanf(fstrm, "%s\n", &cfg->trigtm);
    fscanf(fstrm, "%s\n", &cfg->ft);
    fscanf(fstrm, "%f\n", &cfg->timemult);
    fscanf(fstrm, "%[^,],%[^,\r\n]", &cfg->tz_data, &cfg->tz_recorder);
    fscanf(fstrm, "%x,%d\n", &cfg->tmq, &cfg->leap);
    fclose(fstrm);
#if 1
    printf("%s,%s,%s\n", cfg->station_name, cfg->rec_dev_id, cfg->rev_year);
    printf("%d,%d,%d\n", cfg->TT, cfg->nA, cfg->nD);
    printf("%f\n", cfg->lf);
    printf("%d\n", cfg->nrates);
    printf("%f,%d\n", cfg->samp, cfg->endsamp);
    printf("%s\n", cfg->starttm);
    printf("%s\n", cfg->trigtm);
    printf("%s\n", cfg->ft);
    printf("%f\n", cfg->timemult);
    printf("%s,%s\n", cfg->tz_data, cfg->tz_recorder);
    printf("%x,%d\n", cfg->tmq, cfg->leap);
#endif
    return 0;
}

void CleanConfig(SCfgParam *cfg)
{
    if (cfg->achannel_param) {
        delete [] cfg->achannel_param;
    }
    if (cfg->dchannel_param) {
        delete [] cfg->dchannel_param;
    }
}

/*!
Show data from sn to sn+cnt in data file

    Input:  sn -- start sn
            cnt -- count of data
*/
void ShowData(char *fname, int sn, int cnt)
{
    FILE *fstrm = fopen(fname, "rb");
    if (!fstrm) {
        printf("Cannot open %s\n", fname);
        return;
    }
    int n;
    char *pstr = strtok(fname, ".");
    for (;;) {
        if (pstr) {
            SCfgParam cfg;
            if ( !ReadConfig(&cfg, pstr) ) {
                n = cfg.endsamp;
                CleanConfig(&cfg);
                break;
            }
        }
        fseek(fstrm, 0, SEEK_END);
        n = ftell(fstrm)/sizeof(ComtradeData);
        break;
    }
    printf("Total number of SV: %d\n", n);
    n = fseek(fstrm, (sn-1)*sizeof(ComtradeData), SEEK_SET);
    ComtradeData data;
    for (int i=0; i<cnt; i++) {
        n = fread(&data, sizeof(data), 1, fstrm);
        if (n==1) {
            printf("%07d %010d : %06d %06d %06d %06d %06d %06d\n", data.sn, data.tm, data.a[0], data.a[1], data.a[2], data.a[3], data.a[4], data.a[5]);
            continue;
        }
        break;
    }
    fclose(fstrm);
}

/*!
Save comtrade config file

    Input:  fname -- COMTRADE config file name
            cfg -- config parameter
*/
void SaveCfgFile(char *fname, SCfgParam *cfg)
{
    FILE *fstrm = fopen(fname, "wb");
    if (!fstrm) {
        printf("Cannot create %s\n", fname);
        return;
    }

    fprintf(fstrm, "%s,%s,%s\r\n", cfg->station_name, cfg->rec_dev_id, cfg->rev_year);
    fprintf(fstrm, "%d,%dA,%dD\r\n", cfg->TT, cfg->nA, cfg->nD);
    for (int i=0;i<cfg->nA;i++) {
        fprintf(fstrm, "%d,%s,%s,%s,%s,%4.3f,%4.3f,%4.2f,%d,%d,%f,%f,%c\r\n", cfg->achannel_param[i].An,
                cfg->achannel_param[i].ch_id, cfg->achannel_param[i].ph,
                cfg->achannel_param[i].ccbm, cfg->achannel_param[i].uu,
                cfg->achannel_param[i].a, cfg->achannel_param[i].b,
                cfg->achannel_param[i].skew, cfg->achannel_param[i].min,
                cfg->achannel_param[i].max, cfg->achannel_param[i].primary,
                cfg->achannel_param[i].secondary, cfg->achannel_param[i].PS );
    }
    for (int i=0;i<cfg->nD;i++) {
        fprintf(fstrm, "%d,%s,%s,%s,%d\r\n", cfg->dchannel_param[i].Dn,
                cfg->dchannel_param[i].ch_id, cfg->dchannel_param[i].ph, 
                cfg->dchannel_param[i].ccbm, cfg->dchannel_param[i].y);
    }
    fprintf(fstrm, "%2.0f\r\n", cfg->lf);  //Line frequency
    fprintf(fstrm, "%d\r\n", cfg->nrates);
    fprintf(fstrm, "%4.3f,%d\r\n", cfg->samp, cfg->endsamp);
    fprintf(fstrm, "%s\r\n", cfg->starttm); 
    fprintf(fstrm, "%s\r\n", cfg->trigtm); 
    fprintf(fstrm, "%s\r\n",cfg->ft);   //file type
    fprintf(fstrm, "%5.3f\r\n", cfg->timemult);
    fprintf(fstrm, "%s,%s\r\n", cfg->tz_data, cfg->tz_recorder);
    fprintf(fstrm, "%X,%d\r\n", cfg->tmq, cfg->leap);

    fclose(fstrm);
}

/*!
Convert wave to RMS

    Input:  fname -- wave data save file
*/
void Wave2Rms(char *fname)
{
    FILE *fwav = fopen(fname, "rb");
    if (!fwav) {
        printf("Cannot open %s\n", fname);
        return;
    }
    
    for (;;) {
        char *pstr = strtok(fname, ".");
        if (!pstr) break;
        SCfgParam cfg;
        if ( ReadConfig(&cfg, pstr) ) break;
        char rmsname[128];
        sprintf(rmsname, "%s_RMS.dat", pstr);
        FILE *frms = fopen(rmsname, "wb");
        if (!frms) {
            printf("Cannot create %s\n", rmsname);
            break;
        }
        ComtradeData data;
        int cnt = cfg.endsamp;
        LoopBuffer<int32_t> *v1pn2[6];  //v1p^2, square of sample value. [0-2]:Ua-c;[3-5]:Ia-c. v1p--voltage one point.
        int smpr = (cfg.samp+25)/50;
        for (int i=0; i<6; i++) {
            v1pn2[i] = new LoopBuffer<int32_t>(smpr);
        }
        int32_t vn2;
        float sum[6];
        int16_t rms[6];
        uint32_t sn = 0;
        memset(sum, 0, sizeof(sum));
        for (int i=0; i<cnt; i++) {
            int n = fread(&data, sizeof(data), 1, fwav);
            if (n==1) {
                for (int j=0; j<6; j++) {
                    vn2 = data.a[j];
                    vn2 *= data.a[j];
                    v1pn2[j]->Push(&vn2);
                    sum[j] += vn2;
                }
                if (i>=smpr) {
                    for (int j=0; j<6; j++) {
                        v1pn2[j]->Pop(&vn2);
                        sum[j] -= vn2;
                    }
                    if ((i+1)%32==0) {
                        for (int j=0; j<6; j++) {
                            rms[j] = sqrt(sum[j]/smpr);
                        }
                        sn++;
                        fwrite(&sn, sizeof(uint32_t), 1, frms);
                        fwrite(&data.tm, sizeof(uint32_t), 1, frms);
                        fwrite(rms, sizeof(uint16_t), 6, frms);
                    }
                }
            }
        }
        fclose(frms);
        for (int i=0; i<6; i++) {
            delete v1pn2[i];
        }
        cfg.samp /= 32;
        cfg.endsamp = sn;
        sprintf(rmsname, "%s_RMS.cfg", pstr);
        SaveCfgFile(rmsname, &cfg);
        CleanConfig(&cfg);
        break;
    }
    fclose(fwav);
}

