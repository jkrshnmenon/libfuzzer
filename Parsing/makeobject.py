import re
import os
import subprocess


class makeObject:
    def __init__(self, makeout, executable):
        self.makeout = makeout
        self.executable = executable
        self.args = []
        self.static_libs = []
        self.ldflags = []
        self.depends = []
        self.cmd = ['clang', '-shared', '-o', 'libproject.so']
        self.runner()

    def getContents(self):
        cmd = '-o '+self.executable+' '
        for line in self.makeout:
            fixed_line = re.sub(' +', ' ', line)
            if cmd in fixed_line:
                if fixed_line.startswith('/bin/bash'):
                    continue
                return fixed_line.strip().split(' ')

    def getLibs(self):
        args = self.getContents()
        if args is None:
            raise ParseException
        if len(args) == 0:
            raise ParseException
        for x in args:
            if x.endswith('.a'):
                self.static_libs.append(x)
            elif x[:2] == '-l':
                self.ldflags.append(x)
            elif x[:2] == '-L':
                self.ldflags.append(x)
            elif x.endswith('.o'):
                self.depends.append(x)

    def getCommand(self):
        self.getLibs()
        self.cmd += list(set(self.depends))
        self.cmd += list(set(self.static_libs))
        if len(self.ldflags) > 0:
            self.cmd += ['-Wl,--whole-archive']
            self.cmd += list(set(self.ldflags))
            self.cmd += ['-Wl,--no-whole-archive']

    def getLibFlags(self):
        return self.ldflags+self.static_libs

    def getCmd(self):
        return self.cmd

    def runner(self):
        self.getCommand()
        try:
            subprocess.check_output(self.cmd)
        except subprocess.CalledProcessError:
            self.cmd.pop(self.cmd.index('-Wl,--whole-archive'))
            self.cmd.pop(self.cmd.index('-Wl,--no-whole-archive'))
            subprocess.check_output(self.cmd)


class ParseException(Exception):
    def __init__(self, *args, **kwargs):
        Exception.__init__(self, *args, **kwargs)
