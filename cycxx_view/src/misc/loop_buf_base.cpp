#include "loop_buf_base.h"
#include <stdio.h>

LoopBufBase::LoopBufBase(int size)
{
    if ( size > 1 ) {
        size--;
        for (buf_size_=0xffff; size<=buf_size_; buf_size_ >>= 1);
        buf_size_ = (buf_size_ << 1) + 2;
    } else buf_size_ = 0;
    head_ = 0;
    tail_ = 0;
    offset_ = 0;
    data_num_ = 0;
}


