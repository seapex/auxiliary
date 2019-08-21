#include <math.h>
//#include <fastmath67x.h>
#include <stdint.h>
#include <string.h>
#include "flicker_filter.h"

#define kChannelTol 4   //Total number of channel

/*!
0.05~35HZ Bandpass filter parameter
*/
typedef struct {
    double a[7];   //lowpass coefficient a
    double b[7];   //lowpass coefficient b
    float ah;   //highpass coefficient a
    float bh;   //highpass coefficient b
} BandConst;    //bandpass filter constant coefficient
typedef struct {
    double x[7];   //lowpass variable x
    double y[7];   //lowpass variable y
    float xh[2];   //highpass variable x
    float yh[2];   //highpass variable y
} BandTmpVar;    //bandpass filter temp variable

/*!
Visual sensitivity filter parameter
*/
typedef struct {
    double a[5];   //coefficient a
    double b[5];   //coefficient b
} SensConst;    //Visual sensitivity filter constant coefficient
typedef struct {
    double x[5];   //variable x
    double y[5];   //variable y
} SensTmpVar;    //Visual sensitivity filter temp variable

/*!
RC lowpass filter parameter
*/
typedef struct {
    float K;   //coefficient K
    float a;   //coefficient a
    float b;   //coefficient b
} RCLowConst;    //RC lowpass filter constant coefficient
typedef struct {
    float x[2];   //variable x
    float y[2];   //variable y
} RCLowTmpVar;    //RC lowpass filter temp variable
/*!
DC filter parameter
*/
typedef struct {
    int cnt;    //Count of sampling points in 1s
    float bf;  //dc voltage before square in 1s
	float bfs[20]; //For calculate dc voltage before square
	float bfavg;   //The average of bfs[20]
    float aft;  //dc voltage after square in 1s
	float afts[180];    //For calculate dc voltage after square. must >=2min, it's very important.
	float aftavg;   //The average of afts[20]
	int pbf, paft;  //current point to bfs, afts
} DCFltrTmpVar;     //DC filter temp variable

#pragma DATA_SECTION(band_c_, ".data");
static BandConst band_c_;
#pragma DATA_SECTION(band_v_, ".data");
static BandTmpVar band_v_[kChannelTol][3];

#pragma DATA_SECTION(sens_c_, ".data");
static SensConst sens_c_;
#pragma DATA_SECTION(sens_v_, ".data");
static SensTmpVar sens_v_[kChannelTol][3];

#pragma DATA_SECTION(rclo_c_, ".data");
static RCLowConst rclo_c_;
#pragma DATA_SECTION(rclo_v_, ".data");
static RCLowTmpVar rclo_v_[kChannelTol][3];

#pragma DATA_SECTION(dcf_v_, ".data");
static DCFltrTmpVar dcf_v_[kChannelTol][3];

#pragma DATA_SECTION(avg_num_, ".data");
static int16_t avg_num_;    //The number of instantaneous flicker values used to calculate an average
static int16_t smpl_rate_;   //sampling rate. unit:Hz
int avg_num_flicker() { return avg_num_; }

/*!
Preprocessing before filter

    Input:  src --
            cnt -- Number of sampling points of src
            dc -- DC filter parameter
    Output: des --
*/
void PreFilter(float *des, const float *src, int cnt, DCFltrTmpVar *dc)
{
    float d1;
    int i, n, pos;

    n = 0;
    if ((dc->cnt+cnt)>=smpl_rate_) {
        n = smpl_rate_ - dc->cnt;

        //计算平方前的平均直流分量
        for(i = 0; i < n; i++) {
            dc->bf += src[i];
        }
        pos = dc->pbf;
        dc->bfs[pos] = dc->bf/smpl_rate_;
        pos++;
        dc->pbf = pos>=20?0:pos;
        d1 = 0;
        for (i = 0; i < 20; i++) {  // 直流分量平均值
            d1 +=  dc->bfs[i];
        }
        dc->bfavg = d1/i;
        dc->bf = 0;
        //计算平方后的平均直流分量
        for(i = 0; i < n; i++) {
            des[i] = src[i] - dc->bfavg;  // 当前波动数据减去一个直流分量
            des[i] *= des[i];
            dc->aft += des[i];
        }
        pos = dc->paft;
        dc->afts[pos] = dc->aft/smpl_rate_;   // 平方和的平均值 in 1s
        pos++;
        dc->paft = pos>=180? 0:pos;
        d1 = 0;
        for (i = 0; i < 180; i++) { // 平方后直流分量的平均值
            d1 +=  dc->afts[i];
        }
        dc->aftavg = d1/i;
        dc->aft = 0;
        
        dc->cnt = 0;
    }
    for(i = n; i < cnt; i++) {
        dc->bf += src[i];
    }
    for(i = n; i < cnt; i++) {
        des[i] = src[i] - dc->bfavg;  // 当前波动数据减去一个直流分量
        des[i] *= des[i];
        dc->aft += des[i];
    }
    dc->cnt += (cnt-n);

    //提取出调制波的相对波动, 消除载波幅值对最终结果的影响。
    d1 = dc->aftavg;
    if (d1>0) {
        for(i = 0; i < cnt; i++) {
            des[i] = (des[i]-d1)*50/d1;   //It's very important,Don't modify!!!
        }
    } else {
        memset(des, 0, sizeof(float)*cnt);
    }
}

