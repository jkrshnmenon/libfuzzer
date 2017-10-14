import sys
import re

code = '''
#include <stdint.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
'''

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Usage: {} prototype".format(sys.argv[0])
        sys.exit()
    prototype = re.sub(' *\(', '(', sys.argv[1])
    newargs = []
    name = re.sub('\**', '', prototype.split('(')[0].split(' ')[-1])
    args = re.sub(', *', ',', prototype.split('(')[1].split(')')[0])
    if args.count(',') < 1:
        newargs = args.split(' ')[:-1]
        newargs[0] += '*'*args.split(' ')[-1].count('*')
    else:
        for x in args.split(','):
            dtype = ''.join(y for y in x.split(' ')[:-1])
            dtype += '*'*x.split(' ')[-1].count('*')
            newargs.append(dtype)
