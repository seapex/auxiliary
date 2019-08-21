#ifndef _MD5SUM_H_
#define _MD5SUM_H_

#undef BIG_ENDIAN_HOST
typedef unsigned int u32;

typedef struct {
    u32 A, B, C, D;   /* chaining variables */
    u32  nblocks;
    unsigned char buf[64];
    int  count;
} MD5_CONTEXT;


/****************
 * Rotate a 32 bit integer by n bytes
 */
#if defined(__GNUC__) && defined(__i386__)
static inline u32
rol( u32 x, int n)
{
    __asm__("roll %%cl,%0"
            :"=r" (x)
            :"0" (x), "c" (n));
    return x;
}
#else
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
#endif

int md5_check(char *md5_file);
int md5_sum(char *src, char *sum, int type);

#endif	//_MD5SUM_H_
