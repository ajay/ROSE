CC 			= gcc
CFLAGS 		= -pedantic \
				-Wall \
				-Werror \
				-std=gnu99 \
				-O3 \
				-g \
				-o

CPPC 		= g++
CPPFLAGS 	= `pkg-config --cflags opencv` \
				-pedantic \
				-Wall \
				-Werror \
				-Wno-unused-variable \
				-std=c++11 \
				-O3 \
				-g \
				-o

LIBS 		= `pkg-config --libs opencv` \
 				-larmadillo \
 				-pthread

COMPILEC 	= $(CC) $(CFLAGS)
COMPILECPP 	= $(CPPC) $(CPPFLAGS)

OBJECTS 	= serial.o baserobot.o arm.o test.o

all: $(OBJECTS) arm

arm: $(OBJECTS)
	$(COMPILECPP) $@ $^ $(LIBS)

%.o: %.c
	$(COMPILEC) $@ -c $<

%.o: %.cpp
	$(COMPILECPP) $@ -c $<

clean:
	rm -rfv *.o arm