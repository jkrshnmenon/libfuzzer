from backup import *
from os import environ
from initialize import *
from makefuzzer import *
from makeobject import *
from compilefuzzer import *
from subprocess import check_output


class dofuzz:
    def __init__(self, path, filepath, use_default=True, prototype=''):
        """
        path => Path to the root directory of source to be fuzzed
        file => Relative path of file containing main from root directory of
                source. Should be same as the final executable generated.
        use_default => Flag indicating whether to fuzz a particular function or
                       start fuzzing from main
        prototype => prototype of function to be fuzzed. Will only be
                     considered if use_default is True
        """
        self.path = path
        self.filename = filepath.split['/'][-1]
        self.sourcepath = ''.join(x for x in filepath.split[:-1])
        initializeObject = initialize.initialize(self.path)
        os.chdir(self.sourcepath)
        libraryObject = makeobject.makeObject(initializeObject.getOutput(),
                                              self.filename[:-2])

        if use_default is False and filepath is not '':
            backupObject = backup.backup(filename)

        elif use_default is False and prototype is not '':
            customObject = makefuzzer.makefuzzer(prototype)

        check_output('./test', env=dict{os.environ,
                                        LD_LIBRARY_PATH=os.environ['PWD']})
