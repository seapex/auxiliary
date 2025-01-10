#include <math.h>
//#include <fastmath67x.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "flicker_filter.h"

#define kChannelsTol 4   //Total number of channels

/*!
0.05~35HZ Band pass filter parameter
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
typedef struct {
    BandConst con;
    BandTmpVar var[kChannelsTol][3];
} BandPasFltrPara;
#pragma DATA_SECTION(band_fltr_, ".data");
static BandPasFltrPara band_fltr_;

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
typedef struct {
    RCLowConst con;
    RCLowTmpVar var[kChannelsTol][3];
} RCLpasFltrPara;
#pragma DATA_SECTION(rc_fltr1_, ".data");
static RCLpasFltrPara rc_fltr1_;   //27.3s for block1
#pragma DATA_SECTION(rc_fltr4_, ".data");
static RCLpasFltrPara rc_fltr4_;   //0.3s for block4

/*!
Weighting filter parameter
*/
typedef struct {
    double a[5];    //coefficient a
    double b[5];    //coefficient b
} WeightConst;      //Visual sensitivity filter constant coefficient
typedef struct {
    double x[5];    //variable x
    double y[5];    //variable y
} WeightTmpVar;     //Visual sensitivity filter temp variable
typedef struct {
    WeightConst con;
    WeightTmpVar var[kChannelsTol][3];
} WghtFltrPara;
#pragma DATA_SECTION(wght_fltr_, ".data");
static WghtFltrPara wght_fltr_;

typedef struct {
    int32_t buf[512];  //
    uint64_t sum;
    int16_t pnt[2]; //pointer to FIFO32.buf, [0-1]:head,tail、
    int16_t cnt;    //rms calculation interval count.
    int16_t zo_f_;  //Scaling factor to prevent numerical overflow
} FIFO32;
FIFO32 fifo_rms_[kChannelsTol][3];   //FIFO for 5cycle rms calculation.
FIFO32 *prms_;

#pragma DATA_SECTION(avg_num_, ".data");
static uint8_t avg_num_;    //The number of instantaneous flicker values used to calculate an average
static uint8_t block2_ = 0;     //Algorithm of block 2. 0=x^2, 1=x.
static int16_t smpl_rate_;   //sampling rate. unit:Hz
static int16_t rms_invr_;   //rms calculation interval.

uint8_t avg_num_flicker() { return avg_num_; }
void set_block2(uint8_t val) { block2_ = val&1; }
int16_t prms_zo_f(int cdx, int phs) { return fifo_rms_[cdx][phs].zo_f_; }

