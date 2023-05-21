#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifndef OSbeWindows
    #include <poll.h>
#endif

#include "time_cst.h"

static pthread_mutex_t mutex_time_;
void TimeCstInit()
{
    pthread_mutex_init (&mutex_time_,NULL);
}
void TimeCstEnd()
{
    pthread_mutex_destroy(&mutex_time_); //清除互斥锁mutex
}

static struct timeval time_st_[10], time_end_[10];

#ifndef OSbeWindows
/*!
Sleep x ms
*/
void msSleep(int x)
{
    poll(NULL, 0, x);
}

/*!
    Input:  time -- 
            tt -- time type. 0=tm, 1=time_t
            type -- 0=raw, 1=consider time zone
*/
void SetTime(void *time, int tt, int type)
{
    struct timeval tmvi;
    tmvi.tv_usec = 0;
    if (tt) {
        tmvi.tv_sec = *((time_t*)time);
        if (type) tmvi.tv_sec -= time_zone()*3600;
    } else {
        tmvi.tv_sec = MakeTime((tm *)time, type);
    }
    settimeofday(&tmvi, NULL);
    system("hwclock -w");
    //rtc_dev().Set(tmvi.tv_sec);
}

/*!
    Input:  id
            se -- start or end. 1=start, 0=end.
            desc -- description
*/
void StopWatch (int id, bool se, const char *desc)
{
    static const char *pdsc[10];
    if (id>=10) return;

    if (se) {
        gettimeofday(&time_st_[id], NULL);
        pdsc[id] = desc;
    } else {
        gettimeofday(&time_end_[id], NULL);
        time_end_[id].tv_sec -= time_st_[id].tv_sec;
        if (time_end_[id].tv_usec < time_st_[id].tv_usec) {
            time_end_[id].tv_usec += 1000000;
            time_end_[id].tv_sec -= 1;
        }
        time_end_[id].tv_usec -= time_st_[id].tv_usec;
        if (pdsc[id]) {
            printf("%s spend %7.6fs\n", pdsc[id], stopwatch_dur(id));
        }
    }
}
#endif

/*!
Convert tm to time_t

    Input:  src -- source
            type -- 0=raw, 1=consider time zone
*/
time_t MakeTime(struct tm *src, int type)
{
    if (!src) return 0;

	src->tm_isdst = 0;
	time_t tm_t = mktime(src);
    if (type) tm_t -= time_zone()*3600;
	return tm_t;
}

/*!
    Input:  type -- output format
*/
const char *NowTime (int type)
{
    static char strtim[32];
    time_t time1 = time(NULL);
    tm tmi;
    time_t2tm(&tmi, &time1, 1);
    switch (type) {
        case 1:
            strftime(strtim, 32, "%y%m%d_%H%M%S", &tmi);
            break;
        case 2:
            strftime(strtim, 32, "%y%m%d", &tmi);
            break;
        case 3:
            strftime(strtim, 32, "%H%M%S", &tmi);
            break;
        default:
            strftime(strtim, 32, "%Y-%m-%d %H:%M:%S", &tmi);
            break;
    }
    return strtim;
}

static int time_zone_ = 0;
int time_zone() { return time_zone_; }
/*!
Set time zone
    
    Input:  tz -- time zone. [-12,12]
*/
void SetTimeZone(int tz)
{
    if (tz<=12&&tz>=-12) {
        time_zone_ = tz;
    } else {
        time_t timei = 86400;
        tm *ptmi;
        
        pthread_mutex_lock(&mutex_time_);
        ptmi = localtime(&timei);
        tz = ptmi->tm_hour;
        ptmi = gmtime(&timei);
        tz -= ptmi->tm_hour;
        pthread_mutex_unlock(&mutex_time_);
        if (tz>12) {
            tz -= 24;
        } else if (tz<-12) {
            tz += 24;
        }
    }
}

float stopwatch_dur(int id)
{ 
    float fi = time_end_[id].tv_usec;
    fi /= 1000000;
    fi += time_end_[id].tv_sec;
    return fi;
}

/*!
Convert time_t to tm, it's thread-safe
    Input:  src -- source
            tzn_t -- timezone type. 0=utc, 1=local
    Output: des -- destination
*/
void time_t2tm(struct tm *des, const time_t *src, int tzn_t)
{
    if (!src||!des) return;
    time_t tmt = *src;
    if (tzn_t) {
        tmt += time_zone()*3600;
    }
    pthread_mutex_lock(&mutex_time_);
    memcpy(des, gmtime(&tmt), sizeof(tm));
    pthread_mutex_unlock(&mutex_time_);
}

/*!
    Input:  tim -- 
*/
const char *Time2Str(time_t tim)
{
    static char strtim[32];
    tm tmi;
    time_t2tm(&tmi, &tim, 1);
    strftime(strtim, 32, "%Y-%m-%d %H:%M:%S", &tmi);
    return strtim;
}

/*!
Compares two timeval

    Input:  tmv1 --
            tmv2 --
    Return: -1=tmv1<tmv2, 0=tmv1==tmv2, 1=tmv1>tmv2
*/
int TimevalCmp(const timeval *tmv1, const timeval *tmv2)
{
    if (tmv1->tv_sec < tmv2->tv_sec) {
        return -1;    
    } else if (tmv1->tv_sec > tmv2->tv_sec) {
        return 1;
    } else {
        if (tmv1->tv_usec < tmv2->tv_usec) {
            return -1;
        } else if (tmv1->tv_usec > tmv2->tv_usec) {
            return 1;
        } else {
            return 0;
        }
    }
}

/*!
Sum two timeval

    Input:  tmv1 -- 
            tmv2 --
            sign -- sign of tmv2. 0=plus, 1=minus
    Return: tmv1 + (sign)tmv2
*/
timeval TimevalSum(const timeval *tmv1, const timeval *tmv2, int sign)
{
    timeval tmv;
    if (sign) { //minus
        tmv.tv_sec = tmv1->tv_sec - tmv2->tv_sec;
        tmv.tv_usec = tmv1->tv_usec - tmv2->tv_usec;
        while (tmv.tv_usec < 0) {
            tmv.tv_usec += 1000000;
            tmv.tv_sec--;
        }
    } else {    //plus
        tmv.tv_sec = tmv1->tv_sec + tmv2->tv_sec;
        tmv.tv_usec = tmv1->tv_usec + tmv2->tv_usec;
        while (tmv.tv_usec >= 1000000) {
            tmv.tv_usec -= 1000000;
            tmv.tv_sec++;
        }
    }
    return tmv;
}

