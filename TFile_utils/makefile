# directory where the snippets regarding the more generalized stl snippets are stored
SNIPPET_DIR=$(HOME)/snippets
# root configuration including the libs, the include directory and the flags needed for compilation
ROOT_CONFIG=$(shell root-config --libs --cflags)
INC_DIRS=-I$(SNIPPET_DIR) -I../

CXX=g++
CXXFLAGS=-Wall

all: printRootFile saveCanvas

printRootFile: printRootFile.C
	$(CXX) $^ $(INC_DIRS) $(ROOT_CONFIG) $(CXXFLAGS) -o $@ -lRooFitCore

saveCanvas: saveCanvas.C
	$(CXX) $^ $(INC_DIRS) $(ROOT_CONFIG) $(CXXFLAGS) -o $@
