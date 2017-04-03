#include "general/ArgParser.h"
#include "general/progress.h"

#include "root_utils.h"
#include "misc_utils.h"
#include "JpsiFromBEvent.h"

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TLorentzVector.h"
#include "TRotation.h"
#include "TVector3.h"

#include <cmath>
#include <string>
#include <vector>
#include <sstream>

const std::vector<double> massBinning{5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.0};

std::vector<TH2D*> createCosThHists(const int nMassBins, const std::string& frame)
{
  std::vector<TH2D*> hists;

  for (int i = 0; i < nMassBins; ++i) {
    std::stringstream hname;
    hname << "cosThPhi_" << frame << "_mass" << i;
    hists.push_back(new TH2D(hname.str().c_str(), "", 32, -1, 1, 32, -180, 180));
  }

  return hists;
}

std::vector<TH1D*> create1DHists(const int nMassBins, const std::string& base,
                                 const int nBins, const double min, const double max)
{
  std::vector<TH1D*> hists;
  for (int i = 0; i < nMassBins; ++i) {
    std::stringstream hname;
    hname << base << "_mass" << i;
    hists.push_back(new TH1D(hname.str().c_str(), "", nBins, min, max));
  }

  return hists;
}

template<typename HistT>
void writeToFile(TFile* f, std::vector<HistT*>& hists)
{
  f->cd();
  for (const auto* h : hists) h->Write();
}

enum class RefFrame {
  HX,
  PX,
  CS
};

struct Angles {
  double costh;
  double phi;
};

struct ReferenceAxis {
  TVector3 x;
  TVector3 y;
  TVector3 z;
};


/** calculate the angles of the lepInDilepFrame where the frame is defined by the passed axes. */
Angles calcAngles(const ReferenceAxis& refAxis,
                  const TLorentzVector& lepInDilepFrame)
{
  constexpr double overpi = 1 / M_PI;

  TRotation rotation;
  rotation.RotateAxes(refAxis.x, refAxis.y, refAxis.z);
  rotation.Invert(); // now transformin from "xyz" frame to the "new" frame

  auto lepInDilepRotated = lepInDilepFrame.Vect();
  lepInDilepRotated.Transform(rotation);

  const double cosTh = lepInDilepRotated.CosTheta();
  const double phi = lepInDilepRotated.Phi() * 180 * overpi;

  return Angles{cosTh, phi};
}

ReferenceAxis determineRefFrameAxis(TVector3 b1, TVector3 b2, TVector3 oniaLab,
                                    const double oniaRap, const RefFrame refFrame)
{
  // y axis is the same in all frames
  const TVector3 yAxis = (b1.Cross(b2)).Unit() * (oniaRap < 0 ? -1 : 1); // rap depndent change of sign
  TVector3 zAxis;
  TVector3 xAxis;
  switch (refFrame) {
  case RefFrame::CS:
    zAxis = (b1 - b2).Unit(); // bisector of beams is z-axis
    break;
  case RefFrame::HX:
    zAxis = oniaLab;
    break;
  case RefFrame::PX:
    zAxis = (b1 - b2).Cross(yAxis).Unit(); // perpendicular to CS frame
    break;
  }

  xAxis = yAxis.Cross(zAxis);
  return ReferenceAxis{xAxis, yAxis, zAxis};
}

Angles calcAnglesInFrame(TLorentzVector muMinus, TLorentzVector muPlus,
                         const RefFrame refFrame)
{
  constexpr double pbeam = 6500; // GeV
  constexpr double Mprot = 0.9382720; // GeV
  constexpr double Ebeam = std::sqrt(pbeam*pbeam + Mprot*Mprot);
  TLorentzVector beam1(0, 0, pbeam, Ebeam);
  TLorentzVector beam2(0, 0, -pbeam, Ebeam);

  auto onia = muMinus + muPlus;
  const double oniaRap = onia.Rapidity();

  auto boostVecLabToOnia = -onia.BoostVector();
  beam1.Boost(boostVecLabToOnia);
  beam2.Boost(boostVecLabToOnia);
  muPlus.Boost(boostVecLabToOnia);

  auto beam1Dir = beam1.Vect().Unit();
  auto beam2Dir = beam2.Vect().Unit();
  auto oniaDir = onia.Vect().Unit();

  auto refFrameAxis = determineRefFrameAxis(beam1Dir, beam2Dir, oniaDir, oniaRap, refFrame);
  return calcAngles(refFrameAxis, muPlus);
}


#ifndef __CINT__
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto ifn = parser.getOptionVal<std::string>("--inputfile");
  const auto ofn = parser.getOptionVal<std::string>("--outputfile", "bMassSideBands_pol.root");

  TTree* tin = createTChain(ifn, "selectedData");

  TFile* fout = new TFile(ofn.c_str(), "recreate");
  JpsiFromBEvent event;
  event.Init(tin);

  auto cosThHistsCS = createCosThHists(massBinning.size() - 1, "CS");
  auto cosThHistsHX = createCosThHists(massBinning.size() - 1, "HX");
  auto cosThHistsPX = createCosThHists(massBinning.size() - 1, "PX");

  const int nEvents = tin->GetEntries();
  const auto startTime = ProgressClock::now();
  for (int i = 0; i < nEvents; ++i) {
    if (!checkGetEntry(tin, i)) continue;
    const int massBin = getBin(event.bPlus().M(), massBinning);
    if (massBin < 0) continue;
    // std::cout << "i = " << i << ", event.bPlus().M() = " << event.bPlus().M() << ", massBin = " << massBin << std::endl;
    const auto anglesCS = calcAnglesInFrame(event.muNeg(), event.muPos(), RefFrame::CS);
    cosThHistsCS[massBin]->Fill(anglesCS.costh, anglesCS.phi);
    const auto anglesPX = calcAnglesInFrame(event.muNeg(), event.muPos(), RefFrame::PX);
    cosThHistsPX[massBin]->Fill(anglesPX.costh, anglesPX.phi);
    const auto anglesHX = calcAnglesInFrame(event.muNeg(), event.muPos(), RefFrame::HX);
    cosThHistsHX[massBin]->Fill(anglesHX.costh, anglesHX.phi);

    printProgress(i, nEvents, startTime, 5);
  }


  writeToFile(fout, cosThHistsCS);
  writeToFile(fout, cosThHistsHX);
  writeToFile(fout, cosThHistsPX);

  fout->Close();

  return 0;
}

#endif
