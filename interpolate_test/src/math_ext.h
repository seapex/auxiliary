/*! \file math_ext.h
    \brief Extended math function.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _MATH_EXT_H_
#define _MATH_EXT_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <math.h>

static const float kM_PI = 3.14159265;
static const float kSqrt3 = 1.7320508;   //sqrt(3)
static const float kSqrt2 = 1.4142136;   //sqrt(2)

typedef struct {  //custom complex number
    float real; float image; } CComplexNum;
    
typedef struct {  //custom vector
    float amp; float phs; } CVector;    //phs unit:degree

float SquareWave(float time);
void ComplexRotate(CComplexNum *c, int ang);
float ComplexSum(const CComplexNum *c, int cnt);
float ComplexDiff(const CComplexNum *c1, const CComplexNum *c2);
CVector VectorsSum(const CVector *c, int cnt);
CVector VectorsDiff(const CVector *c1, const CVector *c2);

/*!
Returns the arc tangent of y/x

    Input:  y, x
    Return: angle, unit:degree. range[0,360)
*/
inline float ArcTan2(float y, float x)
{
    float ang = atan2f(y, x)*180/kM_PI;
    return ang<0?ang+360:ang;
}

#endif //_MATH_EXT_H_
