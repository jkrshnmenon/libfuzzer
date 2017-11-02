from dofuzz import dofuzz

path = '/home/jake/project/inetutils-1.9.4'
filepath = 'ping/ping.c'
executable = 'ping'
fuzzer = '../libFuzzer.a'
prototype = 'int ping_echo (char *hostname)'
ob = dofuzz(path, filepath, executable, fuzzer, use_default=False, prototype=prototype)
