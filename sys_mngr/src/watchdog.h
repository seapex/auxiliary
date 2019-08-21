#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <linux/watchdog.h>   // /usr/local/arm/cross/am335xt3/devkit/arm-arago-linux-gnueabi/usr/include
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

class WatchDog
{
public:
    WatchDog(){
        fd_ = 0;
    };
    ~WatchDog(){};

    /*! \brief Feed watchdog
     Sends an IOCTL to the driver, which in turn ticks the PC Watchdog card 
     to reset its internal timer so it doesn't trigger a computer reset.
    */
    void Feed() {
        fd_ = Open();
        if (!fd_) return;
        int dummy;
        ioctl(fd_, WDIOC_KEEPALIVE, &dummy);
        Close();
    };
    void Enable(int secs=60) {
        fd_ = Open();
        if (!fd_) return;
#if 1
#else   //AM335x driver not support below
        int rslt = OnOff(WDIOS_ENABLECARD);
	    if (rslt<0) printf("Enable watchdog failure!\n");
	    else printf("Watchdog device enabled.\n");
#endif
        SetTimeout(secs);
        Close();
    };
    void Disable() {
        fd_ = Open();
        if (!fd_) return;
#if 1
        SetTimeout(86400);  //86400(1day) is max value can be set.
#else   //AM335x driver not support below
        int rslt = OnOff(WDIOS_DISABLECARD);
	    if (rslt<0) printf("Disable watchdog failure!\n");
	    else printf("Watchdog device enabled\n");
#endif
        Close();
	};
    
protected:
private:
	int fd_;
	int Open() {
       int fd = open("/dev/watchdog", O_WRONLY);

        if (fd == -1) {
        	fprintf(stderr, "Watchdog device not opened\n");
        	fflush(stderr);
        	fd = 0;
        }
        return fd;
	};
	void Close() {
        if (fd_) {
            close(fd_);
            fd_ = 0;
            //printf("Watchdog device stopped\n");
        }
	};
	int OnOff(int flag) {
        fd_ = Open();
        if (!fd_) return -1;
	    return ioctl(fd_, WDIOC_SETOPTIONS, &flag);
	};
    void SetTimeout(int secs) {
        if(!fd_) return; 
        int rslt = ioctl(fd_, WDIOC_SETTIMEOUT, &secs); 
        if (rslt<0) printf("SetTimeout error %d\n", rslt);
        else printf("Set watchdog timeout to %ds\n", secs);
    };
};

WatchDog &watchdog()
{
	static WatchDog wtdog;
	return wtdog;
};



#endif //_WATCHDOG_H_
