from subprocess import check_call, PIPE


class compilefuzzer:
    def __init__(self, libflags, fuzzer):
        self.cmd = ['clang++']
        self.cmd += ['-g', '-fsanitize=address',
                     '-fsanitize-coverage=func,trace-pc']
        self.cmd += ['-o', 'test', 'test.cpp', 'libproject.so']
        self.cmd += libflags
        self.cmd += [fuzzer]
        check_call(self.cmd, stdout=PIPE, stderr=PIPE)
