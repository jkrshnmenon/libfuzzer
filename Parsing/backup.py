import re
import sys
from subprocess import check_call, Popen


class backup:
    def __init__(self, filename, cmd):
        self.filename = filename
        self.regex = re.compile('[ |\n]main *\(.*\)')
        self.sub = ' runner(int argc, char **argv)'
        self.modifyFile(cmd)

    def modifyFile(self, cmd):
        f = open(self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)
        Popen(cmd)
