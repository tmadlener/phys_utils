#ifndef PHYSUTILS_POLUTILS_GENERALJPSIFROMB_CONFIG_H__
#define PHYSUTILS_POLUTILS_GENERALJPSIFROMB_CONFIG_H__

#include <vector>
#include <string>

namespace config {

  const struct GeneralJpsiSettings {
    const double absRapMax = 1.2;

    static constexpr double massPDG = 3.096916; // pdg mass [static]
    static constexpr double massMin = massPDG - 0.2; // GeV
    static constexpr double massMax = massPDG + 0.2; // GeV

    static constexpr size_t FidCuts = 14; // compatibility with old version

    const std::vector<double> ptBinning = {
      10., 12., 14., 16., 18., 20., 22., 25., 30., 35., 40., 50., 70. // jpsi
      // 10., 15., 20., 25., 30., 50. // chic
    };

    // const std::vector<double> rapBinning = {0.0, 0.6, 1.2};
    const std::vector<double> rapBinning = {0.0, 1.2};

    const bool correctCtau = false; /**< correct lifetime by (M_pdg / M_meas) */

    const std::string workspaceName = "ws_masslifetime";
  } Jpsi;

  const struct GeneralBSettings {
    static constexpr double massMin = 5.0;
    static constexpr double massMax = 6.0;
    static constexpr double massPDG = 5.27926;
  } Bplus;
}

#endif
