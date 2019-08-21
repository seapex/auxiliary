#include <string.h>
#include <stdio.h>
#include "interpolate.h"

Interpolate::Interpolate(int sz)
{
    tdma_sz_ = 16;
    tdma_buf_ = new float[tdma_sz_];

    max_sz_ = sz;
    for (int i=0; i<3; i++) {
        abc_[i] = new float[sz];
    }
    for (int i=0; i<2; i++) {
        int k = i*3 + 1;
        for (int j=1; j<sz; j++) {
            abc_[i][j] = k;
        }
    }
    memcpy(abc_[2], abc_[0], sizeof(float)*sz);
    abc_[1][0] = 1;
    abc_[2][0] = 0;
    abc_[0][sz-1] = 0;
    abc_[1][sz-1] = 1;

    y_ = new int[sz];
    m_ = new float[sz];
    a_ = new float[sz];
    b_ = new float[sz];
    c_ = new float[sz];
    d_ = new float[sz];
}

Interpolate::~Interpolate()
{
    delete [] d_;
    delete [] c_;
    delete [] b_;
    delete [] a_;
    delete [] m_;
    delete [] y_;
    for (int i=0; i<3; i++) {
        delete [] abc_[i];
    }
    delete [] tdma_buf_;
}

/*!
The tridiagonal matrix algorithm(TDMA), also known as the Thomas algorithm

    Input:  y, a, b, c
            N -- matrix size
    Output: x
*/
void Interpolate::TDMA(float x[], const int y[], const float a[], const float b[], const float c[], size_t N) 
{
    if (N>tdma_sz_) {
        tdma_sz_ = N;
        delete [] tdma_buf_;
        tdma_buf_ = new float[tdma_sz_];
    }
    float *r = tdma_buf_;
    
    r[0] = c[0] / b[0]; //r1
    x[0] = y[0] / b[0]; //p1
    for (int n=1; n<N; n++) {
        float m = b[n] - a[n] * r[n-1];
        r[n] = c[n] / m;
        x[n] = (y[n] - a[n] * x[n - 1]) / m;
    }
    for (int n=N-1; n-->0; ) {
        x[n] = x[n] - r[n] * x[n + 1];
    }
}

/*!
Cubic Spline Interpolat3

    Input:  src -- resample source
            scnt -- Number of src
    Output: des -- resampled value
            dcnt -- Number of src
*/
void Interpolate::CubicSpline(int des[], int dcnt, const int src[], int scnt)
{
    if (scnt>max_sz_) {
        printf("%d@%s. scnt=%d, out of bounds!!\n", __LINE__, __FUNCTION__, scnt);
        scnt = max_sz_;
    }

    float bak[2];
    for (int i=0; i<2; i++) bak[i] = abc_[i][scnt-1];
    abc_[0][scnt-1] = 0;
    abc_[1][scnt-1] = 1;
    y_[0] = 0;
    for (int i=1; i<scnt-1; i++) {
        y_[i] = src[i+1]-src[i]*2+src[i-1];
        y_[i] *= 6;
    }
    y_[scnt-1] = 0;
    TDMA(m_, y_, abc_[0], abc_[1], abc_[2], scnt);
    for (int i=0; i<2; i++) abc_[i][scnt-1] = bak[i];
    /*
    for (int i=0; i<10; i++) {
        printf("i=%d %d\t%6.4f\n", i, y_[i], m_[i]);
    }*/

    for (int i=0; i<scnt-1; i++) {
        a_[i] = src[i];
        b_[i] = src[i+1] -src[i] - m_[i]/2 - (m_[i+1]-m_[i])/6;
        c_[i] = m_[i]/2;
        d_[i] = (m_[i+1]-m_[i])/6;
    }
    /*
    for (int i=200; i<208; i++) {
        printf("i=%d. %6.4f %6.4f %6.4f %6.4f\n", i, a_[i], b_[i], c_[i], d_[i]);
    }*/
    
    float dx, y;
    int k, sdot;
    *des = *src;
    des++;
    for (k = 1; k < dcnt; k++) {
        sdot = k * scnt / dcnt;
        dx = k * scnt;
        dx = dx / dcnt - sdot;
        y = a_[sdot] + b_[sdot]*dx + c_[sdot]*dx*dx + d_[sdot]*dx*dx*dx;
        if(y>=0) y += 0.5;
        else y -= 0.5;
        *des = y;
        des++;
    }
}

