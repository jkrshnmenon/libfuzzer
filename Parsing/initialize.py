import os
import sys
from subprocess import PIPE, check_call, Popen


class initialize:
    def __init__(self, path):
        if 'configure' not in os.listdir(path):
            print "Wrong path"
            sys.exit()
        os.chdir(path)
        if check_call('./configure', stdout=PIPE, stderr=PIPE) != 0:
            print "Couldn't configure"
            sys.exit()
        p = Popen(['make', 'V=1'], stdout=PIPE, stderr=PIPE)
        if p.returncode != 0:
            print "Couldn't make"
            sys.exit()
        self.makeout = p.communicate()[0]

    def getOutput(self):
        return self.makeout
