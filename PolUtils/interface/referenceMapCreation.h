#ifndef PHYSUTILS_POLUTILS_REFERENCEMAPCREATION_H__
#define PHYSUTILS_POLUTILS_REFERENCEMAPCREATION_H__

#include "TH_utils/createHistFromFunc.h"


#include "TH2D.h"

#include <sstream>
#include <functional>


/**
 * \f[
 * W(\cos\theta,\phi | \vec{\lambda}) = \frac{3}{4\pi(3+\lambda_{\theta})} (
 *                   1 + \lambda_{\theta}\cos^2\theta +
 *                   \lambda_{\phi}\sin^2\theta\cos 2\phi +
 *                   \lambda_{\theta\phi}\sin 2\theta\cos\phi )
 * \f]
 */
double WcosThetaPhi(const double cosTheta, const double phi, const double lth,
                    const double lph, const double ltp)
{
  using namespace std;
  const double costh2 = cosTheta * cosTheta;
  const double sinth2 = 1 - costh2;
  static constexpr double pi = atan(1) * 4;
  static constexpr double toDeg = pi / 180.0;

  return 3  / (4*pi * (3 + lth)) * (1 +
                                    lth * costh2 +
                                    lph * sinth2 * cos(2*phi*toDeg) +
                                    ltp * 2 * sqrt(sinth2) * cosTheta * cos(phi*toDeg)
                                    // 2 * sinTheta * cosTheta = sin(2*Theta)
                                    );
}

/**
 * create a reference cosThetaPhi map from the passed lth, lph and ltp parameters in the form of a TH2D, where each
 * contains the bin-integrated value of W(cosTheta, phi)
 * Doing this not via TF2::CreateHistogram() because that evaluates the function at the bin center only!
 */
TH2D* createReferenceMap(const double lth, const double lph, const double ltp,
                         const int nBinsCosTh = 64, const int nBinsPhi = 16)
{
  using namespace std::placeholders;
  std::stringstream name;
  name << "cosThPhi_reference_" << std::fixed << std::setprecision(1);
  name << "lth_" << lth << "_lph_" << lph
       << "_ltp_" << ltp << "_nct_" << nBinsCosTh << "_nph_" << nBinsPhi;
  return createTH2DfromFunc(nBinsCosTh, -1.0, 1.0, nBinsPhi, -180, 180,
                            std::bind(WcosThetaPhi, _1, _2, lth, lph, ltp), 20000, name.str());
}


#endif
