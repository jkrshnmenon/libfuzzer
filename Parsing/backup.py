import re
import os
from subprocess import check_output


class backup:
    def __init__(self, filename):
        self.filename = filename
        self.regex = re.compile('[ |\n]main *\(.*\)')
        self.sub = ' runner(int argc, char **argv)'
        self.modifyFile()
        self.buildAgain()

    def modifyFile(self):
        f = open(self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)

    def buildAgain(self):
        if check_call(['./configure', 'CFLAGS=-fPIC'],
                      stdout=PIPE, stderr=PIPE) != 0:
            print "Couldn't configure"
            sys.exit()
        p = subprocess.Popen('make', stdout=PIPE, stderr=PIPE)
        if p.returncode != 0:
            print "Couldn't make"
            sys.exit()
