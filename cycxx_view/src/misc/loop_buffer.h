#ifndef _LOOP_BUFFER_H_
#define _LOOP_BUFFER_H_

#include "loop_buf_base.h"
#include <cstring>
using namespace std;

template <class T>
class LoopBuffer:public LoopBufBase {
public:
    LoopBuffer(int size);
    ~LoopBuffer();
    
    int Push(const T * data);
    int Pop(T * data);
    T *Pop();
    
    T* Read(int idx);
    int Read(T * data);
    void rRead(T * data);
    void GetTrash(T * data) { memcpy(data, &trash_, sizeof(T)); }
    
protected:
    T trash_;
    T * buffer_;
private:
};

template <class T>
LoopBuffer<T>::LoopBuffer(int size):LoopBufBase(size)
{
    buffer_ = new T[buf_size_];
}

template <class T>
LoopBuffer<T>::~LoopBuffer()
{
    delete [] buffer_;
}

/*!
    Output: data -- 
    Return: 0=success, -1=buffer be empty
*/
template <class T>
int LoopBuffer<T>::Pop(T * data)
{
    if (data_num_==0) return -1;
    data_num_--;
    T *ptemp = buffer_ + tail_;
    memcpy(data, ptemp, sizeof(T));
    ++tail_ &= buf_size_-1;
    return 0;
}

/*!
    Return:   NULL=buffer be empty
*/
template <class T>
T *LoopBuffer<T>::Pop()
{
    if (data_num_==0) return NULL;
    data_num_--;
    T *ptemp = buffer_ + tail_;
    ++tail_ &= buf_size_-1;
    return ptemp;
}

/*!
    Input:  data -- 
    Return: 0=success, 1=buffer be overflow
*/
template <class T>
int LoopBuffer<T>::Push(const T * data)
{
    int retv = 0;
    T *ptemp = buffer_ + head_;
    if (data_num_<buf_size_) {
        data_num_++;
    } else {
        memcpy(&trash_, ptemp, sizeof(T));
        ++tail_ &= buf_size_-1;
        retv = 1;
    }
    memcpy(ptemp, data, sizeof(T));   
    ++head_ &= buf_size_-1;
    return retv;
}

/*!
    Input:  idx -- 0=1st data in buffer
    Return: NULL=buffer be empty
*/
template <class T>
T* LoopBuffer<T>::Read(int idx=0)
{
    if (idx>=data_num_) return NULL;
    T *ptemp = buffer_ + (tail_+idx & buf_size_-1);
    return ptemp;
}

/*!
    Output: data -- 
    Return:   0=success, -1=No more data to read
*/
template <class T>
int LoopBuffer<T>::Read(T * data)
{
    if (offset_>=data_num_) return -1;
    T *ptemp = buffer_ + (tail_+offset_ & buf_size_-1);
    memcpy(data, ptemp, sizeof(T));
    offset_++;
    return 0;
}
/*!
Read in reverse order
    
    Output: data -- 
*/
template <class T>
void LoopBuffer<T>::rRead(T * data)
{
    T *ptemp = buffer_ + (tail_+offset_ & buf_size_-1);
    memcpy(data, ptemp, sizeof(T));
    offset_--;
}

#endif  //_LOOP_BUFFER_H_

