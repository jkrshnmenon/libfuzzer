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
            fixed_line = re.sub(' +', ' ', line).replace('\\', '')
            if '-o '+objectfile in fixed_line:
                cmd = fixed_line
        self.modifyFile(cmd.replace(';', '').split(' '))

    def clean(self, cmd):
        if '-MT' in cmd:
            idx = cmd.index('-MT')
            cmd.pop(idx+1)
            cmd.pop(idx)
        if '-MF' in cmd:
            idx = cmd.index('-MF')
            cmd.pop(idx+1)
            cmd.pop(idx)
        if '-MD' in cmd:
            cmd.pop(cmd.index('-MD'))
        if '-MP' in cmd:
            cmd.pop(cmd.index('-MP'))
        for x in cmd:
            if x is '':
                cmd.pop(cmd.index(x))

    def modifyFile(self, cmd):
        self.clean(cmd)
        f = open(self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)
        check_call(cmd)
