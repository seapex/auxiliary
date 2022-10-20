/*! \file buffer_pool.h
    \brief display data buffer pool.
    Copyright (c) 2018  Xi'an Boyuu Electric, Inc.
*/
#ifndef _DATA_BUFS_H_
#define _DATA_BUFS_H_

#include "data_buf.h"

class DataBufs
{
    DataBuf **data_buf_;
    int max_bufs_;  //Maximum number of data_buf_
public:
	DataBufs();
	~DataBufs();

    void Initialize(int max);
    
    #define PRECOND if(idx>=max_bufs_||idx<0)
    //Accessors
    DataBuf *buf(int idx) { PRECOND return NULL; return data_buf_[idx]; }
    //Mutators
    void del_buf(int idx) { PRECOND return; if (data_buf_[idx]) { delete data_buf_[idx]; data_buf_[idx] = NULL; } }
    void new_buf(int idx) { PRECOND return; if (!data_buf_[idx]) data_buf_[idx] = new DataBuf; }
protected:
};

DataBufs & data_bufs();

#endif // _DATA_BUFS_H_
