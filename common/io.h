#pragma once

#include "base.h"

#include <common/stdlib/error.h>
#include <common/stdlib/result.h>

namespace IO
{

class Reader
{
public:
    virtual Stdlib::Result<size_t, Stdlib::Error> Read(void* buf, size_t nbytes) = 0;
};

class Writer
{
public:
    virtual Stdlib::Result<size_t, Stdlib::Error> Write(const void *buf, size_t nbytes) = 0;
};

class Closer
{
public:
    virtual void Close() = 0;
};

class ReadCloser : public Reader, public Closer
{
};

class WriteCloser : public Writer, public Closer
{
};

class ReadWriter : public Reader, public Writer
{
};

class ReadWriteCloser : public ReadWriter, public Closer
{
};

class Serializable
{
public:
    virtual int Serialize(Writer &writer) = 0;
    virtual int Deserialize(Reader &reader) = 0;
    virtual ~Serializable() {};
};


}