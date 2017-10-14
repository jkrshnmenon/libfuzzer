import re
import subprocess


class makeObject:
    def __init__(self, filename, executable):
        self.filename = filename
        self.executable = executable
        self.path = ''.join(x for x in self.executable.split('/').pop(-1))
        self.args = []
        self.static_libs = []
        self.ldflags = []
        self.depends = []
        self.cmd = ['clang', '-shared', '-o ', 'libproject.so']

    def getContents(self):
        contents = open(self.filename).readlines()
        cmd = '-o '+self.executable+' '
        for line in contents:
            fixed_line = re.sub(' +', '', line)
            if cmd in fixed_line:
                return fixed_line.strip().split(' ')

    def getLibs(self):
        args = self.getContents()
        if len(args) == 0:
            throw Exception
        for x in args:
            if x.endswith('.a'):
                self.static_libs.append(self.path+x)
            elif x[:2] == '-l':
                self.ldflags.append(self.path+x)
            elif x.endswith('.o'):
                self.depends.append(self.path+x)

    def getCommand(self):
        self.getLibs()
        self.cmd += list(set(self.depends))
        self.cmd += list(set(self.static_libs))
        if len(self.ldflags) > 0:
            self.cmd += ['-Wl, --whole-archive']
            self.cmd += list(set(ldflags))
            self.cmd += ['-Wl, --no-whole-archive']

    def getLibFlags(self):
        return self.ldflags+self.static_libs

    def runner(self):
        self.getCommand()
        return subprocess.check_output(self.cmd)
