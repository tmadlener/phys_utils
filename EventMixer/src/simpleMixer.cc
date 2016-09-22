#include "EventMixer.h"
#include "ToyMCEvent.h"
#include "ToyMCOutEvent.h"
#include "ToyMCMixFunction.h"

#include "MixerSettings.h" // in config

#include "TTree.h"
#include "TFile.h"

#include <iostream>

/** small main to test and demonstrate the EventMixer class. */
int main(int, char**)
{
  TFile* file = TFile::Open(config::General.inputFileName.c_str());
  TTree* tree = static_cast<TTree*>(file->Get(config::InputTree.treeName.c_str()));

  EventMixer<ToyMCEvent, ToyMCOutEvent> eventMixer(tree, config::General.outputFileName.c_str(),
                                                   config::OutputTree.treeName.c_str());
  std::cout << "Event mixer initialized" << std::endl;
  std::cout << "Event mixer, starting event loops" << std::endl;

  using namespace std::placeholders;
  eventMixer.mix(std::bind(ToyMCMixFunction, _1, _2, _3, _4), config::General.maxEvents);

  eventMixer.writeToFile();

  return 0;
}
