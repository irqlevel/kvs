#pragma once

#include <common/base.h>

namespace Sync
{

class Process
{
public:
    static int Getpid();
    static void Exit(int status);
};

}