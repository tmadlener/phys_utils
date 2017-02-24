#ifndef PHYSUTILS_POLUTILS_MASSFITB_CONFIG_H__
#define PHYSUTILS_POLUTILS_MASSFITB_CONFIG_H__

#include "general/roofit_utilities.h"
#include "config/GeneralJpsiFromB.h"

#include "RooProduct.h"
#include "RooAddition.h"

#include <vector>
#include <string>
#include <utility>

namespace config {
  struct MassFitBplus {
    const std::vector<std::pair<std::string, double> > constVals = {
      // fix values to the same values as in Jacks code
      {"m_jpsipi_mean1", 5.34693},
      {"m_jpsipi_mean2", 5.46876},
      {"m_jpsipi_mean3", 5.48073},
      {"m_jpsipi_sigma1l", 2.90762e-02},
      {"m_jpsipi_sigma1r", 6.52519e-02},
      {"m_jpsipi_sigma2", 9.94712e-02},
      {"m_jpsipi_sigma3", 3.30152e-01},
      {"m_jpsipi_fraction2", 2.34646e-01},
      {"m_jpsipi_fraction3", 1.14338e-01},

      // {"m_nonprompt_shift", 5.14357},
      // {"m_nonprompt_scale", 1.93204e-02},

      {"f_jpsipi", 4.1e-5/1.026e-3}, // BF(Jpsi pi) = (4.1±0.4)×10−5 / BF(Jpsi K) = (1.026±0.031)×10−3
      // {"f_jpsipi", 0},
      // {"f_nonprompt", 2.50259e-01},
    };


    const std::vector<std::string> fitExpressions = {
      "RooGaussian::m_gaussian1(Bmass, m_mean, m_sigma1)",
      "RooGaussian::m_gaussian2(Bmass, m_mean2, m_sigma2)",
      "RooGaussian::m_gaussian3(Bmass, m_mean3, m_sigma3)",

      // "RooCBShape::m_gaussian2(Bmass, m_mean2, m_sigma2, m_cb_alpha[-2.5,2.5], m_cb_n[-2.5,2.5])",
      // "SUM::pdf_m_signal(m_fraction2 * m_gaussian2, m_gaussian2)", // sticking with the two gaussians, not using CB
      "SUM::pdf_m_signal(m_fraction3 * m_gaussian3, m_fraction2 * m_gaussian2, m_gaussian1)",

      "RooExponential::pdf_m_combinatorial(Bmass, m_exp[-0.3, -20.0, 20.0])", // comb. background via exponential
      // Jpsi Pi background
      "RooBifurGauss::m_jpsipi_gaussian1(Bmass, m_jpsipi_mean1, m_jpsipi_sigma1l, m_jpsipi_sigma1r)",
      "RooGaussian::m_jpsipi_gaussian2(Bmass, m_jpsipi_mean2, m_jpsipi_sigma2)",
      "RooGaussian::m_jpsipi_gaussian3(Bmass, m_jpsipi_mean3, m_jpsipi_sigma3)",
      "SUM::pdf_m_jpsipi(m_jpsipi_fraction3 * m_jpsipi_gaussian3, m_jpsipi_fraction2 * m_jpsipi_gaussian2, m_jpsipi_gaussian1)",
    };

    const std::string fitModelExpr = {
      "SUM::Bmass_model(n_signal * pdf_m_signal, n_combinatorial * pdf_m_combinatorial, n_jpsipi * pdf_m_jpsipi, n_nonprompt * pdf_m_nonprompt)"
    };

    const std::vector<FitVariable> fitVariables = {
      // signal Jpsi K
      {"m_mean", config::Bplus.massPDG, config::Bplus.massMin, config::Bplus.massMax},
      {"m_sigma1", 0.015, 0.001, 0.05},
      {"m_sig2scale", 2.0, 1.0, 8.0},
      {"m_fraction2", 0.5, 0.0, 1.0},
      {"m_shiftm2", 0.0, -2.5, 2.5},

      // 3rd gaussian for signal
      {"m_fraction3", 0.0, 0.0, 1.0},
      {"m_shiftm3", 0.0, -2.5, 2.5},
      {"m_sig3scale", 4.0, 3.0, 20.0},

      // Jpsi Pi (3 gaussians, 1 bifurcated + 2 normal)
      // taking the values as they are in the code from jack and also fixing them to the same values (for now)
      {"m_jpsipi_mean1",5.34693, config::Bplus.massMin, config::Bplus.massMax},
      {"m_jpsipi_mean2", 5.46876, config::Bplus.massMin, config::Bplus.massMax},
      {"m_jpsipi_mean3", 5.48073, config::Bplus.massMin, config::Bplus.massMax},
      {"m_jpsipi_sigma1l", 2.90762e-02, 0.01, 0.15},
      {"m_jpsipi_sigma1r", 6.52519e-02, 0.01, 0.15},
      {"m_jpsipi_sigma2", 9.94712e-02, 0.02, 0.5},
      {"m_jpsipi_sigma3", 3.30152e-01, 0.02, 0.5},
      {"m_jpsipi_fraction2", 2.34646e-01, 0.0, 1.0},
      {"m_jpsipi_fraction3", 1.14338e-01, 0.0, 1.0},

      // Jpsi + hadron background (erfc)
      {"m_nonprompt_shift", 5.14357, 5.12, 5.16},
      {"m_nonprompt_scale", 1.93204e-02, 0.001, 0.03},

      // full model combination variables
      {"n_signal", 5e3, 0, 1e6},
      {"n_combinatorial", 1e3, 0, 1e6},
      {"f_jpsipi", 4.1e-5/1.026e-3, 0, 0.1}, // BF(Jpsi pi) = (4.1±0.4)×10−5 / BF(Jpsi K) = (1.026±0.031)×10−3
      {"f_nonprompt", 2.50259e-01, 0, 0.3},
    };

    const std::vector<FitGeneric*> fitFormulas = {
      new RooClass<RooProduct>{"m_sigma2", "m_sigma1", "m_sig2scale"},
      new RooClass<RooProduct>{"m_deltam2", "m_sigma1", "m_shiftm2"},
      new RooClass<RooAddition>{"m_mean2", "m_mean", "m_deltam2"},
      new GenericPdf<3>{"pdf_m_nonprompt", "TMath::Erfc((Bmass - m_nonprompt_shift) / m_nonprompt_scale)",
                        {"Bmass", "m_nonprompt_shift", "m_nonprompt_scale"}}, // does the order of args matter?

      new RooClass<RooProduct>{"n_jpsipi", "n_signal", "f_jpsipi"},
      new RooClass<RooProduct>{"n_nonprompt", "n_signal", "f_nonprompt"},

      // 3rd signal gaussian
      new RooClass<RooProduct>{"m_sigma3", "m_sigma1", "m_sig3scale"},
      new RooClass<RooProduct>{"m_deltam3", "m_sigma1", "m_shiftm3"},
      new RooClass<RooAddition>{"m_mean3", "m_mean", "m_deltam3"},
    };

    const FitModel fitModel{fitVariables, fitFormulas, fitExpressions, fitModelExpr};
  } bPlusMassFit;
}

#endif
