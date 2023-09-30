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
Calculate quadratic spline parameter

    Input:  src --
            scnt -- Number of SV in the current loop
            rcnt -- Number of redundancy SV in the previous loop
*/
void Interpolate::CubicParam(const int src[], size_t scnt, size_t rcnt) 
{
    int n = scnt + rcnt;

    float bak[2];
    for (int i=0; i<2; i++) bak[i] = abc_[i][n-1];
    abc_[0][n-1] = 0;
    abc_[1][n-1] = 1;
    for (int i=1; i<n-1; i++) {
        y_[i] = src[i+1]-src[i]*2+src[i-1];
        y_[i] *= 6;
    }
    y_[n-1] = 0;
    TDMA(m_, y_, abc_[0], abc_[1], abc_[2], n);
    for (int i=0; i<2; i++) abc_[i][n-1] = bak[i];
    
    for (int i=0; i<n-1; i++) {
        a_[i] = src[i];
        b_[i] = src[i+1] -src[i] - m_[i]/2 - (m_[i+1]-m_[i])/6;
        c_[i] = m_[i]/2;
        d_[i] = (m_[i+1]-m_[i])/6;
    }

    if (rcnt>1) {
        scnt++;
        memmove(a_, &a_[rcnt-1], scnt*4);
        memmove(b_, &b_[rcnt-1], scnt*4);
        memmove(c_, &c_[rcnt-1], scnt*4);
        memmove(d_, &d_[rcnt-1], scnt*4);
    }
}

/*!
Calculate quadratic spline parameter

    Input:  m0 --
            src --
            scnt -- Number of SV in the current loop
            rcnt -- Number of redundancy SV in the previous loop
*/
void Interpolate::QuadraticParam(int m0, const int src[], size_t scnt, size_t rcnt) 
{
    int n = scnt + rcnt;
    int m[n];
    m[0] = m0;
    
    for (int i=1; i<n; i++) {
        m[i] = 2*(src[i] - src[i-1]) - m[i-1];
    }
    for (int i=0; i<n-1; i++) {
        a_[i] = src[i];
        b_[i] = m[i];
        c_[i] = (m[i+1]-m[i])/2;
    }
    /*for (int i=0; i<128; i++) {
        printf("%4d:%6d;%6d,%8g,%8g,%8g\n", i, src[i], m[i], a_[i], b_[i], c_[i]);
    }//*/
    if (rcnt>1) {
        scnt++;
        memmove(a_, &a_[rcnt-1], scnt*4);
        memmove(b_, &b_[rcnt-1], scnt*4);
        memmove(c_, &c_[rcnt-1], scnt*4);
    }
    /*for (int i=200; i<208; i++) {
        printf("i=%d. %6.4f %6.4f %6.4f %6.4f\n", i, a_[i], b_[i], c_[i]);
    }*/

}

/*!
Interpolate resample.

    Input:  dcnt -- Number of point into des
            src -- Resample source. -1, 0, 1, ..., scnt-1
            scnt -- splines count in src.
            type -- 0=line, 1=quadratic spline, 2=cubic spline.
    Output: des -- resampled value
*/
void Interpolate::Resample(int des[], int dcnt, const int src[], int scnt, int type)
{
    float dx, y;
    int sdot;
    
    *des = *src;
    des++;
    for (int k = 1; k < dcnt; k++) {
        sdot = k * scnt / dcnt;
        dx = k * scnt;
        dx = dx / dcnt - sdot;
        switch (type) {
            case 1:
                y = a_[sdot] + b_[sdot]*dx + c_[sdot]*dx*dx;
                break;
            case 2:
                y = a_[sdot] + b_[sdot]*dx + c_[sdot]*dx*dx + d_[sdot]*dx*dx*dx;
                break;
            default:
                y = src[sdot] + (src[sdot+1] - src[sdot]) * dx;
                break;
        }
        if(y>=0) y += 0.5;
        else y -= 0.5;
        *des = y;
        des++;
    }
}

/*!
Spline Interpolate.

    Input:  dcnt -- Number of point into des
            src -- Resample source. -rcnt, ..., -2, -1; 0, 1, ..., scnt-1.
            scnt -- Number of SV in the current loop. 0, 1, ..., scnt-1.
            rcnt -- Number of redundancy SV in the previous loop. -rcnt, ..., -2, -1;
            type -- 0=linear, 1=quadratic, 2=cubic
            A -- Derivative of the last point in the previous loop
    Output: des -- Resampled value
            A -- Derivative of the last point in the current loop
*/
void Interpolate::Splines(int des[], int dcnt, const int src[], int scnt, int rcnt, int type, float *A)
{
    if (type==1) {  //quadratic
        QuadraticParam(src[1]-src[0], src, scnt, rcnt); //用 src[1]-src[0]/1 即相邻两点的斜率 近似代替前一个
                                                        //插值周期(10周波)最后一个点的导数，便于代码实现
        //QuadraticParam(0, src, scnt, rcnt);
    } else if (type==2) {   //cubic
        if (scnt+rcnt>max_sz_) {
            printf("%d@%s. scnt+rcnt=%d, out of bounds!!\n", __LINE__, __FUNCTION__, scnt+rcnt);
            return;
        }
        if (A) {    //Natural+Clamped
            abc_[1][0] = 2;
            abc_[2][0] = 1;
            y_[0] = 6*(src[1]-src[0])-6*(*A);
            CubicParam(&src[rcnt-1], scnt, 1);
            *A = b_[scnt-1] + 2*c_[scnt-1] + 3*d_[scnt-1];
        } else {    //Natural
            abc_[1][0] = 1;
            abc_[2][0] = 0;
            y_[0] = 0;
            CubicParam(src, scnt, rcnt);
        } 
    }
    Resample(des, dcnt, &src[rcnt-1], scnt, type);
}

/*!
The tridiagonal matrix algorithm -- TDMA, also known as the Thomas algorithm

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

