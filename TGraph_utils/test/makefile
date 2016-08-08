INCDIR=../include

# library and include directory of googletest
GTEST_INCDIR=$(HOME)/googletest/googletest/include
GTEST_LIB=$(HOME)/googletest/libgtest.a

ROOT_CONFIG=$(shell root-config --libs --cflags)

CXX=g++
CXXFLGAS=-Wall

SRCFILES=$(wildcard *.cc)

all: $(SRCFILES:.cc=)

.cc:
	$(CXX) $^ -isystem $(GTEST_INCDIR) -I$(INCDIR) $(ROOT_CONFIG) $(CXXFLAGS) $(GTEST_LIB) -o $@
