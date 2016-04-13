CFLAGS = `pkg-config --cflags opencv` \
				 -O3 \
				 -I/usr/local/include \
				 -I/usr/include
LIBS = -L/usr/local/lib \
			 `pkg-config --libs opencv` \
			 -larmadillo \
			 -lSDL \
			 -lchilitags \
			 -pthread

OBJS = serial.o sim_map.o sim_robot.o sim_landmark.o \
			 pfilter.o highgui.o draw.o runrobot.o \
			 chili_landmarks.o \
			 Rose.o \
			 astar.o heap.o actions.o mathfun.o


CC = gcc
CCFLAGS = -pedantic -Wall -Werror -std=gnu99 -O3 -o
COMPILEC = $(CC) $(CCFLAGS)

all: $(OBJS) runrobot

runrobot: $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LIBS)

serial.o: serial.c serial.h
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	g++ $(CFLAGS) -std=c++11 -o $@ -c $<

%.o: %.c
	gcc $(CFLAGS) -std=gnu99 -o $@ -c $<

clean:
	rm -rfv *.o runrobot
