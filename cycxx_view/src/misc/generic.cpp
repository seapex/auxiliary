#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "generic.h"

const static int ValidSaveSpaces3s[ ] = {3, 6, 12, 15, 30, 45, 60, 90,
                                         120, 180, 240, 300, 360, 600, 720, 900
                                        };
const static int ValidSaveSpacesFrq[ ] = {10, 60, 180, 600};
const static int ValidSaveSpacesNE[ ] = {3, 15, 30, 60, 113}; //113=0.2s
/*!
Adjust save space

    Input:  space -- space to be adjusted
            type -- 1=freq, 2=new energy, 3=3s
    Return: adjusted space
*/
unsigned int AdjSaveSpc ( unsigned int space, int type )
{
    int k;
    const int *pk;
    switch (type) {
        case 1:     //freq
            k = sizeof ( ValidSaveSpacesFrq ) / sizeof ( int );
            pk = ValidSaveSpacesFrq;
            break;
        case 2:     //new energy
            k = sizeof ( ValidSaveSpacesNE ) / sizeof ( int );
            return ValidSaveSpacesNE[space>=k?0:space];
        default:    //3s
            k = sizeof ( ValidSaveSpaces3s ) / sizeof ( int );
            pk = ValidSaveSpaces3s;
            break;
    }
    for ( int i = 0; i < k; i++ ) {
        if ( space <= pk[i] ) return pk[i];
    }
    return pk[k - 1];
}

/*!
if the path not exist, create it

    Return: 0=success, 1=failure
*/
int AssertPath ( const char *path )
{
    FILE *fp = fopen ( path, "rb" );
    if ( fp == NULL ) {
        char stri[128];
        sprintf ( stri, "mkdir -p %s", path );
        system ( stri );
        fp = fopen ( path, "rb" );
        if ( fp == NULL ) {
            printf ( "create path %s failure!\n", path );
            return 1;
        }
    }
    fclose ( fp );
    return 0;
}

/*!
return the last component of a pathname

    Input:  path -- pathname
*/
const char *BaseName(const char *path)
{
    const char *pch = strrchr(path, '/');
    if(pch) pch++;
    return pch;
}

/*!
Compare two integer

    Input:  arg1,arg2 -- data be compared
    Return: -1=(arg1<arg2), 0=(arg1==arg2), 1=(arg1>arg2)
*/
int CompareInt ( const void *arg1, const void *arg2 )
{
    int a = * ( int *) arg1;
    int b = * ( int *) arg2;
    if ( a > b ) return 1;
    else if ( a < b ) return -1;
    else return 0;
}

/*!
Compare two float

    Input:  arg1,arg2 -- data be compared
    Return: -1=(arg1<arg2), 0=(arg1==arg2), 1=(arg1>arg2)
*/
int CompareFloat ( const void *arg1, const void *arg2 )
{
    float a = * ( float *) arg1;
    float b = * ( float *) arg2;
    if ( a > b ) return 1;
    else if ( a < b ) return -1;
    else return 0;
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

/*!
Calculate the number of decimal places that fdata should retain.
example1: fdata=123456.15675... totl=5 decim=3 result=0
example2: fdata=12345.15675... totl=5 decim=3 result=0
example3: fdata=1234.15675... totl=5 decim=3 result=0
example4: fdata=123.15675... totl=5 decim=3 result=1
example5: fdata=12.15675... totl=5 decim=3 result=2
example6: fdata=1.15675... totl=5 decim=3 result=3
Note: Positive and negative are the same

    Input:  fdata --
            totl -- total number of significant digits
            decim -- Maximum number of decimal places
    Return: number of decimal
*/
int FloatFmt ( float fdata, int totl, int decim )
{
    if (totl<3) return 0;
    int i;
    fdata = fabs(fdata);
    for (i=0; i<totl; i++) {
        if (fdata<10) break;
        fdata /= 10;
    }
    i = totl - i - 2;
    if (i<0) i = 0;
    return i > decim ? decim : i;
}

/*!
Generate GUID

    Input:  guid -- uint16_t[8]
    Return: guid in string type
*/
char *GenGuid ( uint16_t *guid )
{
    int i;
    static char guid_str[33];
    for ( i = 0; i < 8; i++ ) {
        guid[i] = rand() % 0xffff;
        sprintf ( &guid_str[i * 4], "%04x", guid[i] );
    }
    return guid_str;
}

bool IsInt(const char *str)
{
    for (int i=0; i<strlen(str); i++) {
        if(!isdigit(str[i])) return false;
    }
    return true;
}

/*!
Adjust save space to valid value

    Input:  space -- interval time. unit:s
            type -- 0=3s, 1=freq, 2=new energy
*/
uint8_t NESpace2Idx(uint8_t space)
{
    int k = sizeof(ValidSaveSpacesNE) / sizeof (int);
    for (int i=0; i<k; i++) {
        if (space==ValidSaveSpacesNE[i]) return i;
    }
    return 0;
}