/*!
0.05Hz~35Hz bandpass filter

    Input:  src --
            cnt -- Number of sampling points of src
            tvr -- temp variable for bandpass filter
    Output: des --
*/
void BandPsFilter(float *des, const float *src, int cnt, BandTmpVar *tvr)
{
    int i;

#pragma MUST_ITERATE(8);
    for(i = 0; i < cnt; i++) {
        //一阶高通滤波(0.05Hz)
        /*tvr->xh[0] = tvr->xh[1];
        tvr->xh[1] = src[i];    //tvr->y[6];
        tvr->yh[0] = tvr->yh[1];
        tvr->yh[1] = band_c_.ah*(tvr->xh[1]-tvr->xh[0]) - band_c_.bh*tvr->yh[0];
        */
        // 六阶巴特沃斯低通滤波(35Hz)
        tvr->x[0] = tvr->x[1];
        tvr->x[1] = tvr->x[2];
        tvr->x[2] = tvr->x[3];
        tvr->x[3] = tvr->x[4];
        tvr->x[4] = tvr->x[5];
        tvr->x[5] = tvr->x[6];
        tvr->x[6] = src[i]; //tvr->yh[1]; 
        tvr->y[0] = tvr->y[1];
        tvr->y[1] = tvr->y[2];
        tvr->y[2] = tvr->y[3];
        tvr->y[3] = tvr->y[4];
        tvr->y[4] = tvr->y[5];
        tvr->y[5] = tvr->y[6];
        tvr->y[6] = band_c_.a[0] * tvr->x[6]
                    + band_c_.a[1] * tvr->x[5]
                    + band_c_.a[2] * tvr->x[4]
                    + band_c_.a[3] * tvr->x[3]
                    + band_c_.a[4] * tvr->x[2]
                    + band_c_.a[5] * tvr->x[1]
                    + band_c_.a[6] * tvr->x[0]
                    - band_c_.b[1] * tvr->y[5]
                    - band_c_.b[2] * tvr->y[4]
                    - band_c_.b[3] * tvr->y[3]
                    - band_c_.b[4] * tvr->y[2]
                    - band_c_.b[5] * tvr->y[1]
                    - band_c_.b[6] * tvr->y[0];

        //一阶高通滤波(0.05Hz)
        tvr->xh[0] = tvr->xh[1];
        tvr->xh[1] = tvr->y[6];
        tvr->yh[0] = tvr->yh[1];
        tvr->yh[1] = band_c_.ah*(tvr->xh[1]-tvr->xh[0]) - band_c_.bh*tvr->yh[0];
        des[i] = tvr->yh[1];
    }
}

/*!
RC lowpass filter

    Input:  src --
            cnt -- Number of sampling points of src
            tvr -- temp variable for RC lowpass filter
    Output: des --
*/
void RCLowPsFilter(float *des, const float *src, int cnt, RCLowTmpVar *tvr)
{
    int i;
#pragma MUST_ITERATE(8);
    for(i = 0; i < cnt; i++) {
        tvr->x[0] = tvr->x[1];
        tvr->x[1] = src[i] * src[i];
        tvr->y[0] = tvr->y[1];
        tvr->y[1] = rclo_c_.K*rclo_c_.a*(tvr->x[1]+tvr->x[0]) - rclo_c_.b*tvr->y[0];

        des[i] = tvr->y[1];
    }
}

