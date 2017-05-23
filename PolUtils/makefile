# check if we are on a heplx and set the path accordingly if so
# this allows me to compile remotely from inside emacs using tramp
ifneq (,$(findstring oeaw.ac.at,$(HOSTNAME)))
export PATH=$(MAKE_PATH)
export LD_LIBRARY_PATH=$(MAKE_LD_LIBRARY_PATH)
GCC_BASE_DIR=$(MAKE_ROOT_GCC_TOOLCHAIN)/bin/
ROOT_CONFIG_BIN=$(MAKE_ROOT_CONFIG_BIN)
else
GCC_BASE_DIR=
ROOT_CONFIG_BIN=$(shell which root-config)
endif

ROOTLIBS=$(shell $(ROOT_CONFIG_BIN) --libs)
ROOTCXXFLAGS=$(shell $(ROOT_CONFIG_BIN) --cflags)

CXXFLAGS=-std=c++11 -Wall -Wextra -Wpedantic -Wshadow -O2 # -Wconversion # can't use conversion warnings, because there are a lot in ROOT
ROOFITLIBS=-lRooFit -lRooFitCore -lMinuit -lFoam

CXX=$(GCC_BASE_DIR)g++

INCS=-I./interface -I../ -I.

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
