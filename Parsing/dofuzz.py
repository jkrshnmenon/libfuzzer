from backup import backup
from os import chdir, environ
from initialize import initialize
from makefuzzer import makeFuzzer
from makeobject import makeObject
from subprocess import check_output
from compilefuzzer import compilefuzzer


class dofuzz:
    def __init__(self, path, filepath, fuzzer, use_default=True, prototype=''):
        """
        path => Path to the root directory of source to be fuzzed
        filepath => Relative path of file containing main from root directory
                of source. Should be same as the final executable generated.
        fuzzer => Relative path to fuzzer from root directory of source
        use_default => Flag indicating whether to fuzz a particular function or
                       start fuzzing from main
        prototype => prototype of function to be fuzzed. Will only be
                     considered if use_default is True
        """
        self.template = open('template.cpp', 'r').read()
        self.path = path
        self.filename = filepath.split('/')[-1]
        self.sourcepath = ''.join(x for x in filepath.split('/')[:-1])

        print "[-] Intializing"
        initializeObject = initialize(self.path)
        print "[+] Done"
        chdir(self.sourcepath)
        open('test.cpp', 'w').write(self.template)
        print "[-] Building shared object"
        libraryObject = makeObject(initializeObject.getOutput(),
                                   self.filename[:-2])
        print "[+] Done"
        if use_default is True:
            print "[-] Modifying main to runner"
            backupObject = backup(self.filename,
                                  libraryObject.getCompileCmd())
            print "[+] Done"

        elif use_default is False and prototype is not '':
            print "[-] Creating fuzzer targeting arbitrary function"
            customObject = makeFuzzer(prototype)
            print "[+] Done"

        print "[-] Compiling fuzzer"
        compilefuzzer(libraryObject.getLibFlags(),
                      '../'+fuzzer, libraryObject.getCmd())
        print "[+] Done"

        print "[-] Starting fuzzing"
        check_output('./test', env=dict(environ,
                                        LD_LIBRARY_PATH=environ['PWD']))
