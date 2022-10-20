#include <stdio.h>
#include <string.h>

#include "param_cfg.h"

ParamCfg & param_cfg()
{
    static ParamCfg cfg;
    return cfg;
}

static const char * kConfigFile = "mqtt_client.conf";
enum ParamType {kHostName, kPortName, kDeviceID, kSubTopics, kMultiStr, kMultiMix1, kMultiMix2, kParamTypeEnd};
static const char *kParamName[] = {"HostName", "PortName", "DeviceID", "SubTopics", "MultiStr", "MultiMix1", "MultiMix2"};

ParamCfg::ParamCfg()
{
    LoadParam();
}

ParamCfg::~ParamCfg()
{
    CleanCfgpar();
}

void ParamCfg::CleanCfgpar()
{
    for (int i=0; i<kMaxSubPayNum; i++) {
        if (cfg_para_.sbpay[i].topic) {
            delete [] cfg_para_.sbpay[i].topic;
        }
    }
    memset(&cfg_para_, 0, sizeof(cfg_para_));
}

/*!
Load parameter from config file
*/
void ParamCfg::LoadParam()
{
    FILE *fstrm = fopen(kConfigFile, "r");
    if (fstrm) {
        CleanCfgpar();
        char par_name[64], stri[128], buf[65];
        ConfigPara *par = &cfg_para_;
        int retv, sz;
        for (int n=0; n<100; n++) {
            retv = fscanf(fstrm, "%[^#:=]", par_name);   //read to the ":" or "="
            if (retv==EOF) break;
            int i;
            for (i=0; i<kParamTypeEnd; i++) {
                if ( !strcmp(par_name, kParamName[i]) ) break;
            }
            
            fgets(stri, sizeof(stri), fstrm);
            char *p = stri;
            int len = strlen(p), m = 0;
            switch (i) {
                case kHostName:  //hostname or ip
                    sscanf(stri, "=%s", par->hostname); 
                    break;
                case kPortName: //port number or local filename
                    sscanf(stri, "=%s", par->name); 
                    break;
                case kDeviceID:
                    sscanf(stri, "=%s", par->dev_id); 
                    break;
                case kSubTopics:
                    p++;
                    for (int i=0; i<kMaxSubPayNum && m<len-2; i++) {
                        sz = strcspn(p, ",");
                        //printf("m=%d sz=%d\n", m, sz);
                        if (sz) {
                            memcpy(buf, p, sz);
                            buf[sz] = 0;
                            par->sbpay[i].topic = new char[sz+1];
                            sscanf(buf, "%s", par->sbpay[i].topic);
                            p += sz+1;
                            m += sz+1;
                            sz = strcspn(p, ";");
                            if (sz) {
                                memcpy(buf, p, sz);
                                buf[sz] = 0;
                                sscanf(buf, "%hhx", &par->sbpay[i].option);
                            }
                        } else {
                            sz = strcspn(p, ";");
                        }
                        p += sz+1;
                        m += sz+1;
                    }
                    break;
                case kMultiStr:
                    sscanf(stri, "=%[^,;]%*[,;]%[^,;]%*[,;]%[^,;]%*[,;]%[^,;\r\n]",
                    //sscanf(stri, "=%[^,;],%[^,],%[^,],%[^,\r\n]",
                           par->mltstr[0], par->mltstr[1], par->mltstr[2], par->mltstr[3]);
                    break;
                case kMultiMix1:
                    sscanf(stri, "=%hhd%*[,;]%hd%*[,;]%f%*[,;]%[^,;]%*[,;]%hd",
                           &par->mmx1.par1, &par->mmx1.par2, &par->mmx1.par3, par->mmx1.par4, &par->mmx1.par5);
                    break;
                case kMultiMix2:
                    p++;
                    for (int i=0; i<5&&m<len; i++) {
                        sz = strcspn(p, ",;");
                        if (sz) {
                            memcpy(buf, p, sz);
                            buf[sz] = 0;
                            switch(i) {
                                case 0:  sscanf(buf, "%hhd", &par->mmx2.par1);  break;
                                case 1:  sscanf(buf, "%hd", &par->mmx2.par2);   break;
                                case 2:  sscanf(buf, "%f", &par->mmx2.par3);    break;
                                case 3:  sscanf(buf, "%s", par->mmx2.par4);     break;
                                case 4:  sscanf(buf, "%hd", &par->mmx2.par5);   break;
                            }
                        }
                        p += sz+1;
                        m += sz+1;
                    }
                    break;
                default:
                    break;
            }
        }
        fclose(fstrm);
    } else {
        DefaultPara();
        SaveParam();
    }
    Show4Debug();
}

