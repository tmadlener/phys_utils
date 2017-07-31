#ifndef PHYSUTILS_UNBINNEDPOLFW_CHICTUPLINGWITHWEIGHTS_H__
#define PHYSUTILS_UNBINNEDPOLFW_CHICTUPLINGWITHWEIGHTS_H__

#include "ChicInputEvent.h"
#include "ChicTupleEvent.h"

#include "../PolUtils/interface/calcAngles.h"

struct MassRegions;
struct LifeTimeRegions;
struct BkgWeights;

inline bool containedInMass(const double mass, const MassRegions &mr)
{
  // std::cout << mr.SR1 << " " << mr.SR2 << " " << mr.LSB << " " << mr.RSB << " " << mass << "\n";
  return mr.SR1.contains(mass) || mr.SR2.contains(mass) || mr.LSB.contains(mass) || mr.RSB.contains(mass);
}

inline bool containedInLT(const double ctau, const LifeTimeRegions& ltr)
{
  // std::cout << ltr.NP << " " << ltr.PR << " " << ctau << "\n";
  return ltr.NP.contains(ctau) || ltr.PR.contains(ctau);
}

bool chicTuplingWithWeights(const ChicInputEvent &inEvent, ChicTupleEvent &event,
                            const MassRegions &mr, const LifeTimeRegions &ltr,
                            const BkgWeights &weights)
{
  event.chicMass = inEvent.chic().M();
  event.Jpsict = inEvent.Jpsict;
  if (!(containedInMass(event.chicMass, mr) && containedInLT(event.Jpsict, ltr))) return false;

  event.chicPt = inEvent.chic().Pt();
  event.chicRap = inEvent.chic().Rapidity();

  const auto anglesHX = calcAnglesInFrame(inEvent.muN(), inEvent.muP(), RefFrame::HX);
  event.costh_HX = anglesHX.costh;
  event.phi_HX = anglesHX.phi;

  const auto anglesPX = calcAnglesInFrame(inEvent.muN(), inEvent.muP(), RefFrame::PX);
  event.costh_PX = anglesPX.costh;
  event.phi_PX = anglesPX.phi;

  const auto anglesCS = calcAnglesInFrame(inEvent.muN(), inEvent.muP(), RefFrame::CS);
  event.costh_CS = anglesCS.costh;
  event.phi_CS = anglesCS.phi;

  // std::cout << "====================\n";

  double wChic1 = !mr.SR2.contains(event.chicMass);
  double wChic2 = !mr.SR1.contains(event.chicMass);

  // std::cout << wChic1 << " " << wChic2 << "\n";

  if (mr.LSB.contains(event.chicMass) || mr.RSB.contains(event.chicMass)) {
    wChic1 *= weights.wMBkg1;
    wChic2 *= weights.wMBkg2;
  }
  // std::cout << wChic1 << " " << wChic2 << "\n";

  if (ltr.NP.contains(event.Jpsict)) {
    wChic1 *= weights.wNP;
    wChic2 *= weights.wNP;
  }
  // std::cout << wChic1 << " " << wChic2 << "\n";

  if (!(mr.SR1.contains(event.chicMass) && ltr.PR.contains(event.Jpsict))) wChic1 *= -1;
  if (!(mr.SR2.contains(event.chicMass) && ltr.PR.contains(event.Jpsict))) wChic2 *= -1;
  // std::cout << wChic1 << " " << wChic2 << "\n";


  event.wChic1 = wChic1;
  event.wChic2 = wChic2;

  // std::cout << "====================\n";


  return true;
}

#endif
