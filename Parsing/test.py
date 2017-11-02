from dofuzz import dofuzz

path = '/home/jake/project/inetutils-1.9.4'
filepath = 'ping/ping.c'
fuzzer = '../libFuzzer.a'
ob = dofuzz(path, filepath, fuzzer)
