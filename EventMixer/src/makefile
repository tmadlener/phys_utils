ROOT_LIBS=$(shell root-config --libs)
ROOT_FLAGS=$(shell root-config --cflags)
CXX_FLAGS=-Wall -Wextra
CXX=g++
INCDIR=-I../interface -I../config

all: simpleMixer

simpleMixer: simpleMixer.cc
	$(CXX) $(CXX_FLAGS) $(ROOT_LIBS) $(ROOT_FLAGS) $(INCDIR) $^ -o $@
