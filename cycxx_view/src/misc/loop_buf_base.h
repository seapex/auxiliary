/*! \file loop_buf_base.h
    \brief Loop buffer base class.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _LOOP_BUF_BASE_H_
#define _LOOP_BUF_BASE_H_

class LoopBufBase {
public:
    LoopBufBase(int size);
    ~LoopBufBase(){}
    
    void Clear() { head_ = tail_ = data_num_ = 0; }

    //Accessors
    int data_num() { return data_num_; }

    //Mutators
    void set_offset(int val) { offset_ = val; }
    
protected:
    int buf_size_;
    int data_num_;  //The number of data in the buffer
    int head_;      //Points to the next data to be written
    int tail_;      //Point to the oldest data
    int offset_;    //The offset from the tail_
private:
};

#endif  //_LOOP_BUF_BASE_H_

