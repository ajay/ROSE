CXX = g++
CFLAGS = -std=c++11 -g `pkg-config --cflags opencv`
LIBS = -larmadillo -lSDL2 `pkg-config --libs opencv` -pthread
ACTIONS = $(shell pwd)/../actions
INCLUDE = -I$(ACTIONS)
OBJS = astar.o maze_imgio.o sim_window.o draw.o sim.o
SUBOBJS = $(ACTIONS)/actions.o

all: subdirs $(OBJS) sim

subdirs:
	make -C $(ACTIONS)

sim: $(OBJS) $(SUBOBJS)
	$(CXX) $(INCLUDE) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(INCLUDE) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o sim
	make clean -C $(ACTIONS)
