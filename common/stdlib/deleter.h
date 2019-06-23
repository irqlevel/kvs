#pragma once

#include "base.h"

namespace Stdlib
{

template<class T>
class DefaultObjectDeleter
{
public:
    void operator ()(T *obj) {
        delete obj;
    }
};

}