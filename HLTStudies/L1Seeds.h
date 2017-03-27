#ifndef PHYS_UTILS_HLTSTUDIES_L1SEED_H__
#define PHYS_UTILS_HLTSTUDIES_L1SEED_H__

#include <functional>
#include <unordered_map>
#include <string>

#include "general/misc_utils.h"

class L1MuonCand;

/** opposite charge */
inline bool OS(const L1MuonCand& m1, const L1MuonCand& m2)
{
  return (m1.charge * m2.charge) < 0;
}

/** double mu with mu-qlty double. */
inline bool DoubleMuX(const L1MuonCand& m1, const L1MuonCand& m2, const double X)
{
  return (m1.pt >= X && m2.pt >= X && inRange(m1.quality, 8, 15) && inRange(m2.quality, 8, 15));
}

/** double mu, eta restricted. */
inline bool DoubleMuXerY(const L1MuonCand& m1, const L1MuonCand& m2, const double X, const double Y)
{
  return DoubleMuX(m1,m2,X) && std::abs(m1.etaAtVtx) <= Y && std::abs(m2.etaAtVtx) <= Y;
}

/** delta Eta max. */
inline bool dEta_MaxX(const L1MuonCand& m1, const L1MuonCand& m2, const double X)
{
  return std::abs(m1.etaAtVtx - m2.etaAtVtx) < X;
}

/** deltaR cut on L1. */
inline bool dRXtoY(const L1MuonCand& m1, const L1MuonCand& m2, const double X, const double Y)
{
  return inRange(deltaR(m1.etaAtVtx, m2.etaAtVtx, m1.phiAtVtx, m2.phiAtVtx), X, Y);
}

/** invariant mass as calculated in the L1. */
inline double invL1Mass2(const L1MuonCand& m1, const L1MuonCand& m2)
{
  return 2 * m1.pt * m2.pt * (std::cosh(m1.etaAtVtx - m2.etaAtVtx) - cos(m1.phiAtVtx - m2.phiAtVtx));
}

/** invariant mass cut on L1. */
inline bool MASSXtoY(const L1MuonCand& m1, const L1MuonCand& m2, const double X, const double Y)
{
  return inRange(invL1Mass2(m1,m2), X*X, Y*Y);
}

// typedefs
using L1DoubleMuFunc = std::function<bool(const L1MuonCand&, const L1MuonCand&)>;
using L1SeedMap = std::unordered_map<std::string, L1DoubleMuFunc>;

/** create a map with functions for every L1 seed. */
L1SeedMap createL1SeedMap()
{
  L1SeedMap L1Seeds;
  L1Seeds["DoubleMu0"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuX(m1, m2, 0);
  };
  L1Seeds["DoubleMu0er1p2_OS_MASS0to10"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuXerY(m1, m2, 0, 1.202) && MASSXtoY(m1, m2, 0, 10);
  };
  L1Seeds["DoubleMu0er1p6_dEta_Max1p8"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuXerY(m1, m2, 0, 1.604) && dEta_MaxX(m1, m2, 1.8);
  };
  L1Seeds["DoubleMu0er1p6_dEta_Max1p8_OS"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuXerY(m1, m2, 0, 1.604) && dEta_MaxX(m1, m2, 1.8) && OS(m1, m2);
  };
  L1Seeds["DoubleMu0er1p4_dR0to1p8_OS"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuXerY(m1, m2, 0, 1.408) && dRXtoY(m1, m2, 0, 1.8);
  };
  L1Seeds["DoubleMu0er1p2_dEta_Max1p8_OS"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuXerY(m1, m2, 0, 1.202) && dEta_MaxX(m1, m2, 1.8) && OS(m1, m2);
  };
  L1Seeds["DoubleMu8"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuX(m1, m2, 8);
  };
  L1Seeds["DoubleMu7_OS"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuX(m1, m2, 7) && OS(m1, m2);
  };
  L1Seeds["DoubleMu6_OS_MASS0to10"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuX(m1, m2, 6) && OS(m1, m2) && MASSXtoY(m1, m2, 0, 10);
  };
  L1Seeds["DoubleMu6_OS_dR0to1"] = [](const L1MuonCand& m1, const L1MuonCand& m2) {
    return DoubleMuX(m1, m2, 6) && OS(m1, m2) && dRXtoY(m1, m2, 0, 1);
  };

  return L1Seeds;
}

#endif
