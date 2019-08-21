/*! \file fft_sea.h
    \brief fast fourier transform amended by seapex.
    Copyright (c) 2006-2019 Seapex
*/
#ifndef __FFT_SEA_H_
#define __FFT_SEA_H_

#include "math_ext.h"

#ifdef __cplusplus
extern "C" {
#endif

void FFTBase2(CComplexNum *xy, int cnt, int type);
void FFTBase2Test(CComplexNum *xy, int cnt);

#ifdef __cplusplus
}
#endif

#endif // __FFT_SEA_H_

