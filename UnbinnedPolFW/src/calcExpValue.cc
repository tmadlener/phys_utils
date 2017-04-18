// file to test expectation value calculation

#include "Support.h"
#include "AngularDistribution.h"
#include "NormCalculation.h"

#include "general/vector_helper.h"

// ROOT
#include "TFile.h"
#include "TTree.h"

// stl
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

template<size_t N, size_t M, size_t P>
double calcLogL(const std::vector<double>& pT, const std::vector<double>& costh,
                const std::vector<double>& phi, const std::vector<double>& wS,
                const AngularParametrization<N,M,P>& A,
                const PartialExpValues<N,M,P>& pExpVals)
{
  double sum{};
  for (size_t i = 0; i < pT.size(); ++i) {
    sum += wS[i] * std::log(fCosThetaPhi(costh[i], phi[i], A, pT[i]));
  }

  const double norm = calcExpFcosthphi(A, pExpVals);
  sum -= std::log(norm) * pT.size();

  std::cout << A.getValAL() << "| " << A.getValAphi() << "| "
            << A.getValAtp() << " || " << sum <<"\n";

  return sum;
}

std::array<std::vector<double>,4> readFromTTree(TTree* t, const std::string& pt,
                                                const std::string& costh, const std::string& phi,
                                                const std::string& wS)
{
  double p;
  double c;
  double ph;
  double w;

  const int nEntries = t->GetEntries();
  std::vector<double> ptVals;
  ptVals.reserve(nEntries);
  std::vector<double> costhVals;
  costhVals.reserve(nEntries);
  std::vector<double> phiVals;
  phiVals.reserve(nEntries);
  std::vector<double> wVals;
  wVals.reserve(nEntries);

  t->SetBranchAddress(pt.c_str(), &p);
  t->SetBranchAddress(costh.c_str(), &c);
  t->SetBranchAddress(phi.c_str(), &ph);
  t->SetBranchAddress(wS.c_str(), &w);

  for (int i = 0; i < nEntries; ++i) {
    t->GetEntry(i);
    ptVals.push_back(p);
    costhVals.push_back(c);
    phiVals.push_back(ph);
    wVals.push_back(w);
  }

  return std::array<std::vector<double>, 4>{ptVals, costhVals, phiVals, wVals};
}

#ifndef __CINT__
int main(int, char**)
{
  // Support<3,2,1> support("pT", {10, 25, 50}, {10, 50}, {10});
  Support<1,0,0> support("pT", {10}, {}, {});
  BranchNames branchNames{"wS", "costh_HX", "phi_HX"};

  TFile* refF = TFile::Open("/afs/hephy.at/user/t/tmadlener/macros_Pietro_new/genReference.root");
  TTree* refT = static_cast<TTree*>(refF->Get("genData"));

  support.Init(refT, branchNames);
  support.calcSupportExpVals(refT);

  const auto partialExpValues = support.getPartialExpValues();

  delete refT;
  refF->Close();

  // get an easier to use parametrization for the looping
  auto angParams = support.getAngParams();

  const auto rangeAL = linspace(0.3, 0.4, 7);
  const auto rangeAphi = linspace(-0.05, 0.05, 3);

  TFile* f = TFile::Open("/afs/hephy.at/user/t/tmadlener/macros_Pietro_new/genData.root");
  TTree* t = static_cast<TTree*>(f->Get("genData"));

  const auto fileVals = readFromTTree(t, "pT", "costh_HX", "phi_HX", "wS");
  const auto& pTVals = fileVals[0];
  const auto& costhVals = fileVals[1];
  const auto& phiVals = fileVals[2];
  const auto& wVals = fileVals[3];

  delete t;
  f->Close();

  TFile* fout = new TFile("testLoop.root", "recreate");
  TTree* tout = new TTree("llValues", "log likelihood values and parameters");

  // std::array<double, 3> AL;
  // std::array<double, 2> Aphi;
  // std::array<double, 1> Atp;
  // Atp[0] = 0;

  double AL{};
  double logL{};

  tout->Branch("AL", &AL);
  // tout->Branch("Aphi", &Aphi);
  // tout->Branch("Atp", &Atp);
  tout->Branch("logL", &logL);


  unsigned ctr{};
  std::vector<double> ALvals = linspace(0.0, 1.0, 1001);
  for (const auto& v : ALvals) {
    angParams.setVals({v}, {}, {});
    AL = v;
    logL = calcLogL(pTVals, costhVals, phiVals, wVals, angParams, partialExpValues);
    ctr++;
  }

  // for (const auto& AL0 : rangeAL) {
  //   for (const auto& AL1 : rangeAL) {
  //     for (const auto& AL2 : rangeAL) {
  //       for (const auto& Aphi0 : rangeAphi) {
  //         for (const auto& Aphi1 : rangeAphi) {
  //           angParams.setVals({AL0, AL1, AL2}, {Aphi0, Aphi1}, {Atp[0]});
  //           AL = angParams.getValAL();
  //           Aphi = angParams.getValAphi();
  //           Atp = angParams.getValAtp(); // redundant, but this is a prototype after all
  //           logL = calcLogL(pTVals, costhVals, phiVals, wVals, angParams, partialExpValues);
  //           ctr++;
  //           tout->Fill();
  //         }
  //       }
  //     }
  //   }
  // }

  std::cout << "ctr = " << ctr << std::endl;

  tout->Write();
  fout->Close();

  return 0;
}

#endif
