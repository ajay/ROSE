CFLAGS = `pkg-config --cflags opencv` `pkg-config --cflags sdl2` -std=c++11 -g -O3 -Wall -pedantic
LIBS = `pkg-config --libs opencv` `pkg-config --libs sdl2` -larmadillo -lSDL2_ttf
OBJS = main.o

all: $(OBJS) test

test: $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	g++ $(CFLAGS) -o $@ -c $<

clean:
	rm -rfv *.o test