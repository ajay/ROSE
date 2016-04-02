CC = g++
CFLAGS = -pedantic -Wall --std=c++11 -O3
INCLUDE = -I$(shell pwd)/../core \
          -I$(shell pwd)/../visual \
          -I$(shell pwd)/../speech \
          -I$(shell pwd)/../robot
SUBDIRS = $(shell pwd)/../visual \
          $(shell pwd)/../speech \
          $(shell pwd)/../robot
OBJECTS = agent.o
LIBOBJS = $(shell pwd)/../visual/visual.o \
          $(shell pwd)/../speech/speech.o \
          $(shell pwd)/../speech/text-to-speech/tts.o \
          $(shell pwd)/../speech/speech-to-text/stt.o
LIBS = `pkg-config --libs opencv` \
       -L$(shell pwd)/../robot -lrobot \
       -lFestival -lestools -lestbase -leststring
#       -larmadillo
TARGETS = libagent.a
TEST = test.o test

all: subdirs $(OBJECTS) $(TARGETS)

subdirs:
	$(foreach directory, $(SUBDIRS), make -C $(directory);)

agent.o: agent.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

libagent.a: $(OBJECTS) $(LIBOBJS)
	ar rvs $@ $^

test.o: test.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

test: test.o libagent.a
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJECTS) $(TARGETS) $(TEST)
	$(foreach directory, $(SUBDIRS), make clean -C $(directory);)
