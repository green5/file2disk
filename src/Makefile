CXXFLAGS += -std=c++0x
CXXFLAGS += -Wall -Wno-unused-function -g

all: file2disk
	
file2disk: file2disk.cpp Makefile fs.h
	g++ $(CXXFLAGS) -o $@ $<

test: all
	./file2disk /media/green_/LINUX.IMG -A1

test2: all
	gdb.a ./file2disk /media/green/GRLDR

bcpp: file2disk.cpp fs.h
	bcpp.sh file2disk.cpp fs.h
	chown green file2disk.cpp fs.h	
