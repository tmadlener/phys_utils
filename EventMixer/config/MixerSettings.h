// this file serves as a single place to store different "settings" for the EventMixer classes
// All structs are global, but this is one of the most easily implementable solutions in c++ that does not require
// parsing (probably)
// The only real drawback is that a recompilation is necessary for every change here

#ifndef EVENTMIXER_CONFIG_H__
#define EVENTMIXER_CONFIG_H__

#include <string>

namespace config {

  struct {
    const std::string treeName = "genData"; /**< Name of the TTree in the input file. */
    const std::string muPosName = "lepP"; /**< Branch name of positive muon. */
    const std::string muNegName = "lepN"; /**< Branch name of negative muon. */
  } InputTree; /**< Settings for input TTrees. */

  struct {
    const std::string treeName = "genData"; /**< Name of the TTree in the output file. */
    const std::string muPosName = "lepP"; /**< Branch name for positive muon. */
    const std::string muNegName = "lepN"; /**< Branch name for negative muon. */
    const std::string diMuName = "chic"; /**< Branch name for the dimuon (generated from the positive and negative muon). */
  } OutputTree; /**< Settings for the output TTrees. */

  struct {
    /** number of input events to use. Negative values -> use all of input. */
    const long int maxEvents = -1;
  } General; /**< General settings*/

  struct {
    const double massLow = 2.0; /**< lower bound of mass range in GeV.*/
    const double massHigh = 4.0; /**< upper bound of mass range in GeV.*/
  } ToyMCMixConditions; /**< Settings for when to mix Toy MC events. */

  struct {
    const std::string filename = "/afs/hephy.at/work/t/tmadlener/ChiPol/ToyMC/EventMixing/progress_3.5GeV.out"; /**< logfile name. Set to empty string for output to stdout*/
  } Logging;
}

#endif
