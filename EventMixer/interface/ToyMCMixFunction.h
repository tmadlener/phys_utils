#ifndef EVENTMIXER_TOYMCMIXER_H__
#define EVENTMIXER_TOYMCMIXER_H__

#include "ToyMCEvent.h"
#include "ToyMCOutEvent.h"

#include "../config/MixerSettings.h"

#include "TLorentzVector.h"

/**
 * Mixing function for Toy MC events.
 * Checks if any combination of pos + neg muon, where each muon is from a different event results in a dimuon
 * with a suitable mass (defined in MixerSettings.h). If that is the case they are combined into a "new" event
 * and that is appended to the output vector.
 */
std::vector<ToyMCOutEvent> ToyMCMixFunction(const ToyMCEvent& evi, const ToyMCEvent& evj, size_t i, size_t j)
{
  TLorentzVector posI_negJ = evi.muPos() + evj.muNeg();
  TLorentzVector posJ_negI = evi.muNeg() + evj.muPos();

  const double pInJ_mass = posI_negJ.M();
  const double pJnI_mass = posJ_negI.M();

  // from config, for less typing
  const double massLow = config::ToyMCMixConditions.massLow;
  const double massHigh = config::ToyMCMixConditions.massHigh;

  std::vector<ToyMCOutEvent> events;
  events.reserve(2); // at max two events will be here

  if (pInJ_mass > massLow && pInJ_mass < massHigh) {
    events.push_back(ToyMCOutEvent(evi.muPos(), evj.muNeg(), posI_negJ, i, j));
  }
  if (pJnI_mass > massLow && pJnI_mass < massHigh) {
    events.push_back(ToyMCOutEvent(evj.muPos(), evi.muNeg(), posJ_negI, j, i));
  }

  return events;
}

#endif
