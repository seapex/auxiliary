#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/time.h>

#include "misc.h"

/*!
Sleep x ms
*/
void msSleep(int x)
{
    poll(NULL, 0, x);
}

static struct timeval time_st_[10], time_end_[10];

float stopwatch_dur(int id)
{ 
    float fi = time_end_[id].tv_usec;
    fi /= 1000000;
    fi += time_end_[id].tv_sec;
    return fi;
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