/*!
Initialize flicker filter calculation parameters

    Input:  rate -- sampling rate type. refer to kPstSampleRate.
*/
void IniFilterPar(int rate)
{
    int i, j;
    float fi;

    switch (rate) {
        case PstSR800Hz:
			band_fltr_.con.a[0] = band_fltr_.con.a[6] = 0.000003968763;
			band_fltr_.con.a[1] = band_fltr_.con.a[5] = 0.00002381258;
			band_fltr_.con.a[2] = band_fltr_.con.a[4] = 0.00005953145;
			band_fltr_.con.a[3] = 0.00007937526;
			band_fltr_.con.b[1] = -4.945139;   band_fltr_.con.b[2] = 10.26772;
			band_fltr_.con.b[3] = -11.44799;   band_fltr_.con.b[4] = 7.223983;
			band_fltr_.con.b[5] = -2.444865;   band_fltr_.con.b[6] = 0.3465408;
			band_fltr_.con.ah = 0.9998;        band_fltr_.con.bh = -0.99961;
    		wght_fltr_.con.a[0] = 0.00259963;  wght_fltr_.con.a[1] = 0.0000461345;
	    	wght_fltr_.con.a[2] = -0.00515313;  wght_fltr_.con.a[3] = -0.0000461345;
		    wght_fltr_.con.a[4] = 0.0025535;
    		wght_fltr_.con.b[1] = -3.765314;   wght_fltr_.con.b[2] = 5.313669;
	    	wght_fltr_.con.b[3] = -3.330456;   wght_fltr_.con.b[4] = 0.7821078;
		    rc_fltr4_.con.a = 0.002079;   rc_fltr4_.con.b = -0.99584;  rc_fltr4_.con.K = 121;
		    avg_num_ = 20;
		    smpl_rate_ = 800;
            break;
        case PstSR1600Hz:
			band_fltr_.con.a[0] = band_fltr_.con.a[6] = 8.07852914e-8;
			band_fltr_.con.a[1] = band_fltr_.con.a[5] = 4.84711748e-7;
			band_fltr_.con.a[2] = band_fltr_.con.a[4] = 1.21177937e-6;
			band_fltr_.con.a[3] = 1.61570583e-6;
			band_fltr_.con.b[1] = -5.46984852; band_fltr_.con.b[2] = 12.4879330;
			band_fltr_.con.b[3] = -15.2305828; band_fltr_.con.b[4] = 10.4650611;
			band_fltr_.con.b[5] = -3.84070239; band_fltr_.con.b[6] = 0.588144766;
			band_fltr_.con.ah = 0.9999;        band_fltr_.con.bh = -0.9998;
    		wght_fltr_.con.a[0] = 0.00068655489;   wght_fltr_.con.a[1] = 6.1191384e-6;
	    	wght_fltr_.con.a[2] = -0.0013669906;   wght_fltr_.con.a[3] = -6.1191384e-6;
		    wght_fltr_.con.a[4] = 0.00068043575;
    		wght_fltr_.con.b[1] = -3.880097;       wght_fltr_.con.b[2] = 5.6448073;
	    	wght_fltr_.con.b[3] = -3.6491997;      wght_fltr_.con.b[4] = 0.8844899;
		    rc_fltr4_.con.a = 0.0010406;  rc_fltr4_.con.b = -0.99792;  rc_fltr4_.con.K = 120;
		    avg_num_ = 40;
		    smpl_rate_ = 1600;
            break;
        case PstSR2560Hz:
			band_fltr_.con.a[0] = band_fltr_.con.a[6] = 5.318723435e-9;
			band_fltr_.con.a[1] = band_fltr_.con.a[5] = 3.191234061e-8;
			band_fltr_.con.a[2] = band_fltr_.con.a[4] = 7.978085152e-8;
			band_fltr_.con.a[3] = 1.063744687e-7;
			band_fltr_.con.b[1] = -5.668303216; band_fltr_.con.b[2] = 13.39607301;
			band_fltr_.con.b[3] = -16.89562353; band_fltr_.con.b[4] = 11.99384858;
			band_fltr_.con.b[5] = -4.543586172; band_fltr_.con.b[6] = 0.7175916763;
			band_fltr_.con.ah = 0.99994;       band_fltr_.con.bh = -0.99988;
    		wght_fltr_.con.a[0] = 0.0002738507269;  wght_fltr_.con.a[1] = 1.528042052e-6;
	    	wght_fltr_.con.a[2] = -0.0005461734118; wght_fltr_.con.a[3] = -1.528042052e-6;
		    wght_fltr_.con.a[4] = 0.0002723226849;
    		wght_fltr_.con.b[1] = -3.924423823;      wght_fltr_.con.b[2] = 5.775048124;
	    	wght_fltr_.con.b[3] = -3.776793934;      wght_fltr_.con.b[4] = 0.9261697117;
		    rc_fltr4_.con.a = 0.00065062; rc_fltr4_.con.b = -0.9987;   rc_fltr4_.con.K = 124;
		    avg_num_ = 80;
		    smpl_rate_ = 2560;
            break;
        default:    // 400Hz
			band_fltr_.con.a[0] = band_fltr_.con.a[6] = 0.000151;
			band_fltr_.con.a[1] = band_fltr_.con.a[5] = 0.000904;
			band_fltr_.con.a[2] = band_fltr_.con.a[4] = 0.00226;
			band_fltr_.con.a[3] = 0.00301;
			band_fltr_.con.b[1] = -3.9315;     band_fltr_.con.b[2] = 6.6912;
			band_fltr_.con.b[3] = -6.2495;     band_fltr_.con.b[4] = 3.3605;
			band_fltr_.con.b[5] = -0.9828;     band_fltr_.con.b[6] = 0.1218;
			band_fltr_.con.ah = 0.99961;       band_fltr_.con.bh = -0.99921;
    		wght_fltr_.con.a[0] = 0.00931245;  wght_fltr_.con.a[1] = 0.00032762;
	    	wght_fltr_.con.a[2] = -0.01829728; wght_fltr_.con.a[3] = -0.00032762;
		    wght_fltr_.con.a[4] = 0.00898483;
    		wght_fltr_.con.b[1] = -3.548754;   wght_fltr_.con.b[2] = 4.714548;
	    	wght_fltr_.con.b[3] = -2.776010;   wght_fltr_.con.b[4] = 0.610325;
		    rc_fltr4_.con.a = 0.00415;    rc_fltr4_.con.b = -0.99170;  rc_fltr4_.con.K = 124;
		    avg_num_ = 20;
		    smpl_rate_ = 400;
            break;
    }
    rms_invr_ = smpl_rate_/100; //The rms is calculated every half period
    //rc_fltr1_.con.a = 1.8312E-04;  rc_fltr1_.con.b = -0.9996338;  rc_fltr1_.con.K = 1;  //27.3s at 100Hz
    //rc_fltr1_.con.a = 8.332638947E-05;  rc_fltr1_.con.b = -0.999833347;  rc_fltr1_.con.K = 1;  //60s at 100Hz
    //rc_fltr1_.con.a = 5.555246931E-05;  rc_fltr1_.con.b = -0.999888895;  rc_fltr1_.con.K = 1;  //90s at 100Hz
    rc_fltr1_.con.a = 4.166493063E-05;  rc_fltr1_.con.b = -0.99991667;  rc_fltr1_.con.K = 1;  //120s at 100Hz

    memset(rc_fltr1_.var, 0, sizeof(rc_fltr1_.var));
    memset(band_fltr_.var, 0, sizeof(band_fltr_.var));
    memset(wght_fltr_.var, 0, sizeof(wght_fltr_.var));
    memset(rc_fltr4_.var, 0, sizeof(rc_fltr4_.var));

    memset(fifo_rms_, 0, sizeof(fifo_rms_));
}

