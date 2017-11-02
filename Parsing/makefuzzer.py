
from re import sub


class makeFuzzer:
    def __init__(self, prototype):
        self.prototype = 'extern "C" '+sub(' *\(', '(', prototype)
        self.args = []
        self.invocation = ""
        self.name = sub('\**', '', self.prototype.split('(')[0].split(' ')[-1])
        self.header = '#include<stdint.h>\n#include<stdio.h>\n'
        self.fuzzer = 'extern "C" int LLVMFuzzerTestOneInput'
        self.fuzzer += '(const uint8_t *Data, size_t Size) {\n'
        self.footer = '    return 0;\n}'
        self.writeCode()

    def getArgs(self):
        args = sub(', *', ',', self.prototype.split('(')[1].split(')')[0])
        if args.count(',') < 1:
            self.args = args.split(' ')[:-1]
            self.args[0] += "*"*args.split(' ')[-1].count("*")
        else:
            for x in args.split(','):
                dtype = ''.join(y for y in x.split(' ')[:-1])
                dtype += '*'*x.split(' ')[-1].count('*')
                self.args.append(dtype)

    def getInvocation(self):
        self.getArgs()
        self.invocation += self.name
        self.invocation += '('
        for x in self.args:
            self.invocation += '({})'.format(x)+self.substitute_variable(x)+','
        self.invocation = self.invocation[:-1]
        self.invocation += ');\n'

    def substitute_variable(self, dtype):
        if 'char*' in dtype:
            return 'Data'
        elif 'size_t' in dtype:
            return 'Size'
        elif 'int' in dtype:
            return 'Size'
        elif 'long' in dtype:
            return 'Size'
        else:
            raise CustomDatatype

    def getCode(self):
        self.getInvocation()
        self.code = self.header
        self.code += self.prototype
        if self.prototype.count(';') < 1:
            self.code += ';'
        self.code += '\n'
        self.code += self.fuzzer
        self.code += ' '*4 + self.invocation
        self.code += self.footer

    def writeCode(self):
        try:
            self.getCode()
        except CustomDatatype:
            print "Invalid arguments detected\n Try another function"
            return
        f = open('test.cpp', 'w')
        f.write(self.code+'\n')
        f.close()


class Error(Exception):
    pass


class CustomDatatype(Error):
    pass
