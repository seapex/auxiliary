/*! \file misc.h
    \brief miscellaneous function.
*/

#ifndef _MISC_H_
#define _MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

void msSleep(int ms);
void StopWatch (int id, bool se, const char * desc=NULL);
float stopwatch_dur(int id);

#ifdef __cplusplus
}
#endif

#endif //_MISC_H_

