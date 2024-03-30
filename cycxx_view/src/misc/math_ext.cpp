/*! \file match_ext.cpp
    \brief Extension mathematical functions.
*/

#include "math_ext.h"
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>

/*!
Generate square wave.
 
    Input:  time -- cycle is 2
    Return: y
*/
float SquareWave(float time)
{
	int t;
    t = time/2;
    t *= 2;
    if (time<0) {   //x,y=(0,-1),-1; [-1,-2),1; [-2,-3),-1; [3,4),1; ...
	    if((t-time)<1) return -1;
    	else return 1;
	} else {    //x,y=[0,1),1; [1,2),-1; [2,3),1; [3,4),-1; ...
        if (time-t<1) return 1;
        else return -1;
    }
}

/*!
Calculate the difference between two complex number

    Input:  c1 -- complex number1
            c2 -- complex number2
    Return:  mod(c1-c2)
*/
float ComplexDiff(const CComplexNum *c1, const CComplexNum *c2)
{
	float fr = c1->real-c2->real;
	float fi = c1->image-c2->image;
	
    return sqrt(fr * fr + fi * fi);
}

/*!
Rotate vector

    Input:  ang -- angle be rotated. 0=120, 1=240
    Output: c -- complex number be rotated
*/
void ComplexRotate(CComplexNum *c, int ang)
{
    float cos_ang, sin_ang;
    if (!ang) {   //+120° i.e. -240°
        cos_ang = -0.5;
        sin_ang = kSqrt3/2;
    } else {        //+240° i.e. -120°
        cos_ang = -0.5;
        sin_ang = -kSqrt3/2;
    }
    float fr = c->real;
    float fi = c->image;
    c->real = fr * cos_ang - fi * sin_ang;
    c->image = fr * sin_ang + fi * cos_ang;
}

/*!
Calculate sum of complex number

    Input:  c -- complex number
            cnt -- count of complex number
    Return: mod(sum(c))
*/
float ComplexSum(const CComplexNum *c, int cnt)
{
    float fr=0, fi=0;
    for (int i=0; i<cnt; i++,c++) {
        fr += c->real;
        fi += c->image;
    }
    
    return sqrt(fr * fr + fi * fi);
}

/*!
Calculate sum of vector

    Input:  c -- vectors
            cnt -- count of vectors
    Return: sum(c)
*/
CVector VectorsSum(const CVector *c, int cnt)
{
    float real, image;
    float fr=0, fi=0;
    for (int i=0; i<cnt; i++,c++) {
        real = c->amp*cosf(c->phs*kM_PI/360);
        image = c->amp*sinf(c->phs*kM_PI/360);
        fr += real;
        fi += image;
    }
    
    CVector vec;
    vec.amp = sqrt(fr * fr + fi * fi);
    vec.phs = ArcTan2(fi, fr);
    return vec;
}

/*!
Calculate the difference between two vectors

    Input:  c1 -- vector1
            c2 -- vector2
    Return: c1-c2
*/
CVector VectorsDiff(const CVector *c1, const CVector *c2)
{
	float fr = c1->amp*cosf(c1->phs*kM_PI/180) - c2->amp*cosf(c2->phs*kM_PI/180);
	float fi = c1->amp*sinf(c1->phs*kM_PI/180) - c2->amp*sinf(c2->phs*kM_PI/180);
	
    CVector vec;
    vec.amp = sqrt(fr * fr + fi * fi);
    vec.phs = ArcTan2(fi, fr);
    return vec;
}

