from backup import backup
from os import listdir, chdir, environ
from initialize import initialize
from makefuzzer import makeFuzzer
from makeobject import makeObject
from subprocess import check_output, PIPE, call
from compilefuzzer import compilefuzzer


class dofuzz:
    def __init__(self, path, filepath, executable,
                 fuzzer, use_default=True, prototype=''):
        """
        path => Path to the root directory of source to be fuzzed
        filepath => Relative path of file containing main from root directory
                of source
        executable => Name of final executable generated
        fuzzer => Relative path to fuzzer from root directory of source
        use_default => Flag indicating whether to fuzz a particular function or
                       start fuzzing from main
        prototype => prototype of function to be fuzzed. Will only be
                     considered if use_default is True
        """
        self.template = open('template.cpp', 'r').read()
        self.path = path
        self.filename = filepath.split('/')[-1]
        self.executable = executable
        self.sourcepath = ''.join(x for x in filepath.split('/')[:-1])
        self.libpath = self.path+'/'+self.sourcepath

        print "[-] Intializing"
        initializeObject = initialize(self.path)
        print "[+] Done"
        chdir(self.sourcepath)
        open('test.cpp', 'w').write(self.template)

        print "[-] Modifying main to runner"
        backupObject = backup(self.filename,
                              initializeObject.getOutput())
        print "[+] Done"

        print "[-] Building shared object"
        libraryObject = makeObject(initializeObject.getOutput(),
                                   self.executable)
        print "[+] Done"

        if use_default is False and prototype is not '':
            print "[-] Creating fuzzer targeting arbitrary function"
            customObject = makeFuzzer(prototype)
            print "[+] Done"

        print "[-] Compiling fuzzer"
        compilefuzzer(libraryObject.getLibFlags(), '../'+fuzzer)
        print "[+] Done"

        print "[-] Starting fuzzing"
        call('./test', env=dict(environ,
                                LD_LIBRARY_PATH=self.libpath))
