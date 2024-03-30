#ifndef LOOP_BUF_SORT_H_
#define LOOP_BUF_SORT_H_

#include "loop_buffer.h"
#include <cstdio>
using namespace std;

template <class T>
class LoopBufSort:public LoopBuffer<T> {
    using LoopBuffer<T>::buf_size_;
    using LoopBuffer<T>::trash_;
    using LoopBuffer<T>::buffer_;
    using LoopBuffer<T>::data_num_;
    using LoopBuffer<T>::head_;
    using LoopBuffer<T>::tail_;
public:
    LoopBufSort(int size, int (*compfunc)(const void *, const void *), const char *fname);
    ~LoopBufSort();
    int SaveFile(const char *fname);
    int LoadFile(const char *fname);
    
    int Insert(T * data);
    int InsertMax(T * data);
    int Match(T *data, int type);
        
protected:
private:
    int (*compare_)(const void *, const void *);

    T * tmp_buf_;
};

template <class T>
LoopBufSort<T>::LoopBufSort(int size, int (*compfunc)(const void *, const void *), const char *fname):LoopBuffer<T>(size)
{
    compare_ = compfunc;
    tmp_buf_ = new T[buf_size_];
    if (fname) LoadFile(fname);
}

template <class T>
LoopBufSort<T>::~LoopBufSort()
{
    delete [] tmp_buf_;
}

/*!
Insert data into buffer in ascending order(head=max, tail=min).

    Input:  data -- be inserted
    Return: 0=success, 1=buffer be overflow
*/
template <class T>
int LoopBufSort<T>::Insert(T * data)
{
    T min, max, *ptmp;
    int num = data_num_;
    int retval = 0;
    if (!num) this->Push(data);
    else {
        this->set_offset(0);     this->Read(&min);
        this->set_offset(num-1); this->Read(&max);
        if (compare_(data, &max)>0) retval = this->Push(data);    //data>max
        else {
            if (compare_(data, &min)<0) {                   //data<min
                memcpy(tmp_buf_, data, sizeof(T));
                ptmp = &tmp_buf_[1];
                while (!this->Pop(ptmp++));
                if (num<buf_size_) {
                    num++;
                } else {
                    memcpy(&trash_, &tmp_buf_[buf_size_], sizeof(T));
                    retval = 1;
                }
            } else {                                        //data∈[min,max]
                ptmp = tmp_buf_;
                while (!this->Pop(ptmp++));
                if (!bsearch(data, tmp_buf_, num, sizeof(T), compare_)) {   //not found
                    memcpy(&tmp_buf_[num], data, sizeof(T));
                    num++;
                    qsort(tmp_buf_, num, sizeof(T), compare_);
                    if (num>buf_size_) retval = 1;
                }
            }
            ptmp = tmp_buf_;
            for (int i=0;i<num;i++) {
                this->Push(ptmp++);
            }
        }
    }    
    return retval;
}

/*!
Insert data into buffer in ascending order. If overflow, pop minimum data first

    Input:  data -- be inserted
    Return: 0=success, 1=buffer be overflow
*/
template <class T>
int LoopBufSort<T>::InsertMax(T * data)
{
    T min, max, *ptmp;
    int num = data_num_;
    int retval = 0;
    if (!num) this->Push(data);
    else {
        this->set_offset(0);     this->Read(&min);
        this->set_offset(num-1); this->Read(&max);
        if (compare_(data, &max)>0) retval = this->Push(data);    //data>max
        else {
            if (compare_(data, &min)<0) {                   //data<min
            } else {                                        //data∈[min,max]
                ptmp = tmp_buf_;
                while (!this->Pop(ptmp++));
                if (!bsearch(data, tmp_buf_, num, sizeof(T), compare_)) {   //not found
                    memcpy(&tmp_buf_[num], data, sizeof(T));
                    num++;
                    qsort(tmp_buf_, num, sizeof(T), compare_);
                    if (num>buf_size_) retval = 1;
                }
            }
            ptmp = tmp_buf_;
            for (int i=0;i<num;i++) {
                this->Push(ptmp++);
            }
        }
    }    
    return retval;
}

/*!
Description:Lookup data in buffer_

    Input:  data -- be lookup
            type -- match type. -1=(>=data), 0=(==data), 1=(<=data)
    Return: offset of data be found, -1=not found
*/
template <class T>
int LoopBufSort<T>::Match(T *data, int type)
{
    T min, max, tmpi;
    int retval = -1;
    int num = data_num_;
    this->set_offset(0); this->Read(&min);
    this->set_offset(num-1); this->Read(&max);
    if (compare_(data, &min)<0) {       //data < min
        retval = type<0?0:-1;
    } else if (compare_(data, &max)>0) { //data > max
        retval = type>0?0:-1;
    } else {                            //data∈[min, max]
        int i = 0;
        this->set_offset(i);
        for (;i<num;i++) {
            this->Read(&tmpi);
            if (compare_(&tmpi, data)==0) {
                break;
            } else if (compare_(&tmpi, data)>0) {
                if (!type) i = -1;
                else if (type>0) i--;
                break;
            }
        }
        retval = i;
    }
    return retval;
}

struct LoopBufSave {
    int size;   //buf_size_
    int head;
    int tail;
    int num;    //data_num_
    uint8_t ver;    //19. initial version
    uint8_t reserved[11];
};
/*!
Load value of object from file

    Input:  fname -- file name be load
    Return: 0=success, 1=can't open file, 2=read file error, 3=hd.size<1
*/
template <class T>
int LoopBufSort<T>::LoadFile(const char *fname)
{
    int retval = 0;
    FILE *f_strm = fopen(fname, "rb");
    if (f_strm) {   //File be opened successfully
        LoopBufSave hd;
        int i = fread(&hd, sizeof(LoopBufSave), 1, f_strm);   //read head of save file
        if (i == 1) {
            if ( hd.size>1 ) {
                buf_size_ = hd.size;
                if (buffer_) delete [] buffer_;
                buffer_ = new T[buf_size_];
                if (tmp_buf_) delete [] tmp_buf_;
                tmp_buf_ = new T[buf_size_];
                head_ = hd.head;
                tail_ = hd.tail;
                if (hd.ver==19) data_num_ = hd.num;
                else {
                    data_num_ = hd.head - hd.tail;
                    if (data_num_<0) data_num_ += buf_size_;
                }
                i = fread(buffer_, sizeof(T), hd.size, f_strm);   //read buffer
                if (i != hd.size) retval = false;
            } else retval = 3; 
        } else retval = 2;
        fclose(f_strm);
    } else retval = 1;
    return retval;
}

/*!
Description:Save value of object to file

    Input:  fname -- file name be saved
    Return: 0=success, 1=can't open file, 2=write file error, 3=hd.size<=0
*/
template <class T>
int LoopBufSort<T>::SaveFile(const char *fname)
{
    int retval = 0;
    FILE *f_strm = fopen(fname, "rb+");
    if (f_strm == NULL) {
        printf("file:%s not exist! Create it\n", fname);
        f_strm = fopen(fname, "wb");
    }
    LoopBufSave hd;
    hd.size = buf_size_;
    hd.head = head_;
    hd.tail = tail_;
    hd.num = data_num_;
    hd.ver = 19;
    int i = fwrite(&hd, sizeof(LoopBufSave), 1, f_strm);
    if (i == 1) {
        i = fwrite(buffer_, sizeof(T), hd.size, f_strm);
        if (i != hd.size) retval = 2;
    } else retval = 2;
    fclose(f_strm);
    return retval;
}

#endif  //LOOP_BUF_SORT_H_