/*!
Make parameter default value
*/
void ParamCfg::DefaultPara()
{
    CleanCfgpar();
    ConfigPara *par = &cfg_para_;
    strcpy(par->hostname, "192.168.1.3");
    strcpy(par->name, "1883");
    strcpy(par->dev_id, "PQNet300D000003");
    
    const char *topic="seapex/test1/#";
    int len = strlen(topic);
    for (int i=0; i<2; i++) {
        if (i) {
            topic="seapex/test2/#";
            len = strlen(topic);
        }
        par->sbpay[i].topic = new char[len+1];
        strcpy(par->sbpay[i].topic, topic);
        par->sbpay[i].option = 5;
    }
}

/*!
Save parameter to config file
*/
void ParamCfg::SaveParam()
{
    FILE *fstrm = fopen(kConfigFile, "w");
    if (!fstrm) {
        printf("Cannot create file %s!\n", kConfigFile);
        return;
    }
    
    ConfigPara *par = &cfg_para_;
    fprintf(fstrm, "%s=%s\n", kParamName[kHostName], par->hostname);
    fprintf(fstrm, "%s=%s\n", kParamName[kPortName], par->name);
    fprintf(fstrm, "%s=%s\n", kParamName[kDeviceID], par->dev_id);

    fprintf(fstrm, "%s=", kParamName[kSubTopics]);
    for (int i=0; i<kMaxSubPayNum; i++) {
        if (par->sbpay[i].topic) {
            fprintf(fstrm, "%s,%02X;", par->sbpay[i].topic, par->sbpay[i].option);
        } else {
            fprintf(fstrm, ",;");
        }
    }
    fprintf(fstrm, "\n");
   
    fprintf(fstrm, "%s=%s,%s,%s,%s\n", kParamName[kMultiStr], par->mltstr[0], par->mltstr[1], par->mltstr[2], par->mltstr[3]);
    fprintf(fstrm, "%s=%d,%d,%f,%s,%d\n", kParamName[kMultiMix1], par->mmx1.par1, par->mmx1.par2, par->mmx1.par3, par->mmx1.par4, par->mmx1.par5);
    fprintf(fstrm, "%s=%d,%d,%f,%s,%d\n", kParamName[kMultiMix2], par->mmx2.par1, par->mmx2.par2, par->mmx2.par3, par->mmx2.par4, par->mmx2.par5);

    fclose(fstrm);
}

void ParamCfg::Show4Debug()
{
    ConfigPara *par = &cfg_para_;
    
    printf("%s=%s\n", kParamName[kHostName], par->hostname);
    printf("%s=%s\n", kParamName[kPortName], par->name);
    printf("%s=%s\n", kParamName[kDeviceID], par->dev_id);

    printf("%s=", kParamName[kSubTopics]);
    for (int i=0; i<kMaxSubPayNum; i++) {
        if (par->sbpay[i].topic) {
            printf("%s,%02X;", par->sbpay[i].topic, par->sbpay[i].option);
        } else {
            printf(",;");
        }
    }
    printf("\n");

    printf("%s=%s,%s,%s,%s\n", kParamName[kMultiStr], par->mltstr[0], par->mltstr[1], par->mltstr[2], par->mltstr[3]);
    printf("%s=%d,%d,%f,%s,%d\n", kParamName[kMultiMix1], par->mmx1.par1, par->mmx1.par2, par->mmx1.par3, par->mmx1.par4, par->mmx1.par5);
    printf("%s=%d,%d,%f,%s,%d\n", kParamName[kMultiMix2], par->mmx2.par1, par->mmx2.par2, par->mmx2.par3, par->mmx2.par4, par->mmx2.par5);

}

/*!
Get subscribe palyload parameter

    Output: topics -- Topic Filter[..]
            options -- Options[..]
    Return: number of Topic Filter&Options
*/
int ParamCfg::SubPayload(char **topics, uint8_t *options)
{
    ConfigPara *par = &cfg_para_;
    int num = 0;
    for (int i=0; i<kMaxSubPayNum; i++) {
        if (par->sbpay[i].topic) {
            topics[num] = par->sbpay[i].topic;
            options[num] = par->sbpay[i].option;
            num++;
        }
    }
    return num;
}

