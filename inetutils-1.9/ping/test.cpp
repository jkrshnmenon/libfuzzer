extern "C"{
#include "ping.h"
}

extern "C" int LLVMFuzzerTestOneInput( const uint8_t *Data, size_t Size)
{
    double a = 0.0;
    nabs(a);
    return 0;
}
