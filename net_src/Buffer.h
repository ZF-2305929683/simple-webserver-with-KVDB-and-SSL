#pragma once
#include <string>
#include "Macros.h"


class Buffer
{
private:
    std::string buf_;
public:
    Buffer();
    ~Buffer();
    ssize_t size();

    DISALLOW_COPY_AND_MOVE(Buffer);

    void append(const char* _str,int _size);
    const char* c_str();
    void clear();
    void getline();
    void SetBuf(const char *buf);
    void SetBuf(const char *buffer, size_t length);
};

