CC=clang
CXX=clang++
DEPDIR=.deps
FUZZER=../../libFuzzer.a
FFLAGS=-g -fsanitize=address -fsanitize-coverage=bb
INCLUDES=-I. -I.. -I../lib -I../lib -I../libinetutils
CFLAGS=-DHAVE_CONFIG_H $(INCLUDES) -fPIC -MD -MP -MF
OBJ=cmds.o cmdtab.o domacro.o ftp.o main.o ruserpass.o
LDFLAGS=-Wl,--whole-archive ../libinetutils/libinetutils.a ../lib/libgnu.a  -lreadline -ltermcap  -lresolv -lnsl -Wl,--no-whole-archive

%.o: %.c 
	$(CC) $(CFLAGS) $(DEPDIR)/$*.Tpo -c $<

libproject.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

test: test.cpp libproject.so
	$(CXX) $(FFLAGS) -o $@ $< $(INCLUDES) libproject.so -lutil $(FUZZER)

