CC = gcc
CFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -g -o
COMPILEC = $(CC) $(CFLAGS)

CPPC = g++
CPPFLAGS = `pkg-config --cflags sdl2` -pedantic -Wall -Werror -Wno-unused-variable -std=c++11 -O3 -g -o
COMPILECPP = $(CPPC) $(CPPFLAGS)
LIBS = `pkg-config --libs sdl2` -larmadillo -pthread -lSDL2_ttf

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
