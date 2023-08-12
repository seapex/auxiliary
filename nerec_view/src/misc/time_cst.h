/*! \file time_cst.h
    \brief customized defined time function.
*/

#ifndef TIME_CST_H_
#define TIME_CST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

void TimeCstInit();
void TimeCstEnd();

void LocalTime(struct tm *des, const time_t *src);
time_t MakeTime(struct tm *src, int type);
void msSleep(int ms);
const char *NowTime(int type);
void SetTime(void *ptm, int tt, int type);
void SetTimeZone(int tz);
void StopWatch (int id, bool se, const char * desc=NULL);
void time_t2tm(struct tm *des, const time_t *src, int tzn_t);
int time_zone();
int TimevalCmp(const timeval *tmv1, const timeval *tmv2);
timeval TimevalSum(const timeval *tmv1, const timeval *tmv2, int sign);
const char *Time2Str(time_t tim);

float stopwatch_dur(int id);


#ifdef __cplusplus
}
#endif

#endif //TIME_CST_H_

