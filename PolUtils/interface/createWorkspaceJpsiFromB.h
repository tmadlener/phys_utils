#ifndef PHYSUTILS_POLUTILS_CREATEWORKSPACEJPSIFROMB_H__
#define PHYSUTILS_POLUTILS_CREATEWORKSPACEJPSIFROMB_H__

#include "JpsiFromBEvent.h"
#include "misc_utils.h"
#include "config/GeneralJpsiFromB.h"

#include "general/progress.h"

#include "TFile.h"
#include "TTree.h"

#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooArgList.h"
#include "RooDataSet.h"

#include <chrono>
#include <string>
#include <array>

RooDataSet* createFullDataSet(TTree* intree)
{
  using namespace RooFit;

  JpsiFromBEvent event;
  event.Init(intree);

  // define variables necessary for J/Psi(Psi(2S)) mass,lifetime fit
  RooRealVar* JpsiMass = new RooRealVar("JpsiMass", "M [GeV]", config::Jpsi.massMin, config::Jpsi.massMax);
  RooRealVar* JpsiRap = new RooRealVar("JpsiRap", "y", -config::Jpsi.absRapMax, config::Jpsi.absRapMax);
  RooRealVar* JpsiPt = new RooRealVar("JpsiPt", "p_{T} [GeV]", 0. ,100.);
  RooRealVar* Jpsict = new RooRealVar("Jpsict", "lifetime [mm]", -1., 2.5);
  RooRealVar* JpsictErr = new RooRealVar("JpsictErr", "Error on lifetime [mm]", 0.0001, 1);
  RooRealVar* JpsiVprob = new RooRealVar("JpsiVprob", "", 0.01, 1.);
  RooRealVar* JpsiP = new RooRealVar("JpsiP", "p [GeV]", 0.1, 120.); // calculated from max pT, y=1.2, m=3.1

  // B mass fit variables
  RooRealVar* bMass = new RooRealVar("Bmass", "M [GeV]", config::Bplus.massMin, config::Bplus.massMax);
  RooRealVar* bP = new RooRealVar("bP", "p [GeV]", 0.1, 120.0); // bound below to not make powerlaw fit go nuts

  // Set bins
  Jpsict->setBins(10000,"cache");
  Jpsict->setBins(100);
  JpsictErr->setBins(100);

  // The list of data variables
  RooArgList dataVars(*JpsiMass, *JpsiRap, *JpsiPt, *Jpsict, *JpsictErr, *JpsiVprob, *bMass, *bP, *JpsiP);
  // construct data set to contain events
  RooDataSet* fullData = new RooDataSet("fullData", "The Full Data from the Input ROOT Trees", dataVars);

  std::array<unsigned, 8> rejectionCtr{0,0,0,0,0,0,0,0};

  const int nEvents = intree->GetEntries();
  const auto startTime = ProgressClock::now();
  for (int i = 0; i < nEvents; ++i) {
    if (!checkGetEntry(intree, i)) continue;

    const double mJpsi = event.jpsi().M();
    const double mB = event.bPlus().M();
    const double pB = event.bPlus().P();
    const double jpsiPt = event.jpsi().Pt();
    const double jpsiRap = event.jpsi().Rapidity();

    if(!inRange(mJpsi, JpsiMass)) rejectionCtr[0]++;
    if(!inRange(pB, bP)) rejectionCtr[1]++;
    if(!inRange(jpsiPt, JpsiPt)) rejectionCtr[2]++;
    if(!inRange(jpsiRap, JpsiRap)) rejectionCtr[3]++;
    if(!inRange(mB, bMass)) rejectionCtr[4]++; // this one does the most damage
    if(!inRange(event.Jpsict, Jpsict)) rejectionCtr[5]++;
    if(!inRange(event.JpsictErr, JpsictErr)) rejectionCtr[6]++;
    if(!inRange(event.JpsiVprob, JpsiVprob)) rejectionCtr[7]++;

    if (inRange(mJpsi, JpsiMass) && inRange(pB, bP) &&
        inRange(jpsiPt, JpsiPt) && inRange(jpsiRap, JpsiRap) && inRange(mB, bMass) &&
        inRange(event.Jpsict, Jpsict) && inRange(event.JpsictErr, JpsictErr) &&
        inRange(event.JpsiVprob, JpsiVprob) && inRange(event.jpsi().P(), JpsiP)) {

      JpsiPt->setVal(jpsiPt);
      JpsiRap->setVal(jpsiRap);
      JpsiMass->setVal(mJpsi);
      JpsiVprob->setVal(event.JpsiVprob);
      bMass->setVal(mB);
      bP->setVal(pB);
      JpsiP->setVal(event.jpsi().P());

      if (config::Jpsi.correctCtau) {
        event.Jpsict *= (config::Jpsi.massPDG / mJpsi);
        event.JpsictErr *= (config::Jpsi.massPDG / mJpsi);
      }
      Jpsict->setVal(event.Jpsict);
      JpsictErr->setVal(event.JpsictErr);

      fullData->add(dataVars);
    }

    printProgress(i, nEvents, startTime, 10);
  }

  std::cout << "Full data set contains " << fullData->numEntries() << " events" << std::endl;

  for (const auto i : rejectionCtr) std::cout << i << std::endl;

  return fullData;
}

void splitAndStoreDataSets(RooDataSet* fullSet,
                           const std::vector<double>& ptBinning, const std::vector<double>& rapBinning,
                           const std::string& filenameBase)
{
  for (size_t iRap = 0; iRap < rapBinning.size(); ++iRap) {
    const auto rapCut = getCutString(iRap, rapBinning, "TMath::Abs(JpsiRap)");
    for (size_t iPt = 0; iPt < ptBinning.size(); ++iPt) {
      const auto ptCut = getCutString(iPt, ptBinning, "JpsiPt");

      std::string cutString = ptCut + " && " + rapCut;
      std::cout << "cutString: " << cutString << std::endl;

      RooDataSet* binData = static_cast<RooDataSet*>(fullSet->reduce(cutString.c_str()));
      std::cout << "bin contains " << binData->numEntries() << " events" << std::endl;

      std::string name = "jpsi_fromB_data" + getBinString(iRap, iPt);
      binData->SetNameTitle(name.c_str(), "Data for Fitting");

      RooWorkspace* ws = new RooWorkspace(config::Jpsi.workspaceName.c_str());
      ws->import(*binData);

      std::string filename = filenameBase + getBinString(iRap, iPt) + ".root";
      ws->writeToFile(filename.c_str());
    }
  }
}

#endif
