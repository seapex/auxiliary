/*! \file generic.cpp
    \brief Generic functions.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _GENERIC_H_
#define _GENERIC_H_
//---------------------------------------------------------------------------
#include <stdint.h>

#define setbit(x,y) (x|=(1<<y)) //Set the Yth bit of X
#define clrbit(x,y) (x&=~(1<<y)) //Clear the Yth bit of X
#define spybit(x,y) (x&(1<<y)) //Check the Yth bit of X

const char *BaseName(const char *path);
unsigned int AdjSaveSpc(unsigned int space, int type);
int AssertPath(const char * path);
int CompareInt(const void *arg1, const void *arg2);
int CompareFloat(const void *arg1, const void *arg2);
uint32_t DiskFree(char *devname);
int FloatFmt(float fdata, int totl, int decim);
char *GenGuid(uint16_t *guid);
bool IsInt(const char *str);
uint8_t NESpace2Idx(uint8_t space);

#endif //_GENERIC_H_