/*!
Visual sensitivity filter

    Input:  src --
            cnt -- Number of sampling points of src
            tvr -- temp variable for sens filter
    Output: des --
*/
void SensFilter(float *des, const float *src, int cnt, SensTmpVar *tvr)
{
    int i;

#pragma MUST_ITERATE(8);
    for(i = 0; i < cnt; i++) {
        tvr->x[0] = tvr->x[1];
        tvr->x[1] = tvr->x[2];
        tvr->x[2] = tvr->x[3];
        tvr->x[3] = tvr->x[4];
        tvr->x[4] = src[i];
        tvr->y[0] = tvr->y[1];
        tvr->y[1] = tvr->y[2];
        tvr->y[2] = tvr->y[3];
        tvr->y[3] = tvr->y[4];
        tvr->y[4] = sens_c_.a[0] * tvr->x[4]
                    + sens_c_.a[1] * tvr->x[3]
                    - sens_c_.b[1] * tvr->y[3]
                    + sens_c_.a[2] * tvr->x[2]
                    - sens_c_.b[2] * tvr->y[2]
                    + sens_c_.a[3] * tvr->x[1]
                    - sens_c_.b[3] * tvr->y[1]
                    + sens_c_.a[4] * tvr->x[0]
                    - sens_c_.b[4] * tvr->y[0];

        des[i] = tvr->y[4];
    }
}

/*!
Flicker filter

    Input:  src -- sampling value
            cnt -- Number of sampling points of src
            cdx -- channel index. 0-
            phs -- phase, 0~2=A~C
    Output: des -- Average instantaneous flicker value
    Return: number of output data.
*/
int FlickerFilter(float *des, const float *src, int cnt, int cdx, int phs)
{
    float fi;
    int i, j;
    
    float *pd = des;
    const float *ps = src;
    i = cnt;
    while(i>0) {
        j = i<smpl_rate_?i:smpl_rate_;
        PreFilter(pd, ps, j, &dcf_v_[cdx][phs]);
        BandPsFilter(pd, pd, j, &band_v_[cdx][phs]);
        SensFilter(pd, pd, j, &sens_v_[cdx][phs]);
        RCLowPsFilter(pd, pd, j, &rclo_v_[cdx][phs]);
        i -= j;
        ps += j;
        pd += j;
    }

    for(i = 0; i < cnt/avg_num_; i++) {
        fi = 0;
        for(j = 0; j < avg_num_; j++) {
            fi += des[i * avg_num_ + j];
        }
        des[i] = fi/avg_num_;
    }
    return cnt/avg_num_;
}

