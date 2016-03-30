CC 			= gcc
CFLAGS 		= -pedantic \
				-Wall \
				-Werror \
				-std=gnu99 \
				-O3 \
				-g \
				-o

CPPC 		= g++
CPPFLAGS 	= `pkg-config --cflags sdl2` \
				`pkg-config --cflags libmongocxx` \
				-pedantic \
				-Wall \
				-Werror \
				-Wno-unused-variable \
				-std=c++11 \
				-O3 \
				-g \
				-o

LIBS 		= `pkg-config --libs sdl2` \
				`pkg-config --libs libmongocxx` \
 				-larmadillo \
 				-pthread \
 				-lSDL2_ttf

COMPILEC 	= $(CC) $(CFLAGS)
COMPILECPP 	= $(CPPC) $(CPPFLAGS)

OBJECTS 	= serial.o window.o Rose.o dbconn.o planner.o

all: $(OBJECTS) rose

rose: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

%.o: %.c
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	$(COMPILECPP) $@ -c $<

clean:
	rm -rfv *.o rose