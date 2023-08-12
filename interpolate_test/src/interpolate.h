/*! \interpolate.h
    \brief interpolate.
    Copyright (c) 2019 Seapex
*/
#ifndef __INTERPOLATE_H_
#define __INTERPOLATE_H_

#include "math_ext.h"

class Interpolate
{
    float *tdma_buf_;   //buffer for temporarily storing \gamma_i in TDMA()
    int tdma_sz_;       //tdma_buf_ size

    int max_sz_;
    float *abc_[3];
    int *y_;
    float *m_;
    float *a_, *b_, *c_, *d_;
    
    void CubicParam(const int src[], size_t scnt, size_t rcnt);
    void QuadraticParam(int m0, const int src[], size_t scnt, size_t rcnt);
    void Resample(int des[], int dcnt, const int src[], int scnt, int type);
    void TDMA(float x[], const int y[], const float a[], const float b[], const float c[], size_t N);

  public:
    Interpolate(int sz);
    ~Interpolate();
    
    void Splines(int des[], int dcnt, const int src[], int scnt, int rcnt, int type, float *A=NULL);
};

#endif // __INTERPOLATE_H_

