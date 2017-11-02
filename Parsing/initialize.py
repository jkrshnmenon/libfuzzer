import os
import sys
from subprocess import PIPE, check_call, Popen


class initialize:
    def __init__(self, path):
        self.makeout = ''
        if 'configure' not in os.listdir(path):
            print "Wrong path"
            sys.exit()
        os.chdir(path)
        if check_call(['./configure', 'CFLAGS=-fPIC'],
                      stdout=PIPE, stderr=PIPE) != 0:
            print "Couldn't configure"
            sys.exit()
        p = Popen(['make', 'V=1'], stdout=PIPE, stderr=PIPE)
        self.makeout = p.communicate()[0]
        if p.returncode != 0:
            print "Couldn't make"
            sys.exit()

    def getOutput(self):
        return self.makeout.split('\n')
