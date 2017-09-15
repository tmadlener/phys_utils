#include "NormCalculation.h"
#include "FileHelper.h"
#include "LLCalculation.h"

#include "general/vector_helper.h"
#include "general/ArgParser.h"

// ROOT
#include "TFile.h"
#include "TTree.h"

// stl
#include <iostream>
#include <vector>
#include <array>

#ifndef __CINT__
int main(int argc, char* argv[])
{
  ArgParser parser(argc, argv);
  const auto refFileName = parser.getOptionVal<std::string>("--reffile");
  const auto dataFileName = parser.getOptionVal<std::string>("--datafile");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile");

  TFile* refF = TFile::Open(refFileName.c_str());

  AngularParametrization<1, 0, 0> angParams{{}, {}, {}};
  DataBranchNames branches{"pT", "pT", "costh_HX", "phi_HX", "wS"};
  const auto refData = readFromFile(refF, "genData", branches,
                                    Range<RangeType::Empty>{}, Range<RangeType::Empty>{});

  const auto partialExpValues = calcPartialExpVals(angParams, refData);

  refF->Close();

  // read data
  TFile* f = TFile::Open(dataFileName.c_str());
  const auto data = readFromFile(f, "genData", branches, EmptyRange{}, EmptyRange{});
  f->Close();

  double AL{};
  double logL{};
  double lth{};

  TFile* fout = new TFile(outFileName.c_str(), "recreate");
  TTree* tout = new TTree("llValues", "log likelihood values and parameters");

  tout->Branch("AL", &AL);
  tout->Branch("logL", &logL);
  tout->Branch("lth", &lth);

  std::vector<double> lthVals = linspace(-0.2, 0.2, 1000);
  for (const auto& l : lthVals) {
    lth = l;
    AL = (1 - lth) / (3 + lth);
    angParams.setVals({AL},{},{});
    logL = calcLogL(data, angParams, partialExpValues);
    tout->Fill();
  }

  tout->Write();
  fout->Close();

  return 0;
}

#endif
