from subprocess import check_output
import os


class compilefuzzer:
    def __init__(self, libflags, fuzzer, compile1):
        print compile1
        check_output(compile1)
        self.cmd = ['clang++']
        self.cmd += ['-g', '-fsanitize=address',
                     '-fsanitize-coverage=func,trace-pc']
        self.cmd += ['-o', 'test', 'test.cpp', 'libproject.so']
        self.cmd += libflags
        self.cmd += [fuzzer]
        check_output(self.cmd)
