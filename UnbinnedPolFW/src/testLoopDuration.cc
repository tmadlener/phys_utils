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
#include <chrono>
#include <algorithm>

using Clock = std::chrono::high_resolution_clock;

#ifndef __CINT__
int main(int argc, char* argv[])
{
  ArgParser parser(argc, argv);
  const auto refFileName = parser.getOptionVal<std::string>("--reffile");
  const auto dataFileName = parser.getOptionVal<std::string>("--datafile");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile");

  TFile* refF = TFile::Open(refFileName.c_str());

  AngularParametrization<3, 2, 1> angParams{{10, 15, 50}, {25, 40}, {10}};
  DataBranchNames branches{"pT", "pT", "costh_HX", "phi_HX", "wS"};
  const auto refData = readFromFile(refF, "genData", branches,
                                    EmptyRange{}, EmptyRange{});

  const auto partialExpValues = calcPartialExpVals(angParams, refData);

  refF->Close();

  // read data
  TFile* f = TFile::Open(dataFileName.c_str());
  const auto data = readFromFile(f, "genData", branches, EmptyRange{}, EmptyRange{});
  f->Close();

  const auto rangeAL = linspace(0.24, 0.44, 7);
  const auto rangeAphi = linspace(-0.1, 0.1, 3);
  double Atp = 0;

  TFile* fout = new TFile(outFileName.c_str(), "recreate");
  TTree* tout = new TTree("llValues", "log likelihood values and parameters");

  std::array<double, 3> AL{};
  std::array<double, 2> Aphi{};
  double logL{};

  tout->Branch("AL_0", &AL[0]);
  tout->Branch("AL_1", &AL[1]);
  tout->Branch("AL_2", &AL[2]);
  tout->Branch("Aphi_0", &Aphi[0]);
  tout->Branch("Aphi_1", &Aphi[1]);
  tout->Branch("Atp", &Atp);
  tout->Branch("logL", &logL);

  std::vector<long int> fitTimes;
  fitTimes.reserve(7*7*7 * 3*3);

  for (const auto& AL0 : rangeAL) {
    for (const auto& AL1 : rangeAL) {
      for (const auto& AL2 : rangeAL) {
        for (const auto& Aphi0 : rangeAphi) {
          for (const auto& Aphi1 : rangeAphi) {
            const auto st = Clock::now();

            AL = std::array<double, 3>{AL0, AL1, AL2};
            Aphi = std::array<double, 2>{Aphi0, Aphi1};
            angParams.setVals(AL, Aphi, {Atp});
            logL = calcLogL(data, angParams, partialExpValues);
            tout->Fill();

            fitTimes.push_back(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - st).count());
          }
        }
      }
    }
  }

  double m = mean(fitTimes);
  double std = stddev(fitTimes, m);

  std::cout << "average time per point: " << m << " +/- " << std << " us\n";

  tout->Write();
  fout->Close();

  return 0;
}

#endif
