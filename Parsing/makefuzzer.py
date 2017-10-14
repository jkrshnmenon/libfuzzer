from re import sub


class makeFuzzer:
    def __init__(self, prototype):
        self.prototype = sub(' *\(', '(', prototype.split('{')[0])
        self.args = []
        self.invocation = ""
        self.name = ""

    def getArgs(self):
        self.name = sub('\**', '', self.prototype.split('(')[0].split(' ')[-1])
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

    def getCode(self):
        self.getInvocation()
        self.code = '#include<stdint.h>\n#include<stdio.h>\n'
        self.code += self.prototype
        if self.prototype.count(';') < 1:
            self.code += ';'
        self.code += '\n'
        self.code += 'extern "C" int LLVMFuzzerTestOneInput'
        self.code += '(const uint8_t *Data, size_t Size)\n'
        self.code += 'int main() {\n'
        self.code += ' '*4 + self.invocation
        self.code += '    return 0;\n}'

    def writeCode(self):
        self.getCode()
        f = open('test.cpp', 'w')
        f.write(self.code+'\n')
        f.close()
