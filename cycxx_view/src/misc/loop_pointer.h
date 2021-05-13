#ifndef _LOOP_POINTER_H_
#define _LOOP_POINTER_H_

#include <cstring>
using namespace std;
#include "loop_buf_base.h"

template <class T>
class LoopPointer:public LoopBufBase {
public:
    LoopPointer(int size);
    ~LoopPointer();
    
    int Push(T * data);
    T* Pop();

    T* Read(int idx);
    T* trash() { return trash_; };
   
protected:
    T *trash_;
    T **buffer_;
private:
};

template <class T>
LoopPointer<T>::LoopPointer(int size):LoopBufBase(size)
{
    buffer_ = new T*[buf_size_+1];
}

template <class T>
LoopPointer<T>::~LoopPointer()
{
    delete [] buffer_;
}

//Return:   0=success, 1=buffer be overflow
template <class T>
int LoopPointer<T>::Push(T *data)
{
    T* *ptemp = buffer_ + head_;
    *ptemp = data;
    
    ++head_ &= buf_size_;
    if (tail_==head_) {
        ptemp = buffer_ + tail_;
        //delete [] *ptemp;
        trash_ = *ptemp;
        ++tail_ &= buf_size_;
        return 1;
    } else {
        trash_ = NULL;
    }
    return 0;
}

//Return:   NULL=buffer be empty
template <class T>
T* LoopPointer<T>::Pop()
{
    if (tail_==head_) return NULL;
    T* *ptemp = buffer_ + tail_;
    ++tail_ &= buf_size_;
    return *ptemp;
}

/*!
    Input:  idx -- 0=1st data in buffer
    Return: NULL=buffer be empty
*/
template <class T>
T* LoopPointer<T>::Read(int idx=0)
{
    if (idx>=DataNum()) return NULL;
    int pos = tail_ + idx;
    pos &= buf_size_;
    T* *ptemp = buffer_ + pos;
    return *ptemp;

}

#endif  //_LOOP_POINTER_H_

