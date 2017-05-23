#ifndef PHYSUTILS_POLUTILS_BROOTUPLEEVENT_H__
#define PHYSUTILS_POLUTILS_BROOTUPLEEVENT_H__

// forward declarations
class TTree;

struct BRootupleEvent {
  void Init(TTree* t);
  void Create(TTree* t);

  double JpsiPt;
  double JpsiRap;
  double JpsiMass;
  double JpsiCtauSig;

  double MuPPt;
  double MuNPt;
  double MuPEta;
  double MuNEta;
  double MuPPhi;
  double MuNPhi;

  double cosTh_HX;
  double phi_HX;
  double cosTh_PX;
  double phi_PX;
  double cosTh_CS;
  double phi_CS;

  double bMass;
  double bPt;
  double bRap;

  double trackPt;

  int event;
  int run;
  int lumi;
};

void BRootupleEvent::Init(TTree* t)
{
  t->SetBranchAddress("JpsiPt", &JpsiPt);
  t->SetBranchAddress("JpsiRap", &JpsiRap);
  t->SetBranchAddress("JpsiMass", &JpsiMass);
  t->SetBranchAddress("JpsiCtauSig", &JpsiCtauSig);

  t->SetBranchAddress("MuPPt", &MuPPt);
  t->SetBranchAddress("MuNPt", &MuNPt);
  t->SetBranchAddress("MuPEta", &MuPEta);
  t->SetBranchAddress("MuNEta", &MuNEta);
  t->SetBranchAddress("MuPPhi", &MuPPhi);
  t->SetBranchAddress("MuNPhi", &MuNPhi);

  t->SetBranchAddress("cosTh_HX", &cosTh_HX);
  t->SetBranchAddress("phi_HX", &phi_HX);
  t->SetBranchAddress("cosTh_PX", &cosTh_PX);
  t->SetBranchAddress("phi_PX", &phi_PX);
  t->SetBranchAddress("cosTh_CS", &cosTh_CS);
  t->SetBranchAddress("phi_CS", &phi_CS);

  t->SetBranchAddress("bMass", &bMass);
  t->SetBranchAddress("bPt", &bPt);
  t->SetBranchAddress("bRap", &bRap);

  t->SetBranchAddress("trackPt", &trackPt);

  t->SetBranchAddress("event", &event);
  t->SetBranchAddress("run", &run);
  t->SetBranchAddress("lumi", &lumi);
}

void BRootupleEvent::Create(TTree* t)
{
  t->Branch("JpsiPt", &JpsiPt);
  t->Branch("JpsiRap", &JpsiRap);
  t->Branch("JpsiMass", &JpsiMass);
  t->Branch("JpsiCtauSig", &JpsiCtauSig);

  t->Branch("MuPPt", &MuPPt);
  t->Branch("MuNPt", &MuNPt);
  t->Branch("MuPEta", &MuPEta);
  t->Branch("MuNEta", &MuNEta);
  t->Branch("MuPPhi", &MuPPhi);
  t->Branch("MuNPhi", &MuNPhi);

  t->Branch("cosTh_HX", &cosTh_HX);
  t->Branch("phi_HX", &phi_HX);
  t->Branch("cosTh_PX", &cosTh_PX);
  t->Branch("phi_PX", &phi_PX);
  t->Branch("cosTh_CS", &cosTh_CS);
  t->Branch("phi_CS", &phi_CS);

  t->Branch("bMass", &bMass);
  t->Branch("bPt", &bPt);
  t->Branch("bRap", &bRap);

  t->Branch("trackPt", &trackPt);

  t->Branch("event", &event);
  t->Branch("run", &run);
  t->Branch("lumi", &lumi);
}

#endif
