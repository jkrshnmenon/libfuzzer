Automating fuzzing using libFuzzer.

Example usage:

Fuzzing main

                 from dofuzz import *

                 ob = dofuzz('/home/foo/source', 'subdir/filename.c', '../libFuzzer.a')

-------------------------------------------------------------------------------------------

                 
Fuzzing arbitrary function

                 from dofuzz import *

                 ob = dofuzz('/home/foo/source', 'subdir/filename.c', '../libFuzzer.a',
                             use_default=False, prototype='int blah(int, char*)')

-------------------------------------------------------------------------------------------


Example fuzzing binutils/objdump from main

		from dofuzz import *

		ob = dofuzz('~/binutils-2.29', 'binutils/objdump.c', 'objdump', '../libFuzzer.a')

-------------------------------------------------------------------------------------------
