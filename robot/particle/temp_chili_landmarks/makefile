CPP = g++
CPPFLAGS = `pkg-config --cflags opencv` -std=c++11 -O3 -g -Wall
COMPILE = $(CPP) $(CPPFLAGS)
LIBS = `pkg-config --libs opencv` -lchilitags -lpthread
OBJS = chilitags.o

all: test

test: test.cpp $(OBJS)
	$(COMPILE) -o $@ $< $(OBJS) $(LIBS)

chilitags.o: chili_landmarks.cpp chili_landmarks.h
	$(COMPILE) -o $@ -c $<

clean:
	rm -rfv test *.o