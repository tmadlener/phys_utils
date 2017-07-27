#include "../PolUtils/interface/TTreeLooper.h"
#include "../PolUtils/interface/root_utils.h"
#include "../PolUtils/interface/calcAngles.h"

#include "general/ArgParser.h"

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TLorentzVector.h"

#include <vector>
#include <string>
#include <iostream>

constexpr double M_JPSI_PDG = 3.096916;

struct ChicTupleEvent {
  void Create(TTree* t);

  double chicPt;
  double chicRap;
  double chicMass;
  double chicGenMass;
  double chicMassQ;

  double jpsiPt;
  double jpsiRap;
  double jpsiMass;

  double photonPt;
  double photonEta;
  double convVtxR;

  double MuPPt;
  double MuPEta;
  double MuPPhi;
  double MuNPt;
  double MuNEta;
  double MuNPhi;

  double cosTh_HX;
  double phi_HX;
  double cosTh_PX;
  double phi_PX;
  double cosTh_CS;
  double phi_CS;

  double vtxProb;

  int event;
  int run;
};

void ChicTupleEvent::Create(TTree* t)
{
  t->Branch("chicPt", &chicPt);
  t->Branch("chicRap", &chicRap);
  t->Branch("chicMass", &chicMass);
  t->Branch("chicGenMass", &chicGenMass);
  t->Branch("chicMassQ", &chicMassQ);

  t->Branch("jpsiPt", &jpsiPt);
  t->Branch("jpsiRap", &jpsiRap);
  t->Branch("jpsiMass", &jpsiMass);

  t->Branch("photonPt", &photonPt);
  t->Branch("photonEta", &photonEta);
  t->Branch("convVtxR", &convVtxR);

  t->Branch("MuPPt", &MuPPt);
  t->Branch("MuPEta", &MuPEta);
  t->Branch("MuPPhi", &MuPPhi);
  t->Branch("MuNPt", &MuNPt);
  t->Branch("MuNEta", &MuNEta);
  t->Branch("MuNPhi", &MuNPhi);

  t->Branch("cosTh_HX", &cosTh_HX);
  t->Branch("phi_HX", &phi_HX);
  t->Branch("cosTh_PX", &cosTh_PX);
  t->Branch("phi_PX", &phi_PX);
  t->Branch("cosTh_CS", &cosTh_CS);
  t->Branch("phi_CS", &phi_CS);

  t->Branch("vtxProb", &vtxProb);

  t->Branch("event", &event);
  t->Branch("run", &run);
}

struct ChicInputEvent {
  void Init(TTree* t);

  int event;
  int run;

  const TLorentzVector& jpsi() const { return *dimuon_p4; }
  const TLorentzVector& chic() const { return *chi_p4; }
  const TLorentzVector& gen_chic() const { return *gen_chi_p4; }
  const TLorentzVector& muP() const { return *muonP_p4; }
  const TLorentzVector& muN() const { return *muonN_p4; }
  const TLorentzVector& photon() const {return *photon_p4; }

  TLorentzVector *chi_p4{nullptr};
  TLorentzVector *gen_chi_p4{nullptr};
  TLorentzVector *dimuon_p4{nullptr};
  TLorentzVector *muonP_p4{nullptr};
  TLorentzVector *muonN_p4{nullptr};
  TLorentzVector *photon_p4{nullptr};

  double vtxProb;
  double convVtxR;
};

void ChicInputEvent::Init(TTree* t)
{
  t->SetBranchAddress("event", &event);
  t->SetBranchAddress("run", &run);

  t->SetBranchAddress("chi_p4", &chi_p4);
  t->SetBranchAddress("gen_chic_p4", &gen_chi_p4);

  t->SetBranchAddress("dimuon_p4", &dimuon_p4);
  t->SetBranchAddress("muonP_p4", &muonP_p4);
  t->SetBranchAddress("muonN_p4", &muonN_p4);
  t->SetBranchAddress("photon_p4", &photon_p4);


  t->SetBranchAddress("probFit1S", &vtxProb);
  t->SetBranchAddress("conv_vertex", & convVtxR);
}

bool chicPGunMCTupling(const ChicInputEvent& inEvent, ChicTupleEvent& event)
{
  event.jpsiPt = inEvent.jpsi().Pt();
  event.jpsiRap = inEvent.jpsi().Rapidity();
  event.jpsiMass = inEvent.jpsi().M();

  event.chicPt = inEvent.chic().Pt();
  event.chicRap = inEvent.chic().Rapidity();
  event.chicMass = inEvent.chic().M();
  event.chicGenMass = inEvent.gen_chic().M();
  event.chicMassQ = inEvent.gen_chic().M() - inEvent.jpsi().M() + M_JPSI_PDG;

  event.photonPt = inEvent.photon().Pt();
  event.photonEta = inEvent.photon().Eta();

  event.MuPPt = inEvent.muP().Pt();
  event.MuPEta = inEvent.muP().Eta();
  event.MuPPhi = inEvent.muP().Phi();
  event.MuNPt = inEvent.muP().Pt();
  event.MuNEta = inEvent.muP().Eta();
  event.MuNPhi = inEvent.muP().Phi();

  const auto anglesHX = calcAnglesInFrame(inEvent.muP(), inEvent.muN(), RefFrame::HX);
  event.cosTh_HX = anglesHX.costh;
  event.phi_HX = anglesHX.phi;

  const auto anglesPX = calcAnglesInFrame(inEvent.muP(), inEvent.muN(), RefFrame::PX);
  event.cosTh_PX = anglesPX.costh;
  event.phi_PX = anglesPX.phi;

  const auto anglesCS = calcAnglesInFrame(inEvent.muP(), inEvent.muN(), RefFrame::CS);
  event.cosTh_CS = anglesCS.costh;
  event.phi_CS = anglesCS.phi;

  event.vtxProb = inEvent.vtxProb;
  event.convVtxR = inEvent.convVtxR;

  event.run = inEvent.run;
  event.event = inEvent.event;

  return true;
}


#if !(defined(__CINT__) or defined(__CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto inputFiles = parser.getOptionVal<std::vector<std::string>>("--files");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile", "chic_tuple.root");

  TTree* tin = createTChain(inputFiles, "rootuple/chicTree");

  TFile* fout = new TFile(outFileName.c_str(), "recreate");
  TTree* tout = new TTree("chic_tuple", "tupled pGun chic MC events");
  tout->SetDirectory(fout);

  TTreeLooper<ChicInputEvent, ChicTupleEvent> treeLooper(tin, tout);
  treeLooper.loop(chicPGunMCTupling, -1);

  fout->Write();
  fout->Close();

  return 0;
}
#endif
