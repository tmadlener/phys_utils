#ifndef PHYSUTILS_POLUTILS_JPSIFROMBINPUTEVENT_H__
#define PHYSUTILS_POLUTILS_JPSIFROMBINPUTEVENT_H__

#include "TTree.h"
#include "TLorentzVector.h"

#include "config/PreselectionJpsiFromB.h"

class JpsiFromBInputEvent {
public:
  JpsiFromBInputEvent() = default;

  void Init(TTree* tree);

  bool triggerDecision() const;

  const TLorentzVector& jpsi() const { return *m_jpsi; }
  const TLorentzVector& muPos() const { return *m_muPos; }
  const TLorentzVector& muNeg() const { return *m_muNeg; }
  const TLorentzVector& bPlus() const { return *m_bPlus; }

  int eventNb;
  int runNb;
  int lumiBlock;

  double Jpsict;
  double JpsictErr;
  double JpsiMassErr;
  double JpsiVprob;

  double BvProb;

private:

  TLorentzVector* m_jpsi{nullptr};

  TLorentzVector* m_muPos{nullptr};

  TLorentzVector* m_muNeg{nullptr};

  TLorentzVector* m_bPlus{nullptr};
};

void JpsiFromBInputEvent::Init(TTree* tree)
{
  tree->SetBranchAddress("JpsiP", &m_jpsi);
  tree->SetBranchAddress("muPosP", &m_muPos);
  tree->SetBranchAddress("muNegP", &m_muNeg);

  tree->SetBranchAddress("eventNb", &eventNb);
  tree->SetBranchAddress("runNb", &runNb);
  tree->SetBranchAddress("lumiBlock", &lumiBlock);

  tree->SetBranchAddress("Jpsict", &Jpsict);
  tree->SetBranchAddress("JpsictErr", &JpsictErr);
  tree->SetBranchAddress("JpsiMassErr", &JpsiMassErr);
  tree->SetBranchAddress("JpsiVprob", &JpsiVprob);

  tree->SetBranchAddress("BplusP", &m_bPlus);
  tree->SetBranchAddress("bVprob", &BvProb);
}

#endif
