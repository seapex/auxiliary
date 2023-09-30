#ifndef _FLICKER_FILTER_H_
#define _FLICKER_FILTER_H_
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

enum kPstSampleRate {PstSR400Hz, PstSR800Hz, PstSR1600Hz, PstSR2560Hz};

//初始化闪变滤波参数
void IniFilterPar(int rate);
//闪变滤波，输出为经过平均处理后的瞬时闪变值
int FlickerFilter(float *des, const int32_t *src, int cnt, int cdx, int phs, float frq);

//Accessors
uint8_t avg_num_flicker();
int16_t prms_zo_f(int cdx, int phs);
//Mutators
void set_block2(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif  //_FLICKER_FILTER_H_
