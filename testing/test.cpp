#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int func(char *buf, int size) {
    int i = 0;
    char *p = (char *) malloc(size);
    if ( size == 0)
        exit(1);
    for(i = 0; i <= size; i++)
        p[i] = buf[i];
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    func((char *)Data, (int) Size);
    return 0;
}
    
