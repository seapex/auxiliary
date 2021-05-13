#ifndef _COMTRADE_VIEW_H_
#define _COMTRADE_VIEW_H_
//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

struct ComtradeData{
    uint32_t sn;
    uint32_t tm;
    int16_t a[6];
};

void ShowData(char *fname, int sn, int cnt);
void Wave2Rms(char *fname);

#ifdef __cplusplus
}
#endif

#endif //_COMTRADE_VIEW_H_
