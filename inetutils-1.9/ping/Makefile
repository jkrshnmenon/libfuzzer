CC = clang
CXX = clang++
REST=-MD -MP -MF
DEPDIR=.deps
FUZZER = ../../libFuzzer.a
FFLAGS = -g -fsanitize=address -fsanitize-coverage=bb
INCLUDES=-I. -I../ -I../lib -I../libinetutils -I../libicmp
CFLAGS = -DHAVE_CONFIG_H $(INCLUDES) -c -fPIC $(REST)
OBJ = ping.o ping_common.o ping_echo.o ping_address.o ping_router.o ping_timestamp.o libping.o
LDFLAGS = -Wl,--whole-archive ../libinetutils/libinetutils.a ../lib/libgnu.a ../libicmp/libicmp.a -lresolv -lnsl -Wl,--no-whole-archive

%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEPDIR)/$*.Tpo $<

libproject.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

test: test.cpp libproject.so
	$(CXX) $(FFLAGS) -o $@ $< $(INCLUDES) libproject.so -lutil $(FUZZER) 

