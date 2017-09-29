import sys
import re
import subprocess
import os

if __name__ == '__main__':
    if len(sys.argv) <= 3:
        print "Usage: {} <make_output> <executable> <path>".format(sys.argv[0])
        sys.exit()
    make_out = open(sys.argv[1]).readlines()
    cmd = "-o "+sys.argv[2]+" "
    for line in make_out:
        if cmd in line:
            break
    if cmd not in line:
        print "Could not get command from make_output"
        sys.exit()
    os.chdir(sys.argv[3])
    args = re.sub(' +', ' ', line).strip().split(' ')
    static_libs = []
    ldflags = []
    depends = []
    for x in xrange(len(args)):
        if args[x].endswith('.a'):
            static_libs.append(args[x])
        elif args[x].endswith('-l', 0, 2):
            ldflags.append(args[x])
        elif args[x].endswith('.o'):
            depends.append(args[x])
    final_cmd = ['clang', '-shared', '-o', 'libproject.so'] 
    final_cmd += depends
    final_cmd += ['-Wl,--whole-archive']
    final_cmd += static_libs
    final_cmd += ldflags
    final_cmd += ['-lresolv', '-lnsl', '-Wl,--no-whole-archive']
    print subprocess.check_output(final_cmd)
