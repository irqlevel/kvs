#pragma once

namespace Stdlib
{

class Errno final
{
public:
    static int Get();
    static void Reset(int value);
};

}