ROOTLIBS=$(shell root-config --libs)
# ROOTINCS=-I$(shell root-config --incdir) # already contained in --cflags
ROOTCXXFLAGS=$(shell root-config --cflags)
CXXFLAGS=-std=c++11 -Wall -Wextra -Wpedantic -Wshadow -O2 # -Wconversion # can't use conversion warnings, because there are a lot in ROOT
ROOFITLIBS=-lRooFit -lRooFitCore -lMinuit -lFoam
CXX=g++

INCS=-I./interface -I../ -I. -I$(HOME)/snippets

BIN=./bin/
SRC=./src/

SRCS=$(wildcard $(SRC)*.cc)

PROG=$(SRCS:$(SRC)%.cc=%)
LIST=$(addprefix $(BIN), $(PROG))

DEPS=$(LIST:%=%.d)

-include $(DEPS)

all: $(LIST)

$(BIN)%: $(SRC)%.cc
	$(CXX) $(INCS) $(ROOTINCS) $(CXXFLAGS) $(ROOTCXXFLAGS) -MMD -MP $< -o $@ $(ROOTLIBS) $(ROOFITLIBS)
