/*! \file loop_buf_base.h
    \brief Loop buffer base class.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _LOOP_BUF_BASE_H_
#define _LOOP_BUF_BASE_H_

class LoopBufBase {
public:
    LoopBufBase(int size);
    ~LoopBufBase(){};
    
    int DataNum(){ int i = head_-tail_; return i<0?buf_size_+1+i:i; };  //Return number of valid data
    void Clear() { head_ = tail_ = 0; };
    void Seek(int pos);

    //Accessors
    int buf_size(){ return buf_size_; };
    int head(){ return head_; };
    int tail(){ return tail_; };
    //Mutators
    void set_head(int val) { head_ = val; };
    void set_tail(int val) { tail_ = val; };
    int tail_;
    
protected:
    int buf_size_;
    int head_;
    int pos_;
private:
};

#endif  //_LOOP_BUF_BASE_H_

