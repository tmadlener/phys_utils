#ifndef PHYSUTILS_POLUTILS_JPSIFROMBROOTUPLING_H__
#define PHYSUTILS_POLUTILS_JPSIFROMBROOTUPLING_H__

#include "calcAngles.h"

// forward declarations
class JpsiFromBInputEvent;
class BRootupleEvent;

/**
 * very simple "preselection" method, for storing all values of interest in separate
 * branches for easier access.
 */
bool jpsiFromBRootupling(const JpsiFromBInputEvent& inEvent, BRootupleEvent& event)
{
  event.JpsiPt = inEvent.jpsi().Pt();
  event.JpsiRap = inEvent.jpsi().Rapidity();
  event.JpsiMass = inEvent.jpsi().M();
  event.JpsiCtauSig = inEvent.lxyToSigma;

  event.MuPPt = inEvent.muPos().Pt();
  event.MuPEta = inEvent.muPos().Eta();
  event.MuPPhi = inEvent.muPos().Phi();
  event.MuNPt = inEvent.muNeg().Pt();
  event.MuNEta = inEvent.muNeg().Eta();
  event.MuNPhi = inEvent.muNeg().Phi();

  const auto anglesHX = calcAnglesInFrame(inEvent.muPos(), inEvent.muNeg(),
                                          RefFrame::HX);
  event.cosTh_HX = anglesHX.costh;
  event.phi_HX = anglesHX.phi;

  const auto anglesPX = calcAnglesInFrame(inEvent.muPos(), inEvent.muNeg(),
                                          RefFrame::PX);
  event.cosTh_PX = anglesPX.costh;
  event.phi_PX = anglesPX.phi;

  const auto anglesCS = calcAnglesInFrame(inEvent.muPos(), inEvent.muNeg(),
                                          RefFrame::CS);
  event.cosTh_CS = anglesCS.costh;
  event.phi_CS = anglesCS.phi;

  event.bMass = inEvent.bPlus().M();
  event.bPt = inEvent.bPlus().Pt();
  event.bRap = inEvent.bPlus().Rapidity();

  event.trackPt = inEvent.track().Pt();

  event.event = inEvent.eventNb;
  event.run = inEvent.runNb;
  event.lumi = inEvent.lumiBlock;

  return true; // always return true, since we want to store all events
}

#endif
