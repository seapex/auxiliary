/*! \file flicker_statis.h
    \brief Flicker statistics
    Copyright (c) 2019  Xi'an Boyuu Electric, Inc.
*/
#ifndef _FLICKER_STATIS_H_
#define _FLICKER_STATIS_H_
#include <stdint.h>

#define PST_UINT32_T

#ifdef __cplusplus
extern "C" {
#endif

void IniFlickerStatis(int tol);
void DeFlickerStatis();

void SetAvrgIns(const float *avrg, int cnt, uint8_t phs);
float GetPst(short phs, short mdfy);

//Accessors
int flk_statis_tol_avrg();
    

#ifdef __cplusplus
}
#endif

#endif //_FLICKER_STATIS_H_

