from dofuzz import dofuzz

path = '/home/jake/project/libfuzzer_stuff/binutils-2.29'
filepath = 'binutils/ar.c'
executable = 'ar'
fuzzer = '../libFuzzer.a'
ob = dofuzz(path, filepath, executable, fuzzer,
            use_default=False, prototype='void ar_open(char *name, int t)')
# ob = dofuzz(path, filepath, executable, fuzzer)
