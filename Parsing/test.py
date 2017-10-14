import makefuzzer

ob = makefuzzer.makeFuzzer('extern int ping_echo (char *hostname);')
ob.writeCode()