/*!
Preprocessing before filter

    Input:  src --
            cnt -- Number of SV in src
            tvr -- temp variable for RC low filter
            smp_n -- sampling number per n cycle
    Output: des --
*/
void PreFilter(float *des, const int32_t *src, int cnt, RCLowTmpVar *tvr, int smp_n)
{
    float rms= tvr->y[1];
    int i, v, sz, pos, nz=0, cnt4zof=0;
    if (rms>0) nz=block2_+1;    //not zero. 0=zero, 1=x^2, 2=x.
    
    int16_t head, tail;
    head = prms_->pnt[0];
    tail = prms_->pnt[1];
    for (i=0; i<cnt; i++) {
        v = src[i]>>prms_->zo_f_;
        if (abs(v)>28000) {
            cnt4zof++;
        }
        v *= v;
        prms_->sum += v;
        prms_->buf[head++] = v;
        head &= 0x1ff;
        if (++(prms_->cnt) >= rms_invr_) {
            prms_->cnt = 0;
            sz = head;
            if (head<tail) sz += 512;
            sz -= tail;
            while (sz>smp_n) {
                prms_->sum -= prms_->buf[tail++];
                tail &= 0x1ff;
                sz--;
            }
            rms = sqrt(prms_->sum/sz);
            //printf("prms_->sum=%lld,sz=%d;  ", prms_->sum, sz);
            tvr->x[0] = tvr->x[1];
            tvr->x[1] = rms;
            tvr->y[0] = tvr->y[1];
            tvr->y[1] = rc_fltr1_.con.K*rc_fltr1_.con.a*(tvr->x[1]+tvr->x[0]) - rc_fltr1_.con.b*tvr->y[0];
            rms = tvr->y[1];
            if (rms>0) nz=block2_+1;
            else nz=0;
        }
        des[i] = src[i];
        des[i] = des[i]/rms/(1<<prms_->zo_f_);
        //printf("%f, ", rms);
        switch (nz) {
            case 1:     //x^2. for AC & DC
                des[i] *= des[i];
                des[i] *= 50;   //(1/sqrt(2))^2=1/2=50%
                break;
            case 2:     //x. only for DC
                des[i] *= 100;   //100%
                break;
            default:
                des[i] = 0; //此句必须放此处，否则会导致开始的几个des未被赋值.
                break;
        }
    }
    if (cnt4zof>15) prms_->zo_f_++;
    if (prms_->zo_f_) {
        if (rms<5000) prms_->zo_f_--;
    }
    prms_->pnt[0] = head;
    prms_->pnt[1] = tail;
}

