#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern "C" int runner(int , char**);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data,size_t Size){
    int i = 0, j = 1, k = 0;
    #define MAX_ARGS 1024
    char *buffers[MAX_ARGS+2];  //Safety. 
    buffers[0] = strdup("./a.out");
    if(Size >0 ) {
        for(i =0;i<Size;i++){
            if(Data[i]==NULL && j < MAX_ARGS){
                while(Data[i]==NULL && i<Size)  
                    i++;    //Don’t need an argv of nulls
                if(Data[i]==NULL)
                    break; //There are no more args
                buffers[j++] = strndup(Data+k,i-k);
                k = i;
            }
        }
    }
    buffers[j] = NULL;
    runner(j,buffers); 
    return 0;
}
