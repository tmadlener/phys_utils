#ifndef PHYSUTILS_POLUTILS_JPSIFROMBEVENT_H__
#define PHYSUTILS_POLUTILS_JPSIFROMBEVENT_H__

#include "JpsiFromBInputEvent.h"
#include "general/root_utils.h"

#include "TTree.h"
#include "TLorentzVector.h"

#include <algorithm> // std::swap
#include <iostream>

class JpsiFromBEvent {
public:
  JpsiFromBEvent() = default;

  /** ctor from input event */
  JpsiFromBEvent(const JpsiFromBInputEvent& inputEvent);

  /** copy */
  JpsiFromBEvent(const JpsiFromBEvent& other);

  // /** move ctor. */
  // JpsiFromBEvent(JpsiFromBEvent&& other) : JpsiFromBEvent() { swap(*this, other); }

  /** assignment. */
  JpsiFromBEvent& operator=(JpsiFromBEvent other);

  /** dtor. */
  ~JpsiFromBEvent();

  /** create the necessary branches in the TTree. */
  void Create(TTree* tree);

  /** set the branch addresses in the TTree. */
  void Init(TTree* tree);

  const TLorentzVector& jpsi() const { return *m_jpsi; }
  const TLorentzVector& muPos() const { return *m_muPos; }
  const TLorentzVector& muNeg() const { return *m_muNeg; }
  const TLorentzVector& bPlus() const { return *m_bPlus; }

  /** for copy-and-swap idiom. */
  friend void swap(JpsiFromBEvent& first, JpsiFromBEvent& second);

  double Jpsict;
  double JpsictErr;
  double runNb;
  double JpsiVprob;
  double JpsiMassErr;
  double BvProb;

private:

  TLorentzVector* m_jpsi{new TLorentzVector()};
  TLorentzVector* m_muPos{new TLorentzVector()};
  TLorentzVector* m_muNeg{new TLorentzVector()};
  TLorentzVector* m_bPlus{new TLorentzVector()};
};


JpsiFromBEvent::JpsiFromBEvent(const JpsiFromBInputEvent& inputEvent) :
  Jpsict(inputEvent.Jpsict), JpsictErr(inputEvent.JpsictErr), runNb(inputEvent.runNb),
  JpsiVprob(inputEvent.JpsiVprob), JpsiMassErr(inputEvent.JpsiMassErr), BvProb(inputEvent.BvProb),
  m_jpsi(clone(inputEvent.jpsi())), m_muPos(clone(inputEvent.muPos())), m_muNeg(clone(inputEvent.muNeg())),
  m_bPlus(clone(inputEvent.bPlus()))
{
  // No-op
}

JpsiFromBEvent::JpsiFromBEvent(const JpsiFromBEvent& other) :
  Jpsict(other.Jpsict), JpsictErr(other.JpsictErr), runNb(other.runNb), JpsiVprob(other.JpsiVprob), JpsiMassErr(other.JpsiMassErr),
  m_jpsi(clone(other.m_jpsi)), m_muPos(clone(other.m_muPos)), m_muNeg(clone(other.m_muNeg)),
  m_bPlus(clone(other.m_bPlus))
{
  // No-op
}

JpsiFromBEvent& JpsiFromBEvent::operator=(JpsiFromBEvent other)
{
  swap(*this, other);
  return *this;
}

JpsiFromBEvent::~JpsiFromBEvent()
{
  delete m_jpsi;
  delete m_muPos;
  delete m_muNeg;
  delete m_bPlus;
}

void JpsiFromBEvent::Create(TTree* tree)
{
  tree->Branch("JpsiP", &m_jpsi);
  tree->Branch("lepN", &m_muNeg);
  tree->Branch("lepP", &m_muPos);
  tree->Branch("BplusP", &m_bPlus);

  tree->Branch("Jpsict", &Jpsict);
  tree->Branch("JpsictErr", &JpsictErr);
  tree->Branch("JpsiMassErr", &JpsiMassErr);
  tree->Branch("JpsiVprob", &JpsiVprob);
  tree->Branch("runNb", &runNb);
  tree->Branch("bVprob", &BvProb);
}

void JpsiFromBEvent::Init(TTree* tree)
{
  tree->SetBranchAddress("JpsiP", &m_jpsi);
  tree->SetBranchAddress("lepN", &m_muNeg);
  tree->SetBranchAddress("lepP", &m_muPos);
  tree->SetBranchAddress("BplusP", &m_bPlus);

  tree->SetBranchAddress("Jpsict", &Jpsict);
  tree->SetBranchAddress("JpsictErr", &JpsictErr);
  tree->SetBranchAddress("JpsiMassErr", &JpsiMassErr);
  tree->SetBranchAddress("JpsiVprob", &JpsiVprob);
  tree->SetBranchAddress("runNb", &runNb);
  tree->SetBranchAddress("bVprob", &BvProb);
}

void swap(JpsiFromBEvent& first, JpsiFromBEvent& second)
{
  using std::swap;

  swap(first.Jpsict, second.Jpsict);
  swap(first.JpsictErr, second.JpsictErr);
  swap(first.JpsiMassErr, second.JpsiMassErr);
  swap(first.JpsiVprob, second.JpsiVprob);
  swap(first.runNb, second.runNb);
  swap(first.BvProb, second.BvProb);

  swap(first.m_jpsi, second.m_jpsi);
  swap(first.m_muPos, second.m_muPos);
  swap(first.m_muNeg, second.m_muNeg);
  swap(first.m_bPlus, second.m_bPlus);
}

#endif
