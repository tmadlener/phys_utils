#ifndef EVENTMIXER_TOYMCEVENT_H__
#define EVENTMIXER_TOYMCEVENT_H__

#include "../config/MixerSettings.h"

#include "TTree.h"
#include "TLorentzVector.h"

#include <memory>

/**
 * Toy MC input event.
 * Provides only the two 4-vectors of the single muons.
 */
class ToyMCEvent{
public:

  ToyMCEvent() = default;

  void Init(std::unique_ptr<TTree>& tree);

  // void Print() { m_muNeg->Print(); m_muPos->Print(); }

  const TLorentzVector& muPos() const { return *m_muPos; }

  const TLorentzVector& muNeg() const { return *m_muNeg; }

private:

  TLorentzVector* m_muNeg{nullptr};

  TLorentzVector* m_muPos{nullptr};
};

void ToyMCEvent::Init(std::unique_ptr<TTree>& tree)
{
  tree->SetBranchAddress(config::InputTree.muNegName.c_str(), &m_muNeg);
  tree->SetBranchAddress(config::InputTree.muPosName.c_str(), &m_muPos);
}

#endif
