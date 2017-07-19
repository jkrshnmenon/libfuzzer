CC=clang
CXX=clang++
DEPDIR=.deps
FUZZER=../../libFuzzer.a
REST=-MD -MP -MF
FFLAGS=-g -fsanitize=address -fsanitize-coverage=bb
INCLUDES=-I. -I.. -I../lib -I../lib -I../libinetutils
CFLAGS=-DHAVE_CONFIG_H $(INCLUDES) -DTERMCAP -DLINEMODE -DKLUDGELINEMODE -DENV_HACK -fPIC $(REST)
OBJ=authenc.o commands.o main.o network.o ring.o sys_bsd.o telnet.o terminal.o tn3270.o utilities.o
LDFLAGS=-Wl,--whole-archive ../libinetutils/libinetutils.a ../lib/libgnu.a ../libtelnet/libtelnet.a -ltermcap -lcrypt  -lreadline -lresolv -lnsl -Wl,--no-whole-archive

%.o: %.c
	$(CC) $(CFLAGS) $(DEPDIR)/$*.Tpo -c $<

libproject.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

test: test.cpp libproject.so
	$(CXX) $(FFLAGS) -o $@ $< $(INCLUDES) libproject.so -lutil $(FUZZER)
