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


![Alt text](./Output_1.png?raw=true "Output of fuzzing objdump from main")


-------------------------------------------------------------------------------------------

Example fuzzing ar_open from binutils/arsup.c

		from dofuzz import *

		ob = dofuzz('~/binutils-2.29', 'binutils/ar.c', 'ar', '../libFuzzer.a',
		            use_default=False, prototype='void ar_open(char *name, int t)')

-------------------------------------------------------------------------------------------


![Alt text](./Output_2.png?raw=true "Output of fuzzing ar_open")


-------------------------------------------------------------------------------------------
