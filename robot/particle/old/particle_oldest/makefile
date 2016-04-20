CC = gcc
CFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -o
COMPILEC = $(CC) $(CFLAGS)

CPPC = g++
CPPFLAGS = `pkg-config --cflags opencv` -pedantic -Wall -std=c++11 -O3 -Wno-maybe-uninitialized -o
COMPILECPP = $(CPPC) $(CPPFLAGS)
LIBS = -larmadillo -lSDL -pthread -lSDL2 `pkg-config --libs opencv`

PFOBJS = sim_window.o sim_map.o sim_robot.o sim_lidar.o sim_landmark.o \
			 pfilter.o \
			 highgui.o draw.o sim.o

OBJECTS = Rose.o serial.o chilitags.o test.o planner $(PFOBJS)

all: $(OBJECTS) test

serial.o: serial.c serial.h
	$(COMPILEC) $@ -c $<

Rose.o: Rose.cpp Rose.h
	$(COMPILECPP) $@ -c $<

chilitags.o: chili_landmarks.cpp chili_landmarks.h
	$(COMPILECPP) $@ -c $<

%.o: %.cpp
	g++ $(CPPFLAGS) -o $@ -c $<

test.o: test.cpp
	$(COMPILECPP) $@ -c $<

test: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

planner.o: planner.cpp
	$(COMPILECPP) $@ -c $<

clean:
	rm -rfv *.o test