#include<iostream>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

extern "C" void decode_pattern(const char *text, int *pattern_len, unsigned char *pattern_data);

extern "C" int LLVMFuzzerTestOneInput( const uint8_t *Data, size_t Size)
{
    char *ptr = (char *)malloc(Size);
    memcpy(ptr, Data, Size);
    int size = (int)Size;
    decode_pattern((const char *)ptr, &size, (unsigned char *)Data);
    return 0;
}
