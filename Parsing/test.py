from dofuzz import dofuzz

path = '/home/jake/project/libfuzzer_stuff/binutils-2.29'
filepath = 'binutils/objdump.c'
executable = 'objdump'
fuzzer = '../libFuzzer.a'
# ob = dofuzz(path, filepath, executable, fuzzer,
#             use_default=False, prototype='int ping_echo(char* hostname)')
ob = dofuzz(path, filepath, executable, fuzzer)
