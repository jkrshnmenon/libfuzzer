Testing unexit.so with _exit()_

    INFO: Seed: 2964117788
    INFO: Loaded 0 modules (0 guards): 
    INFO: -max_len is not provided, using 64
    INFO: A corpus is not provided, starting from an empty corpus
    #0	READ units: 1

-----------------------------------------------------------------------------------

Testing unext.so without _exit()_ 

    INFO: Seed: 3173239660
    INFO: Loaded 0 modules (0 guards): 
    INFO: -max_len is not provided, using 64
    INFO: A corpus is not provided, starting from an empty corpus
    #0	READ units: 1
    =================================================================
    ==2750==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000071 at pc 0x00000051a658 bp 0x7ffd27045080 sp 0x7ffd27045078
    READ of size 1 at 0x602000000071 thread T0
        #0 0x51a657 in func(char*, int) /home/jake/project/test.cpp:11:16
        #1 0x51a705 in LLVMFuzzerTestOneInput /home/jake/project/test.cpp:16:5
        #2 0x52c454 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:440:13
        #3 0x52c683 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:397:3
        #4 0x52c2a1 in fuzzer::Fuzzer::RunOne(std::vector<unsigned char, std::allocator<unsigned char> > const&) /home/jake/project/./Fuzzer/FuzzerInternal.h:97:41
        #5 0x52c2a1 in fuzzer::Fuzzer::ShuffleAndMinimize(std::vector<std::vector<unsigned char, std::allocator<unsigned char> >, std::allocator<std::vector<unsigned char, std::allocator<unsigned char> > > >*) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:378
        #6 0x52524b in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long)) /home/jake/project/./Fuzzer/FuzzerDriver.cpp:741:6
        #7 0x51a750 in main /home/jake/project/./Fuzzer/FuzzerMain.cpp:20:10
        #8 0x7f0359c4482f in __libc_start_main /build/glibc-bfm8X4/glibc-2.23/csu/../csu/libc-start.c:291
        #9 0x41ca98 in _start (/home/jake/project/test+0x41ca98)

    0x602000000071 is located 0 bytes to the right of 1-byte region [0x602000000070,0x602000000071)
    allocated by thread T0 here:
        #0 0x516498 in operator new[](unsigned long) /home/jake/git_stuff/llvm/projects/compiler-rt/lib/asan/asan_new_delete.cc:95
        #1 0x52c389 in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:431:23
        #2 0x52c683 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:397:3
        #3 0x52c2a1 in fuzzer::Fuzzer::RunOne(std::vector<unsigned char, std::allocator<unsigned char> > const&) /home/jake/project/./Fuzzer/FuzzerInternal.h:97:41
        #4 0x52c2a1 in fuzzer::Fuzzer::ShuffleAndMinimize(std::vector<std::vector<unsigned char, std::allocator<unsigned char> >, std::allocator<std::vector<unsigned char, std::allocator<unsigned char> > > >*) /home/jake/project/./Fuzzer/FuzzerLoop.cpp:378
        #5 0x52524b in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long)) /home/jake/project/./Fuzzer/FuzzerDriver.cpp:741:6
        #6 0x51a750 in main /home/jake/project/./Fuzzer/FuzzerMain.cpp:20:10
        #7 0x7f0359c4482f in __libc_start_main /build/glibc-bfm8X4/glibc-2.23/csu/../csu/libc-start.c:291

    SUMMARY: AddressSanitizer: heap-buffer-overflow /home/jake/project/test.cpp:11:16 in func(char*, int)
    Shadow bytes around the buggy address:
      0x0c047fff7fb0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
      0x0c047fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
      0x0c047fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
      0x0c047fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
      0x0c047fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    =>0x0c047fff8000: fa fa 01 fa fa fa fd fa fa fa 01 fa fa fa[01]fa
      0x0c047fff8010: fa fa 01 fa fa fa fa fa fa fa fa fa fa fa fa fa
      0x0c047fff8020: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
      0x0c047fff8030: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
      0x0c047fff8040: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
      0x0c047fff8050: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
    Shadow byte legend (one shadow byte represents 8 application bytes):
      Addressable:           00
      Partially addressable: 01 02 03 04 05 06 07 
      Heap left redzone:       fa
      Freed heap region:       fd
      Stack left redzone:      f1
      Stack mid redzone:       f2
      Stack right redzone:     f3
      Stack after return:      f5
      Stack use after scope:   f8
      Global redzone:          f9
      Global init order:       f6
      Poisoned by user:        f7
      Container overflow:      fc
      Array cookie:            ac
      Intra object redzone:    bb
      ASan internal:           fe
      Left alloca redzone:     ca
      Right alloca redzone:    cb
    ==2750==ABORTING
    MS: 0 ; base unit: 0000000000000000000000000000000000000000
    0xa,
    \x0a
    artifact_prefix='./'; Test unit written to ./crash-adc83b19e793491b1c6ea0fd8b46cd9f32e592fc
    Base64: Cg==

-----------------------------------------------------------------------------------
