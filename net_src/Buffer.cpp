#include "Buffer.h"
#include <string.h>
#include <iostream>

Buffer::Buffer(){}

Buffer::~Buffer(){}

void Buffer::append(const char* str,int size)
{
    for(int i = 0;i<size;++i)
    {
        buf_.push_back(str[i]);
    }
}

ssize_t Buffer::size()
{
    return buf_.size();
}

const char* Buffer::c_str()
{
    return buf_.c_str();
}

void Buffer::clear()
{
    buf_.clear();
}

void Buffer::getline()
{
    buf_.clear();
    std::getline(std::cin,buf_);
}

void Buffer::SetBuf(const char *buffer)
{
    buf_.clear();
    buf_.append(buffer);
}

void Buffer::SetBuf(const char *buffer, size_t length)
{
    buf_.clear();
    buf_.insert(buf_.end(), buffer, buffer + length);
}