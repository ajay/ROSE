CC			= gcc
CFLAGS		= -pedantic \
				-Wall \
				-Werror \
				-std=gnu99 \
				-O3 \
				-g \
				-o

CPPC		= g++
CPPFLAGS	= `pkg-config --cflags opencv` \
				`pkg-config --cflags libmongocxx` \
				`pkg-config --cflags sdl` \
				-pedantic \
				-Wno-unused-variable \
				-std=c++11 \
				-O3 \
				-g \
				-o
#				-Wall \
#				-Werror \

LIBS		= `pkg-config --libs opencv` \
				`pkg-config --libs libmongocxx` \
				`pkg-config --libs sdl` \
				-larmadillo \
				-lchilitags \
				-pthread
#				-lSDL2_ttf

COMPILEC	= $(CC) $(CFLAGS)
COMPILECPP	= $(CPPC) $(CPPFLAGS)

OBJECTS		= serial.o \
				actions.o \
				astar.o \
				chili_landmarks.o \
				draw.o \
				heap.o \
				highgui.o \
				mathfun.o \
				pfilter.o \
				Rose.o \
				runrobot.o \
				sim_landmark.o \
				sim_map.o \
				sim_robot.o

all: $(OBJECTS) runrobot

runrobot: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

%.o: %.c
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	$(COMPILECPP) $@ -c $<

clean:
	rm -rfv *.o runrobot