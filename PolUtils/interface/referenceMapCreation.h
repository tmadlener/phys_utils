#ifndef PHYSUTILS_POLUTILS_REFERENCEMAPCREATION_H__
#define PHYSUTILS_POLUTILS_REFERENCEMAPCREATION_H__

#include "TH_utils/createHistFromFunc.h"
#include "general/vector_helper.h"

#include "root_utils.h"

#include "TH2D.h"
#include "TLorentzVector.h"
#include "TRotation.h"
#include "TVector3.h"

#include <sstream>
#include <functional>
#include <utility>
#include <cmath>
#include <fstream>

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

  auto* refMap = createTH2DfromFunc(nBinsCosTh, -1.0, 1.0, nBinsPhi, -180, 180,
                                    std::bind(WcosThetaPhi, _1, _2, lth, lph, ltp), 20000, name.str());
  refMap->SetXTitle("cos#theta");
  refMap->SetYTitle("#phi");

  return refMap;
}

/** calculate angle between J/psi and B in B rest frame. */
std::pair<double, double> calcCosThetaPhiInBFrame(const TLorentzVector* B, const TLorentzVector* Jpsi)
{
  auto labToB = -B->BoostVector();
  auto* jpsiB = clone(*Jpsi);
  jpsiB->Boost(labToB);

  TVector3 Bvec = B->Vect();
  TVector3 jpsiBvec = jpsiB->Vect();

  // rotate xyz axis from laboratory to B rest frame
  jpsiBvec = jpsiBvec.Unit();
  Bvec = Bvec.Unit();

  // defining the y-axis the same way as it is done in the polarization framework to have the correct
  // signs of lambda_theta,phi (automatically)
  static constexpr double beamP = 4000.0; // GeV
  static constexpr double mProt = 0.9382720; // GeV, proton mass
  static constexpr double beamE = std::sqrt(beamP * beamP + mProt * mProt);
  TLorentzVector beam1(0, 0, beamP, beamE);
  TLorentzVector beam2(0, 0, -beamP, beamE);
  beam1.Boost(labToB);
  beam2.Boost(labToB);

  TVector3 newYaxis = beam1.Vect().Unit().Cross(beam2.Vect().Unit()).Unit();
  // avoiding automatic cancellation of diagonal terms in angular distribution by flipping direction with rapidity
  if (B->Rapidity() < 0) newYaxis = -newYaxis;

  TVector3 newXaxis = newYaxis.Cross(Bvec); // Bvec is our new is new z-axis

  TRotation rotation;
  rotation.RotateAxes(newXaxis, newYaxis, Bvec); // now transform from new frame to old lab xyz
  rotation.Invert(); // now lab xyz to new frame

  TVector3 rotJpsiB = jpsiB->Vect();
  rotJpsiB.Transform(rotation);

  static constexpr double pi = std::atan(1) * 4;

  delete jpsiB;
  return {rotJpsiB.CosTheta(), rotJpsiB.Phi() * 180.0 / pi};
}

/** helper struct for easier handling. */
struct Lambdas {
  Lambdas(const double lth_, const double lph_, const double ltp_,
          const double lthErr_, const double lphErr_, const double ltpErr_) :
    lth(lth_), lph(lph_), ltp(ltp_), lthErr(lthErr_), lphErr(lphErr_), ltpErr(ltpErr_) {}
  double lth;
  double lph;
  double ltp;
  double lthErr;
  double lphErr;
  double ltpErr;
  int iRap{};
  int iPt{};
  double meanPt;
};

/** determine the lambdas from the passed data. */
Lambdas calcLambdasFromData(const std::vector<double>& cosTh2, const std::vector<double>& sinTh2cos2Ph,
                            const std::vector<double>& sin2ThcosPh)
{
  // check for nans in input values (should be virtually impossible)
  // auto isnan = [](const double x) { return std::isnan(x); };
  // if (any(cosTh2, isnan) || any(sinTh2cos2Ph, isnan) || any(sin2ThcosPh, isnan)) {
  //   std::cerr << "Got nan in input values. This will propagate through to the lambdas!" << std::endl;
  // }

  const double costh2 = mean(cosTh2);
  const double sinth2cos2ph = mean(sinTh2cos2Ph);
  const double sin2thcosph = mean(sin2ThcosPh);

  const double overCosTh2 = 1 / (1 + costh2); // micro-optimisations ... yay
  const double lth = (1 - 3 * costh2) * overCosTh2;
  const double lph = -sinth2cos2ph * overCosTh2;
  const double ltp = -sin2thcosph * overCosTh2; // Pietro is not sure about the sign here! resp. it depends on the used convention

  // calculate the errors. first calculate the sigma of the above quantities as stddev / sqrt(n)
  const double ct2Sig = stddev(cosTh2, costh2) / std::sqrt((double)cosTh2.size()); // double casting should not be necessary
  const double st2c2pSig = stddev(sinTh2cos2Ph, sin2thcosph) / std::sqrt((double)sinTh2cos2Ph.size());
  const double s2tcpSig = stddev(sin2ThcosPh, sin2thcosph) / std::sqrt((double)sin2ThcosPh.size());

  // errors are now calculated via simple error propagation
  const double overCosTh2Sq = overCosTh2 * overCosTh2;
  const double lthErr = 4 * ct2Sig * overCosTh2Sq;
  const double lphErr = std::sqrt(overCosTh2Sq * st2c2pSig*st2c2pSig +
                                  sinth2cos2ph*sinth2cos2ph * overCosTh2Sq*overCosTh2Sq * ct2Sig*ct2Sig);
  const double ltpErr = std::sqrt(overCosTh2Sq * s2tcpSig*s2tcpSig +
                                  sin2thcosph*sin2thcosph * overCosTh2Sq*overCosTh2Sq * ct2Sig*ct2Sig);

  return {lth, lph, ltp, lthErr, lphErr, ltpErr};
}

/** Store the lambdas in a json file (specified by filename) */
void storeLambdasInJSON(const std::vector<Lambdas>& lambdas, const std::string& filename)
{
  std::ofstream ofs(filename.c_str(), std::fstream::out);
  ofs << "{\n\t\"lambdas\":\n\t[" << std::endl;
  for (size_t i = 0; i < lambdas.size(); ++i) {
    const auto& lam = lambdas[i];
    ofs << "\t\t{";
    ofs << "\"pt\": " << lam.iPt << ", \"rap\": " << lam.iRap << ", ";
    ofs << "\"meanPt\": " << lam.meanPt << ", ";
    ofs << "\"lth\": [" << lam.lth << ", " << lam.lthErr << "], ";
    ofs << "\"lph\": [" << lam.lph << ", " << lam.lphErr << "], ";
    ofs << "\"ltp\": [" << lam.ltp << ", " << lam.ltpErr << "]";
    ofs << "}" << (i == lambdas.size() - 1 ? "": ",") << std::endl; // no comma after last element for valid json
  }

  ofs << "\t]\n}" << std::endl;

  ofs.close();
}


#endif
