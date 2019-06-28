#pragma once

#include <common/stdlib/error.h>

#define LbsErrNoMemory     STDLIB_ERROR(1, "lbs", "no memory")
#define LbsErrInvalidRequestType STDLIB_ERROR(2, "lbs", "invalid request type")
#define LbsErrInvalidRequest STDLIB_ERROR(3, "lbs", "invalid request")
#define LbsErrDiskAlreadyExists STDLIB_ERROR(4, "lbs", "disk already exists")
#define LbsErrDiskNotFound STDLIB_ERROR(5, "lbs", "disk not found")
#define LbsErrHexEncoding STDLIB_ERROR(6, "lbs", "hex encoding")
