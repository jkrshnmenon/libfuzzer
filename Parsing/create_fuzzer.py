import sys

code = '''
     extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)\n
'''

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Usage:{} <name> <array_offset> <size_offset> [args]"
        sys.exit()
    array_offset = int(sys.argv[2])
    size_offset = int(sys.argv[3])
    args = sys.argv[4].split(' ')
    for x in xrange(len(args)):
        if args[x] == '':
            args.pop(x)
    if len(args) < max(array_offset, size_offset)-2:
        print "Not enough arguments"
        sys.exit()
    args.insert(array_offset, 'Data')
    if size_offset != 0:
        args.insert(size_offset, 'Size')
    code += '''{ %s(''' % sys.argv[1]
    code += ''.join(x+',' for x in args)
    code = code[:-1]
    code += ''');\n}'''
    print code
