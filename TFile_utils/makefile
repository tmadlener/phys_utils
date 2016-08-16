# directory where the snippets regarding the more generalized stl snippets are stored
SNIPPET_DIR=$(HOME)/snippets
# root configuration including the libs, the include directory and the flags needed for compilation
ROOT_CONFIG=$(shell root-config --libs --cflags)

CXX=g++
CXXFLAGS=-Wall

all: printRootFile

printRootFile: printRootFile.C
	$(CXX) $^ -I$(SNIPPET_DIR) $(ROOT_CONFIG) $(CXXFLAGS) -o $@ -I../
