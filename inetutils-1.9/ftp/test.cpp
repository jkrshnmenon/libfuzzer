#include<iostream>

extern "C" char *hookup(char *host, int port);

int main(){
    hookup((char *)"asdf", 1234);
    return 0;
}
