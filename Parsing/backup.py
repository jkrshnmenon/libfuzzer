import re
import sys
import os
from subprocess import check_call


class backup:
    def __init__(self, filename, makeout):
        self.filename = filename
        objectfile = filename.replace('.c', '.o')
        self.regex = re.compile('[ |\n]main *\(.*\)')
        self.sub = ' runner(int argc, char **argv)'
        for line in makeout:
            fixed_line = re.sub(' +', ' ', line)
            if '-o '+objectfile in fixed_line:
                cmd = fixed_line
        self.modifyFile(cmd)

    def modifyFile(self, cmd):
        f = open(self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)
        check_call(cmd.split(' '))
