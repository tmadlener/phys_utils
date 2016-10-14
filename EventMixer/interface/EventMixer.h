#ifndef EVENTMIXER_EVENTMIXER_H__
#define EVENTMIXER_EVENTMIXER_H__

#include "MiscHelper.h"
#include "general/progress.h"

#include "TTree.h"
#include "TLorentzVector.h"
#include "TFile.h"

#include <iostream>
#include <string>
#include <fstream>

/**
 * EventMixer class to mix different Events from one TTree.
 * Loops over all possible event combinations.
 * When and how two events are mixed is steered by the function passed to the mix method of this class.
 * See the documentation of mix to find out which interface criteria this function has to meet.
 *
 * The template parameters EventT and OutEventT are the types of the internally used two types of events.
 * The EventT class is used for handling the input, while the OutEventT is used for dealing with the output.
 * Both classes have to provide a ::Init() function taking a std::unique_ptr<TTree>& in case of EventT and a TTree*
 * in case of OutEventT (difference mainly due to ROOT reasons at the moment).
 * The Init() function has to do all necessary initialization (Setting branch addresses, etc.)
 *
 * Other than that their interfaces are dictated by the the needs of the used cond function in ::mix().
 *
 * Note takes ownership of the passed TTree!.
 */
template<typename EventT, typename OutEventT>
class EventMixer {
public:
  EventMixer() = delete; /**< don't want a default constructor. */

  /**
   * Constructor taking the input TTree* and the names for the output file and the TTree therein.
   * Initializes the internal Event classes and creates the output file and TTree.
   */
  EventMixer(TTree* inTree, const std::string& outFileName, const std::string& outTreeName);

  /**
   * Loop over all possible event combinations exactly once and do conditional event mixing.
   * The looping is done in a way that events i & j are combined once (i.e. i & j, but not j & i), because a second
   * combination would be redundant. Also a mixing only occurs if i & j are different.
   *
   * The main work of this function is done by the cond function (operator) it's interface has to be equivalent to:
   *
   * \code{.cpp}
   * std::vector<OutEventT> cond(const EventT& e1, const EventT& e2, size_t i, size_t j);
   * \endcode
   * The function has to return a non-empty vector if the content that is stored in outev should be written to the output.
   * The events e1 and e2 are checked for compatibility and the vector has to be filled inside the function.
   * The two indices i & j are the indices of the e1 (e2 resp.) in the input TTree.
   *
   * The number of events to be processed can be controlled via the maxEvents variable. Setting it to a negative
   * value results in taking all events present in the input TTree.
   *
   * If a non-empty string is passed to the logfile parameter the progress will be printed to a file with this name
   * instead of stdout.
   */
  template<typename CondF>
  void mix(CondF cond, const long int maxEvents = -1, const std::string& logfile = "");

  /** write the output TTree to the output file and close the file. */
  void writeToFile();

private:
  EventT m_event1; /**< Event associated to the i index in the mix loop and the m_inTree. */

  EventT m_event2; /**< Event associated to the j index in the mix loop and the m_cloneInTree. */

  OutEventT m_outEvent; /**< Output Event container, that get's written to the output file*/

  std::unique_ptr<TTree> m_inTree; /**< The input TTree. */

  /** Need a clone of the input tree in order to be able to access two different events simulatenously. */
  std::unique_ptr<TTree> m_cloneInTree;

  TTree* m_outTree{nullptr}; /**< Output TTree. for ROOT reasons not a std::unique_ptr. */

  TFile* m_outFile{nullptr}; /**< Output TFile. for ROOT reasons not a std::unique_ptr. */

};

template<typename EventT, typename OutEventT>
EventMixer<EventT, OutEventT>::EventMixer(TTree* inTree, const std::string& outFileName,
                                          const std::string& outTreeName)
  : m_inTree(inTree), m_cloneInTree(clone(inTree))
{
  m_event1.Init(m_inTree);
  m_event2.Init(m_cloneInTree);

  // init the output tree and file
  m_outFile = new TFile(outFileName.c_str(), "recreate");
  m_outTree = new TTree(outTreeName.c_str(), "mixed events tree");

  m_outEvent.Init(m_outTree);
}

template<typename EventT, typename OutEventT>
template<typename CondF>
void EventMixer<EventT, OutEventT>::mix(CondF cond, const long int maxEvents, const std::string& logfile)
{
  size_t mixed{};
  size_t trials{};

  // check first how many events we want to process and correct for a possible input error, where more events
  // then present are requested
  const long int nInputEvents = m_inTree->GetEntries();
  const size_t nEvents = (maxEvents < 0 || maxEvents > nInputEvents) ? nInputEvents : maxEvents;

  const size_t nCombinations = 0.5 * nEvents * (nEvents - 1); // we now the number of (input) combinations to check

  // open a filestream only if the progress output should be redirected to a file, otherwise use stdout
  std::ofstream filestream;
  if (!logfile.empty()) filestream.open(logfile);
  std::ostream& logstream = logfile.empty() ? std::cout : filestream;

  std::cout << "Starting mixing of " << nEvents << " events. Possible (input) combinations: " << nCombinations << std::endl;
  auto startTime = std::chrono::high_resolution_clock::now(); // start the clock

  for (size_t i = 0; i < nEvents; ++i) {
    m_inTree->GetEntry(i);
    // second loop starts one event after the first loop! -> No mixing of the same event, and no double checking of events
    for (size_t j = i + 1; j < nEvents; ++j) {
      trials++;
      m_cloneInTree->GetEntry(j);

      for (const auto& event : cond(m_event1, m_event2, i, j)) {
        mixed++;
        m_outEvent = event;
        m_outTree->Fill();
      }
      printProgress(trials, nCombinations, startTime, 2500, logstream); // put here to ensure that trials != 0 for all calls
    }
  }

  std::cout << "created " << mixed << " new events from " << trials << " possible (input) combinations." << std::endl;
  if (!logfile.empty()) filestream.close(); // cannot close an unopened fstream
}

template<typename EventT, typename OutEventT>
void EventMixer<EventT, OutEventT>::writeToFile()
{
  m_outFile->cd();
  m_outTree->Write();
  m_outFile->Write();
  m_outFile->Close();
}

#endif
