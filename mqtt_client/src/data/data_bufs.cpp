#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data_bufs.h"

DataBufs & data_bufs()
{
    static DataBufs dbufs;
    return dbufs;
}

DataBufs::DataBufs()
{
    data_buf_ = NULL;
    max_bufs_ = 0;
}

DataBufs::~DataBufs()
{
    if (data_buf_) delete [] data_buf_;
}

/*!
Initialize data buffers

    Input:  max -- maximum number of buffer
*/
void DataBufs::Initialize(int max)
{
    max_bufs_ = max;
    data_buf_ = new DataBuf*[max];
    memset(data_buf_, 0, sizeof(DataBuf*)*max);
}

