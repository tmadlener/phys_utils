#ifndef EVENTMIXER_TOYMCOUTEVENT_H__
#define EVENTMIXER_TOYMCOUTEVENT_H__

#include "MiscHelper.h"
#include "../config/MixerSettings.h"

#include "TLorentzVector.h"
#include "TTree.h"

#include <algorithm>

/**
 * Output Event for ToyMC samples.
 * Stores the 4-vectors of the two single muons as well as the resulting dimuon, along with the event numbers
 * from which the two single muons were taken in the original TTree.
 */
class ToyMCOutEvent {
public:

  ToyMCOutEvent() = default;

  /** constructor from all the necessary information. */
  ToyMCOutEvent(const TLorentzVector& muPos, const TLorentzVector& muNeg, const TLorentzVector& dimuon,
                unsigned i, unsigned j, unsigned flags = 0);

  /** copy-constructor for deep copying the internal pointers. */
  ToyMCOutEvent(const ToyMCOutEvent& other);

  /** assignment operator, using the 'copy and swap' idiom. */
  ToyMCOutEvent& operator=(ToyMCOutEvent other);

  /** destructor, deleting the internal pointers. */
  ~ToyMCOutEvent();

  /** swap for implementing the 'copy and swap' idiom. */
  void swap(ToyMCOutEvent other);

  void Init(TTree* tree);

  TLorentzVector& muPos() { return *m_muPos; }

  TLorentzVector& muNeg() { return *m_muNeg; }

  TLorentzVector& dimuon() { return *m_dimuon; }

  void setPosEv(const size_t i) { m_posEvent = i; }

  void setNegEv(const size_t i) { m_negEvent = i; }

private:

  TLorentzVector* m_muPos{nullptr};

  TLorentzVector* m_muNeg{nullptr};

  TLorentzVector* m_dimuon{nullptr};

  unsigned m_posEvent{};

  unsigned m_negEvent{};

  unsigned m_flags{}; /**< storing additional information. */

};

ToyMCOutEvent::ToyMCOutEvent(const TLorentzVector& muPos, const TLorentzVector& muNeg, const TLorentzVector& dimuon,
                             unsigned i, unsigned j, unsigned flags)
  : m_muPos(clone(muPos)), m_muNeg(clone(muNeg)), m_dimuon(clone(dimuon)), m_posEvent(i), m_negEvent(j), m_flags(flags)
{
  // No-op
}

ToyMCOutEvent::ToyMCOutEvent(const ToyMCOutEvent& other)
  : m_muPos(clone(other.m_muPos)), m_muNeg(clone(other.m_muNeg)), m_dimuon(clone(other.m_dimuon)),
    m_posEvent(other.m_posEvent), m_negEvent(other.m_negEvent), m_flags(other.m_flags)
{
  // Nothing to do here
}

void ToyMCOutEvent::swap(ToyMCOutEvent other)
{
  std::swap(m_muPos, other.m_muPos);
  std::swap(m_muNeg, other.m_muNeg);
  std::swap(m_dimuon, other.m_dimuon);

  std::swap(m_posEvent, other.m_posEvent);
  std::swap(m_negEvent, other.m_negEvent);
  std::swap(m_flags, other.m_flags);
}

ToyMCOutEvent& ToyMCOutEvent::operator=(ToyMCOutEvent other)

{
  this->swap(other);
  return *this;
}

ToyMCOutEvent::~ToyMCOutEvent()
{
  delete m_muPos;
  delete m_muNeg;
  delete m_dimuon;
}

void ToyMCOutEvent::Init(TTree* tree)
{
  tree->Branch(config::OutputTree.muPosName.c_str(), &m_muPos);
  tree->Branch(config::OutputTree.muNegName.c_str(), &m_muNeg);
  tree->Branch(config::OutputTree.diMuName.c_str(), &m_dimuon);
  tree->Branch("posEventNo", &m_posEvent);
  tree->Branch("negEventNo", &m_negEvent);
  // tree->Branch("flags", &m_flags);
}

#endif
