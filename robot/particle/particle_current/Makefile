CFLAGS = `pkg-config --cflags opencv` \
				 -std=c++11 -O3 \
				 -I/usr/local/include \
				 -I/usr/include
LIBS = -L/usr/local/lib \
			 `pkg-config --libs opencv` \
			 -larmadillo \
			 -lSDL2 \
			 -lchilitags \
			 -pthread

OBJS = sim_map.o sim_robot.o sim_landmark.o \
			 pfilter.o highgui.o draw.o sim.o \
			 chili_landmarks.o \
			 serial.o Rose.o \
			 astar.o heap.o actions.o sim_window.o


CC = gcc
CCFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -o
COMPILEC = $(CC) $(CCFLAGS)

all: $(OBJS) sim

sim: $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LIBS)

serial.o: serial.c serial.h
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	g++ $(CFLAGS) -o $@ -c $<

clean:
	rm -f sim *.o