/*!
Initialize flicker filter calculation parameters

    Input:  rate -- sampling rate type. refer to kPstSampleRate.
            val -- Initial voltage peak value. e.g. if rms is 57.74V, then peak value is 57.74*1.414
*/
void IniFilterPar(int rate, float val)
{
    int i, j;
    float fi;

    switch (rate) {
        case PstSR800Hz:
			band_c_.a[0] = band_c_.a[6] = 0.000003968763;
			band_c_.a[1] = band_c_.a[5] = 0.00002381258;
			band_c_.a[2] = band_c_.a[4] = 0.00005953145;
			band_c_.a[3] = 0.00007937526;
			band_c_.b[1] = -4.945139;   band_c_.b[2] = 10.26772;
			band_c_.b[3] = -11.44799;   band_c_.b[4] = 7.223983;
			band_c_.b[5] = -2.444865;   band_c_.b[6] = 0.3465408;
			band_c_.ah = 0.9998;        band_c_.bh = -0.99961;
    		sens_c_.a[0] = 0.00259963;  sens_c_.a[1] = 0.0000461345;
	    	sens_c_.a[2] = 0.00515313;  sens_c_.a[3] = -0.0000461345;
		    sens_c_.a[4] = 0.0025535;
    		sens_c_.b[1] = -3.765314;   sens_c_.b[2] = 5.313669;
	    	sens_c_.b[3] = -3.330456;   sens_c_.b[4] = 0.7821078;
		    rclo_c_.a = 0.002079;   rclo_c_.b = -0.99584;  rclo_c_.K = 121;
		    avg_num_ = 20;
		    smpl_rate_ = 800;
            break;
        case PstSR1600Hz:
			band_c_.a[0] = band_c_.a[6] = 8.07852914e-8;
			band_c_.a[1] = band_c_.a[5] = 4.84711748e-7;
			band_c_.a[2] = band_c_.a[4] = 1.21177937e-6;
			band_c_.a[3] = 1.61570583e-6;
			band_c_.b[1] = -5.46984852; band_c_.b[2] = 12.4879330;
			band_c_.b[3] = -15.2305828; band_c_.b[4] = 10.4650611;
			band_c_.b[5] = -3.84070239; band_c_.b[6] = 0.588144766;
			band_c_.ah = 0.9999;        band_c_.bh = -0.9998;
    		sens_c_.a[0] = 0.00068655489;   sens_c_.a[1] = 6.1191384e-6;
	    	sens_c_.a[2] = -0.0013669906;   sens_c_.a[3] = -6.1191384e-6;
		    sens_c_.a[4] = 0.00068043575;
    		sens_c_.b[1] = -3.880097;       sens_c_.b[2] = 5.6448073;
	    	sens_c_.b[3] = -3.6491997;      sens_c_.b[4] = 0.8844899;
		    rclo_c_.a = 0.0010406;  rclo_c_.b = -0.99792;  rclo_c_.K = 124;
		    avg_num_ = 40;
		    smpl_rate_ = 1600;
            break;
        case PstSR2560Hz:
			band_c_.a[0] = band_c_.a[6] = 4.72720485e-9;
			band_c_.a[1] = band_c_.a[5] = 2.83632291e-8;
			band_c_.a[2] = band_c_.a[4] = 7.09080728e-8;
			band_c_.a[3] = 9.45440971e-8;
			band_c_.b[1] = -5.67492833; band_c_.b[2] = 13.4270495;
			band_c_.b[3] = -16.9536067; band_c_.b[4] = 12.0481609;
			band_c_.b[5] = -4.56904318; band_c_.b[6] = 0.72236816;
			band_c_.ah = 0.99994;       band_c_.bh = -0.99988;
    		sens_c_.a[0] = 0.000273850727;  sens_c_.a[1] = 1.52804205e-6;
	    	sens_c_.a[2] = -0.000546173412; sens_c_.a[3] = -1.52804205e-6;
		    sens_c_.a[4] = 0.000272322685;
    		sens_c_.b[1] = -3.9244238;      sens_c_.b[2] = 5.7750481;
	    	sens_c_.b[3] = -3.7767939;      sens_c_.b[4] = 0.9261697;
		    rclo_c_.a = 0.00065062; rclo_c_.b = -0.9987;   rclo_c_.K = 124;
		    avg_num_ = 80;
		    smpl_rate_ = 2560;
            break;
        default:    // 400Hz
			band_c_.a[0] = band_c_.a[6] = 0.000151;
			band_c_.a[1] = band_c_.a[5] = 0.000904;
			band_c_.a[2] = band_c_.a[4] = 0.00226;
			band_c_.a[3] = 0.00301;
			band_c_.b[1] = -3.9315;     band_c_.b[2] = 6.6912;
			band_c_.b[3] = -6.2495;     band_c_.b[4] = 3.3605;
			band_c_.b[5] = -0.9828;     band_c_.b[6] = 0.1218;
			band_c_.ah = 0.99961;       band_c_.bh = -0.99921;
    		sens_c_.a[0] = 0.00931245;  sens_c_.a[1] = 0.00032762;
	    	sens_c_.a[2] = -0.01829728; sens_c_.a[3] = -0.00032762;
		    sens_c_.a[4] = 0.00898483;
    		sens_c_.b[1] = -3.548754;   sens_c_.b[2] = 4.714548;
	    	sens_c_.b[3] = -2.776010;   sens_c_.b[4] = 0.610325;
		    rclo_c_.a = 0.00415;    rclo_c_.b = -0.99170;  rclo_c_.K = 2;
		    avg_num_ = 20;
		    smpl_rate_ = 1600;
            break;
    }
    
    memset(band_v_, 0, sizeof(band_v_));
    memset(sens_v_, 0, sizeof(sens_v_));
    memset(rclo_v_, 0, sizeof(rclo_v_));
    memset(&dcf_v_[0][0], 0, sizeof(DCFltrTmpVar));
    fi = val*val/2;
    for(i = 0; i < 180; i++) {
        dcf_v_[0][0].afts[i] = fi;
    }
    dcf_v_[0][0].aftavg = fi;
    memcpy(&dcf_v_[0][1], &dcf_v_[0][0], sizeof(DCFltrTmpVar));
    memcpy(&dcf_v_[0][2], &dcf_v_[0][0], sizeof(DCFltrTmpVar));
    for(i = 1; i < kChannelTol; i++) {
        for(j = 0; j < 3; j++) {
            memcpy(&dcf_v_[i][j], &dcf_v_[0][0], sizeof(DCFltrTmpVar));
        }
    }
}
