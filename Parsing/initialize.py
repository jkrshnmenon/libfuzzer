import re
import os
import subprocess


class initialize:
    def __init__(self, filename, path):
        self.filename = filename
        self.path = path
        self.regex = re.compile('[ |\n]main *\(.*\)')
        self.sub = ' runner(int argc, char **argv)'

    def modifyFile(self):
        f = open(self.path+'/'+self.filename, 'r')
        inp = f.read()
        f.close()
        open(self.filename+'.bak', 'w').write(inp)
        self.out = re.sub(self.regex, self.sub, inp)
        open(self.filename, 'w').write(self.out)

    def buildAgain(self):
        subprocess.check_output(['./configure'], "CFLAGS='-fPIC'")
        subprocess.check_output('make')

    def changeDir(self):
        os.chdir(self.path)
