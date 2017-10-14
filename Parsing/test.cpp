#include<stdint.h>
#include<stdio.h>
extern int ping_echo(char *hostname);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
int main() {
    ping_echo((char*)Data);
    return 0;
}
