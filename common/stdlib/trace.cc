#include "trace.h"

#include <stdlib.h>
#include <stdio.h>

namespace Stdlib
{

Tracer::Tracer()
    : Level(0)
{
}

void Tracer::SetLevel(int level)
{
    Level = level;
}

int Tracer::GetLevel()
{
    return Level;
}

Tracer::~Tracer()
{
}

void Tracer::Output(const char *fmt, ...)
{
    char msg[256];

    va_list args;
    va_start(args, fmt);
    int size = VsnPrintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    if (size < 0)
        return;

    if (OutputDst != nullptr)
        OutputDst->Write(msg, size);
    else
        ::fprintf(stdout, "%s", msg);
}

void Tracer::SetOutput(IO::Writer *outputDst)
{
    OutputDst = outputDst;
}

}