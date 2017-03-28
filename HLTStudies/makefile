CXXFLAGS=$(shell root-config --cflags) -Wall -Wextra -O2
LDFLAGS=$(shell root-config --ldflags) -fPIC
ROOTINC=-I$(shell root-config --incdir)
ROOTLIBS=$(shell root-config --libs)
TUPLERDIR=$(CMSSW_BASE)/src/NtupleTools/ntupleForL1


all: RateExtrapolation ntupleTreeDict.cxx libNtupleTree.so L1eff

RateExtrapolation: RateExtrapolation.cc
	g++ -I../ $(shell root-config --libs --cflags) -Wall -Wextra -Wshadow -O2 $^ -o $@

ntupleTreeDict.cxx: $(TUPLERDIR)/src/ntupleTree.h $(TUPLERDIR)/src/ntupleTreeLinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^

libNtupleTree.so: ntupleTreeDict.cxx $(TUPLERDIR)/src/ntupleTree.h
	g++ -shared -o$@ $(LDFLAGS) $(CXXFLAGS) $(ROOTINC) $^

L1eff: L1efficiencies.cc libNtupleTree.so L1Seeds.h
	g++ $(CXXFLAGS) $(ROOTLIBS) -o $@ L1efficiencies.cc $(ROOTINC) -I$$HOME/phys_utils -I$(TUPLERDIR)/src libNtupleTree.so
