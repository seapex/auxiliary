/*! \file flicker_statis.h
    \brief Flicker statistics
    Copyright (c) 2019  Xi'an Boyuu Electric, Inc.
*/
#ifndef _FLICKER_STATIS_H_
#define _FLICKER_STATIS_H_
#include <stdint.h>

#define PST_UINT32_T

class FlickerStatis
{
public:
	FlickerStatis(int tol);
	~FlickerStatis();
	
	void SetAvrgIns(const float *avrg, int cnt, uint8_t phs);
    float GetPst(short phs, short mdfy=0);

    //Accessors
    int tol_avrg() { return tol_avrg_; }
protected:
private:
    void ModifyPst(int phs);
    float PstStatis(uint8_t phs);
    
#ifdef PST_UINT32_T
    uint32_t *avrgin_[3];   //Average instantaneous flicker value buffer
    uint32_t *pst_buf_;     //buffer for pst statistics
#else
    float *avrgin_[3];  //Average instantaneous flicker value buffer
    float *pst_buf_;     //buffer for pst statistics
#endif
    int tol_avrg_;      //Total number of average instantaneous flicker value for Pst statistics
    int pinsert_[3];    //The position where the new value will be inserted
    float pst_[3];
};

#endif //_FLICKER_STATIS_H_

