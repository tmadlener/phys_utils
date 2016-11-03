#include "EventMixer.h"
#include "ToyMCEvent.h"
#include "ToyMCOutEvent.h"
#include "ToyMCMixFunction.h"

#include "MixerSettings.h" // in config

#include "TTree.h"
#include "TFile.h"

#include <iostream>

/** small main to test and demonstrate the EventMixer class. */
int main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Need the name of an input and of an output .root file" << std::endl;
    return 1;
  }

  TFile* file = TFile::Open(argv[1]);
  TTree* tree = static_cast<TTree*>(file->Get(config::InputTree.treeName.c_str()));

  EventMixer<ToyMCEvent, ToyMCOutEvent> eventMixer(tree, argv[2],
                                                   config::OutputTree.treeName);
  std::cout << "Event mixer initialized" << std::endl;
  std::cout << "Event mixer, starting event loops" << std::endl;

  // if there are enough command line parameters, take them as min and max values for the mass
  const double massMin = argc < 5 ? config::ToyMCMixConditions.massLow : std::atof(argv[3]);
  const double massMax = argc < 5 ? config::ToyMCMixConditions.massHigh : std::atof(argv[4]);

  std::cout << "Mass window in mixing: " << massMin << " < M [GeV] < " << massMax << std::endl;
  using namespace std::placeholders;
  eventMixer.mix(std::bind(ToyMCMixFunction, _1, _2, _3, _4, massMin, massMax), config::General.maxEvents);
                 // config::Logging.filename);

  eventMixer.writeToFile();

  return 0;
}
