#include "loop_buf_base.h"

LoopBufBase::LoopBufBase(int size)
{
    if ( size>0 ) {
        for (buf_size_=0xffff; size<=buf_size_; buf_size_ >>= 1);
        buf_size_ = (buf_size_<<1) + 1;
    } else buf_size_ = 0;

    head_ = 0;
    tail_ = 0;
    pos_ = 0;
}

/*!

	Input:    offset -- 0=first
*/
void LoopBufBase::Seek(int offset)
{
    if (offset>=buf_size_) offset = buf_size_-1;
    
    pos_ = tail_ + offset;
    pos_ &= buf_size_;
}
