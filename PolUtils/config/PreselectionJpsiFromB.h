#ifndef PHYSUTILS_POLUTILS_JPSIFROMBPRESELECTION_CONFIG_H__
#define PHYSUTILS_POLUTILS_JPSIFROMBPRESELECTION_CONFIG_H__

#include "config/GeneralJpsiFromB.h"
#include "effsAndCuts.h"

#include <functional>

namespace config {

  const struct JpsiFromBPreSelectionSettings {
    const bool RejectCowboys = false; /**< reject cowboys? */ // for testing
    const bool RejectSeagulls = false;
    const double jpsiPtCut = 10.0; /**< cut in GeV/c */
    const double bPtCut = 0.0; /**< cut in GeV/c on the B meson pT. */
    const double vtxProbB = 0.01; /**< vertex prob. cut on B-vertex. */
    const double vtxProbJpsi = 0.01; /**< vertex prob cut on Jpsi vertex. */
  } JpsiFromBPS; /**< The used settings for the JPsi pre-selection*/
}

/**
 * To have only one definition that is used throughout, we bind a (spezialied) instantiation of the MuonAcceptance
 * functor to a std::function. Only this function will be called throughout all the framework.
 */
const std::function<bool(double, double)> isMuonInAcceptance = MuonAcceptance<config::Jpsi.FidCuts>();

#endif
