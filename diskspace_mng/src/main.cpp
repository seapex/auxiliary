//#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <stdarg.h>
//#include<time.h>
//#include<fcntl.h>
//#include <math.h>
#include <unistd.h>

//#include "time_cst.h"
//#include "script_cfg.h"
//#include "loopbuf_sort.h"
//#include "data_statis.h"

bool doIt = true;

/*!
This function handles the ^c, and allows us to cleanup on exit
*/
void ctrlCfun (int i)
{
    printf("ctrlCfun is invoked!\n");
    doIt = false;
}

/*!
Get the free disk space of the specified device

    Input:  devname -- the device name, e.g. "/dev/mmcblk0p1"
    Return: the free disk space in MB 
*/
uint32_t DiskFree(char *devname)
{
    char buf[128];
    sprintf(buf, "df | grep %s", devname);
    FILE *fstrm = popen(buf, "r");
    if (fstrm == NULL) {
        printf("popen error!\n");
        return 0;
    }
    if (fgets(buf, sizeof(buf), fstrm)==NULL) {
        printf("Unable to obtain relevant information!\n");
        return 0;
    }
    pclose(fstrm);
    uint32_t ui;
    sscanf(buf, "%*s %*d %*d %d", &ui);
    return ui/1000;
}

int main (int argc, char *argv[])
{
    printf("diskfree: %d MB\n", DiskFree("/dev/mmcblk0p1"));
}


