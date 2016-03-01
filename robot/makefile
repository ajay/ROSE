CC = gcc
CFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -o
COMPILEC = $(CC) $(CFLAGS)

CPPC = g++
CPPFLAGS = -pedantic -Wall -std=c++11 -O3 -Wno-maybe-uninitialized -o
COMPILECPP = $(CPPC) $(CPPFLAGS)
LIBS = -larmadillo -lSDL2 -pthread

OBJECTS = Rose.o serial.o test.o

all: $(OBJECTS) test

serial.o: serial.c serial.h
	$(COMPILEC) $@ -c $<

Rose.o: Rose.cpp Rose.h
	$(COMPILECPP) $@ -c $<

test.o: test.cpp
	$(COMPILECPP) $@ -c $<

test: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

clean:
	rm -rfv *.o test