/*!
0.05Hz~35Hz bandpass filter

    Input:  src --
            cnt -- Number of SV of src
            tvr -- temp variable for bandpass filter
    Output: des --
*/
void BandPasFilter(float *des, const float *src, int cnt, BandTmpVar *tvr)
{
    int i;

#pragma MUST_ITERATE(8);
    for(i = 0; i < cnt; i++) {
        //一阶高通滤波(0.05Hz)
        tvr->xh[0] = tvr->xh[1];
        tvr->xh[1] = src[i];
        tvr->yh[0] = tvr->yh[1];
        tvr->yh[1] = band_fltr_.con.ah*(tvr->xh[1]-tvr->xh[0]) - band_fltr_.con.bh*tvr->yh[0];

        // 六阶巴特沃斯低通滤波(35Hz)
        tvr->x[0] = tvr->x[1];
        tvr->x[1] = tvr->x[2];
        tvr->x[2] = tvr->x[3];
        tvr->x[3] = tvr->x[4];
        tvr->x[4] = tvr->x[5];
        tvr->x[5] = tvr->x[6];
        tvr->x[6] = tvr->yh[1]; 
        tvr->y[0] = tvr->y[1];
        tvr->y[1] = tvr->y[2];
        tvr->y[2] = tvr->y[3];
        tvr->y[3] = tvr->y[4];
        tvr->y[4] = tvr->y[5];
        tvr->y[5] = tvr->y[6];
        tvr->y[6] = band_fltr_.con.a[0] * tvr->x[6]
                    + band_fltr_.con.a[1] * tvr->x[5]
                    + band_fltr_.con.a[2] * tvr->x[4]
                    + band_fltr_.con.a[3] * tvr->x[3]
                    + band_fltr_.con.a[4] * tvr->x[2]
                    + band_fltr_.con.a[5] * tvr->x[1]
                    + band_fltr_.con.a[6] * tvr->x[0]
                    - band_fltr_.con.b[1] * tvr->y[5]
                    - band_fltr_.con.b[2] * tvr->y[4]
                    - band_fltr_.con.b[3] * tvr->y[3]
                    - band_fltr_.con.b[4] * tvr->y[2]
                    - band_fltr_.con.b[5] * tvr->y[1]
                    - band_fltr_.con.b[6] * tvr->y[0];
        des[i] = tvr->y[6];
    }
}

/*!
Weighting filter

    Input:  src --
            cnt -- Number of SV of src
            tvr -- temp variable for sens filter
    Output: des --
*/
void WeightFilter(float *des, const float *src, int cnt, WeightTmpVar *tvr)
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
        tvr->y[4] = wght_fltr_.con.a[0] * tvr->x[4]
                    + wght_fltr_.con.a[1] * tvr->x[3]
                    - wght_fltr_.con.b[1] * tvr->y[3]
                    + wght_fltr_.con.a[2] * tvr->x[2]
                    - wght_fltr_.con.b[2] * tvr->y[2]
                    + wght_fltr_.con.a[3] * tvr->x[1]
                    - wght_fltr_.con.b[3] * tvr->y[1]
                    + wght_fltr_.con.a[4] * tvr->x[0]
                    - wght_fltr_.con.b[4] * tvr->y[0];

        des[i] = tvr->y[4];
    }
}

/*!
RC lowpass filter

    Input:  src --
            cnt -- Number of SV of src
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
        tvr->y[1] = rc_fltr4_.con.K*rc_fltr4_.con.a*(tvr->x[1]+tvr->x[0]) - rc_fltr4_.con.b*tvr->y[0];

        des[i] = tvr->y[1];
    }
}

/*!
Flicker filter

    Input:  src -- sampling value
            cnt -- Number of SV of src
            cdx -- channels index. 0-
            phs -- phase, 0~2=A~C
            frq -- Power frequency, unit:Hz
    Output: des -- Average instantaneous flicker value
    Return: number of output data.
*/
int FlickerFilter(float *des, const int32_t *src, int cnt, int cdx, int phs, float frq)
{
    if (frq==0) return 0;
    float fi;
    int i, j, k;
    float *pd = des;
    const int32_t *ps = src;
    i = cnt;
    int freq = smpl_rate_*2/frq+0.5;
    while(i>0) {
        j = i<smpl_rate_?i:smpl_rate_;
        prms_ = &fifo_rms_[cdx][phs];
        PreFilter(pd, ps, j, &rc_fltr1_.var[cdx][phs], smpl_rate_*2/frq+0.5);    //smpl_rate_*n/frq+0.5 -- Calculate the rms based on n cycles
        BandPasFilter(pd, pd, j, &band_fltr_.var[cdx][phs]);
        WeightFilter(pd, pd, j, &wght_fltr_.var[cdx][phs]);
        RCLowPsFilter(pd, pd, j, &rc_fltr4_.var[cdx][phs]);
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

