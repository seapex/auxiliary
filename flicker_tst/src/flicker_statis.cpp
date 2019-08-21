/*! \file flicker_meas.cpp
    \brief flicker measurement.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "flicker_statis.h"
#include "generic.h"

/*!
    Input:  tol -- Total number of instantaneous flicker value for Pst statistics
*/
FlickerStatis::FlickerStatis(int tol)
{
    tol_avrg_ = tol;
    
#ifdef PST_UINT32_T
    for (int i=0; i<3; i++) {
        avrgin_[i] = new uint32_t[tol];
    }
    pst_buf_ = new uint32_t[tol];
#else
    for (int i=0; i<3; i++) {
        avrgin_[i] = new float[tol];
    }
    pst_buf_ = new float[tol];
#endif
    memset(pinsert_, 0, sizeof(pinsert_));
}

FlickerStatis::~FlickerStatis()
{
    delete [] pst_buf_;
    for (int i=0; i<3; i++) {
        delete [] avrgin_[i];
    }
}

/*!
Set average instantaneous flicker value

    Input:  avrg -- Average instantaneous flicker value
            phs -- phase. 0-2=A-C
            cnt -- count of avrg.
*/
void FlickerStatis::SetAvrgIns(const float *avrg, int cnt, uint8_t phs)
{
    int p = pinsert_[phs];
	if (p+cnt>tol_avrg_) {
	    int k = tol_avrg_ - p;
#ifdef PST_UINT32_T
	    for (int i=0; i<k; i++) {
	        avrgin_[phs][p+i] = avrg[i]*10000;
	    }
#else
	    memcpy(&avrgin_[phs][p], avrg, sizeof(float)*k);
#endif
	    p = 0;
	    cnt -= k;
	    avrg += k;
	}
#ifdef PST_UINT32_T
	for (int i=0; i<cnt; i++) {
	    avrgin_[phs][p+i] = avrg[i]*10000;
	}
#else
    memcpy(&avrgin_[phs][p], avrg, sizeof(float)*cnt);
#endif
    pinsert_[phs] = p+cnt;
}

template <class T>
void QuickSort(T *dp, int32_t low, int32_t high)
{
	int32_t i, j;
	T d1, d2;

	i = low;
	j=high;
	d1 = dp[(high+low)/2];
	do{
		while(dp[i]<d1&&i<high) i++; //千万不能改为dp[i]<=d1!!!
		while(d1<dp[j]&&j>low) j--;  //千万不能改为d1<=dp[j]!!!
		if(i<=j){
			d2 = dp[i];
			dp[i] = dp[j];
			dp[j] = d2;
			i++; j--;
		}
	}while(i<=j);

	//对子序列进行快速排序
	if(low<j)QuickSort(dp,low,j);
	if(i<high)QuickSort(dp,i,high);
}

/*!
Pst statistics

    Input:  phs -- phase. 0-2=A-C
    Return: Pst
*/
float FlickerStatis::PstStatis(uint8_t phs)
{
	int tol = tol_avrg_;
#ifdef PST_UINT32_T
    memcpy(pst_buf_, avrgin_[phs], sizeof(uint32_t)*tol);
    uint32_t *dp = pst_buf_;
	qsort(dp, tol, sizeof(float), CompareInt);
    //QuickSort(dp, 0, tol-1);
#else
	memcpy(pst_buf_, avrgin_[phs], sizeof(float)*tol);
	float *dp = pst_buf_;
	qsort(dp, tol, sizeof(float), CompareFloat);
    //QuickSort(dp, 0, tol-1);
#endif

	float P01,P1,P3,P10,P50;
	P01 = dp[tol*999/1000];
	P1 = dp[tol*99/100];
	P3 =  dp[tol*97/100];
	P10 = dp[tol*9/10];
	P50 = dp[tol/2];

	float K01,K1,K3,K10,K50;
	K01 = 0.0314; K1 = 0.0525;
	K3 = 0.0657; K10 = 0.28;
	K50 = 0.08;
	
#ifdef PST_UINT32_T
	return sqrt(K01*P01+K1*P1+K3*P3+K10*P10+K50*P50)/100;
#else
	return sqrt(K01*P01+K1*P1+K3*P3+K10*P10+K50*P50);
#endif
}

/*!
Modify Pst accuracy

    Input:  phs -- phase. 0-2=A-C
*/
void FlickerStatis::ModifyPst(int phs)
{
	if(pst_[phs]>0.93){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>1.9){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>2.9){
		pst_[phs] += 0.02;
	}
	if(pst_[phs]>4.9){
		pst_[phs] += 0.02;
	}
}

/*!
Get Pst result

    Input:	phs -- 相别;
            mdfy -- 是否修正,1=修正
    Return:	Pst计算结果
*/
float FlickerStatis::GetPst(short phs, short mdfy)
{
	pst_[phs] = PstStatis(phs);
	if(mdfy) ModifyPst(phs);
	return pst_[phs];
}

