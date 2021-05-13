#ifndef _FLICKER_FILTER_H_
#define _FLICKER_FILTER_H_
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

enum kPstSampleRate {PstSR400Hz, PstSR800Hz, PstSR1600Hz, PstSR2560Hz};

//初始化闪变滤波参数
void IniFilterPar(int rate, float val);
//闪变滤波，输出为经过平均处理后的瞬时闪变值
int FlickerFilter(float *des, const float *src, int cnt, int cdx, int phs);
int avg_num_flicker();
//int avg_num_flicker();
#ifdef __cplusplus
}
#endif

#endif  //_FLICKER_FILTER_H_
