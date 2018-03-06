import re
import sys
from os import system
from subprocess import check_call, CalledProcessError


class backup:
    def __init__(self, filename, makeout):
        self.filename = filename
        objectfile = filename.replace('.c', '.o')
        self.regex = re.compile('[ |\n]main *\(.*\)')
        self.sub = ' runner(int argc, char **argv)'
        self.internal_string = re.compile('"".*""')
        self.fixed_string = '"\\"\\""'
        cmd = ''
        for line in makeout:
            fixed_line = re.sub(' +', ' ', line).replace('\\', '')
            if '-o '+objectfile in fixed_line:
                cmd = fixed_line
                break
        if cmd is '':
            print "Couldn't find command"
            sys.exit()
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
            offset1 = x.find('""')
            offset2 = x.rfind('""')
            if offset1 != -1 and offset2 != -1 and offset1 != offset2:
                retval = re.sub('""', '"\\"', x[:offset2])
                retval += re.sub('""', '\\""', x[offset2:])
                cmd[cmd.index(x)] = retval

    def modifyFile(self, cmd):
        self.clean(cmd)
        f = open(self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)
        check_call(''.join(x+' ' for x in cmd), shell=True)
