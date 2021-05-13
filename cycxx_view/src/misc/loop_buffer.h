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
    
    int Push(T * data);
    int Pop(T * data);
    T *Pop();
    
    int Read(T * data);
    void GetTrash(T * data) { memcpy(data, &trash_, sizeof(T)); };
    
protected:
    T trash_;
    T * buffer_;
private:
};

template <class T>
LoopBuffer<T>::LoopBuffer(int size):LoopBufBase(size)
{
    buffer_ = new T[buf_size_+1];
}

template <class T>
LoopBuffer<T>::~LoopBuffer()
{
    delete [] buffer_;
}

//Return:   0=success, 1=buffer be overflow
template <class T>
int LoopBuffer<T>::Push(T * data)
{
    T *ptemp = buffer_ + head_;
    memcpy(ptemp, data, sizeof(T));
    
    ++head_ &= buf_size_;
    if (tail_==head_) {
        ptemp = buffer_ + tail_;
        memcpy(&trash_, ptemp, sizeof(T));
        ++tail_ &= buf_size_;
        return 1;
    }
    return 0;
}

//Return:   0=success, -1=buffer be empty
template <class T>
int LoopBuffer<T>::Pop(T * data)
{
    if (tail_==head_) return -1;
    T *ptemp = buffer_ + tail_;
    memcpy(data, ptemp, sizeof(T));
    ++tail_ &= buf_size_;
    return 0;
}

/*!
    Return:   NULL=buffer be empty
*/
template <class T>
T *LoopBuffer<T>::Pop()
{
    if (tail_==head_) return NULL;
    T *ptemp = buffer_ + tail_;
    ++tail_ &= buf_size_;
    return ptemp;
}

//Return:   0=success, -1=buffer be empty
template <class T>
int LoopBuffer<T>::Read(T * data)
{
    if (pos_==head_) return -1;
    T *ptemp = buffer_ + pos_;
    memcpy(data, ptemp, sizeof(T));
    ++pos_ &= buf_size_;
    return 0;
}

#endif  //_LOOP_BUFFER_H_

