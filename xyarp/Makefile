CC=gcc
CXX=g++
LD=g++
SYSDIR=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
CPPFLAGS= -isysroot $(SYSDIR) -O2 -std=c++14 -I/opt/homebrew/include/ -arch arm64 -arch x86_64 -mmacosx-version-min=10.15
LDFLAGS= -L/opt/homebrew/lib/
LDLIBS="-lpcap -lnet"

ALL: xyarp

xyarp.o: xyarp.cpp
	$(CXX) -c -o $@ $^ $(CPPFLAGS)

main.o: main.cpp
	$(CXX) -c -o $@ $^ $(CPPFLAGS)

xyarp: main.o xyarp.o
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f xyarp.o main.o
	rm -f xyarp

install:
	cp xyarp /usr/local/bin/

archive:
	rm -f xyarp-src.zip
	gzip xyarp
	zip xyarp-src.zip *.cpp *.h Makefile

uninstall:
	rm -f /usr/local/bin/xyarp