#ifndef PHYSUTILS_POLUTILS_JPSIFROMBPRESELECTION_H__
#define PHYSUTILS_POLUTILS_JPSIFROMBPRESELECTION_H__

#include "JpsiFromBInputEvent.h"
#include "JpsiFromBEvent.h"
#include "misc_utils.h"

#include "config/PreselectionJpsiFromB.h"
#include "config/GeneralJpsiFromB.h"

#include "TH1D.h"


/** return true if event should be filled. */
bool jpsiFromBPreSelection(const JpsiFromBInputEvent& inEvent, JpsiFromBEvent& event, TH1D* Reco_StatEv)
{
  if (inEvent.JpsiVprob < config::JpsiFromBPS.vtxProbJpsi) return false;
  if (inEvent.BvProb < config::JpsiFromBPS.vtxProbB) return false;
  const double jpsiPt = inEvent.jpsi().Pt();
  if(jpsiPt > 990.0) return false;

  if (inEvent.track().Pt() < config::JpsiFromBPS.trackPtCut) return false;

  if (inEvent.lxyToSigma < config::JpsiFromBPS.lifetimeSignificance) return false;

  // count all events
  Reco_StatEv->Fill(0.5);

  if (jpsiPt < config::JpsiFromBPS.jpsiPtCut) return false; // 10 GeV dimuon cut
  if (inEvent.bPlus().Pt() < config::JpsiFromBPS.bPtCut) return false;
  Reco_StatEv->Fill(1.5);

  const double jpsiAbsRap = std::abs(inEvent.jpsi().Rapidity());
  if (jpsiAbsRap > config::Jpsi.absRapMax) return false;

  Reco_StatEv->Fill(2.5);

  const double deltaPhi = reduceRange(inEvent.muNeg().Phi() - inEvent.muPos().Phi());
  if (config::JpsiFromBPS.RejectCowboys && deltaPhi < 0.) return false;
  if (config::JpsiFromBPS.RejectSeagulls && deltaPhi > 0.) return false;

  Reco_StatEv->Fill(3.5);

  const double jpsiMass = inEvent.jpsi().M();
  if (!inRange(jpsiMass, config::Jpsi.massMin, config::Jpsi.massMax)) return false;
  Reco_StatEv->Fill(4.5);

  const double etaMuPos = inEvent.muPos().Eta();
  const double pTmuPos = inEvent.muPos().Pt();
  const double etaMuNeg = inEvent.muNeg().Eta();
  const double pTmuNeg = inEvent.muNeg().Pt();

  if (!isMuonInAcceptance(pTmuPos, std::abs(etaMuPos)) || !isMuonInAcceptance(pTmuNeg, std::abs(etaMuNeg))) return false;

  Reco_StatEv->Fill(5.5);

  // Not here: (Left out for the moment)
  // * removeEta0p2_0p3
  // * cutDeltaREllDpt

  event = inEvent;
  return true;
}

#endif
