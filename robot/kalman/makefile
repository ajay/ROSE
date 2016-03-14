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
				-pedantic \
				-Wall \
				-Werror \
				-Wno-unused-variable \
				-std=c++11 \
				-O3 \
				-g \
				-o

LIBS 		= `pkg-config --libs sdl2` \
 				-larmadillo \
 				-pthread \
 				-lSDL2_ttf

COMPILEC 	= $(CC) $(CFLAGS)
COMPILECPP 	= $(CPPC) $(CPPFLAGS)

OBJECTS 	= serial.o Rose.o test.o window.o

all: $(OBJECTS) rose

rose: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

%.o: %.c
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	$(COMPILECPP) $@ -c $<

clean:
	rm -rfv *.o rose