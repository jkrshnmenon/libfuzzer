initialize.py

                 Changes dir to the specified directory

                 Executes `configure`

                 Executes `make V=1` and saves output for makeobject to use

-------------------------------------------------------------------

backup.py

                 Creates backup of file which contains `main`

                 Modifies the file to change `main` to `runner`

                 Configure and make with CFLAGS=-fPIC


-------------------------------------------------------------------

makeobject.py    

                 Parses output of make to extract relevant information

                 Creates a shared object (libproject.so) using this information

-------------------------------------------------------------------

makefuzzer.py

                 Parses function prototype

                 Determines if function can be targeted or not.

                 Creates CPP code for fuzzing target function

-------------------------------------------------------------------

compilefuzzer.py

                 Compiles test.cpp to generate fuzzer

                 test.cpp can be either the template for fuzzing main

                 Or the code to target individual functions

-------------------------------------------------------------------

dofuzz.py

                 Invokes all the above scripts in order to create final fuzzer

                 Executes fuzzer with the updated LD_LIBRARY_PATH

-------------------------------------------------------------------
