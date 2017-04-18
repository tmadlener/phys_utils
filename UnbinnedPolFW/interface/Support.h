#ifndef PHYSUTILS_UNBINNEDPOLFW_SUPPORT_H__
#define PHYSUTILS_UNBINNEDPOLFW_SUPPORT_H__

#include "AngularParameters.h"
#include "NormCalculation.h"

#include "general/vector_helper.h"

// ROOT
#include "TTree.h"
#include "TH1D.h"

// stl
#include <array>
#include <vector>
#include <string>
#include <cmath>
#include <iostream> // mainly debugging

/** helper struct */
struct BranchNames {
  std::string weight;
  std::string cosTh;
  std::string phi;
};

/** helper struct */
struct AngularVariables {
  double cosTh2;
  double sinTh2cos2phi;
  double sin2ThcosPhi;
};

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T,N>& a)
{
  for (const auto& e : a) os << e << " ";
  return os;
}

/** helper function to calculate the differences between the support points and the passed value. */
template<size_t N>
std::array<double, N> calcSupportDiffs(const std::array<double, N>& sups, const double x)
{
  std::array<double, N> diffs;
  for (size_t i = 0; i < N; ++i) {
    diffs[i] = x - sups[i];
  }
  return diffs;
}


std::array<double,0> calcSummands(const std::array<double, 0>&, const double, const double)
{
  return std::array<double,0>{};
}

/**
 * calculate the summands to be added for the expectation value calculation.
 * Indices in returned array correspond to indices of the Q
 *
 * If we only have one support point, the summand is only the passed value (i.e. constant)
 */
std::array<double, 1> calcSummands(const std::array<double, 1>&,
                                   const double, const double Y)
{
  return std::array<double, 1>{Y};
}

/**
 * calculate the summands to be added for the expectaition value calculation.
 * Indices in returned array correspond to indices of the Q
 *
 * for linear parametrization we have two support points.
 * The summand for the first support point is (x - x2) * Y,
 * the summand for the second support point is (x - x1) * Y
 */
std::array<double, 2> calcSummands(const std::array<double, 2>& supports,
                                   const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);
  return std::array<double, 2>{supDiffs[1] * Y, supDiffs[0] * Y};
}

/**
 * calculate the summands to be added for the expectation value calculation.
 * Indices in returned array correspond to indices of the Q
 *
 * for a parabolic parametrization we have three support points.
 * The summand for the first support point is (x - x2) * (x - x3) * Y
 * The summand for the second support point is (x - x1) * (x - x3) * Y
 * The summand for the third support point is (x - x1) * (x - x2) * Y
 */
std::array<double, 3> calcSummands(const std::array<double, 3>& supports,
                                   const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);
  return std::array<double, 3>{supDiffs[1] * supDiffs[2] * Y,
      supDiffs[0] * supDiffs[2] * Y,
      supDiffs[0] * supDiffs[1] * Y};
}

/**
 * calculate the summands needed for the squared expectation value calculation
 *
 * constant (one support poine), summand is the square of the passed value
 */
std::array<double, 1> calcSquareSummands(const std::array<double, 1>&,
                                         const double, const double Y)
{
  return std::array<double, 1> { Y*Y };
}

/**
 * calculate the summands needed for the squared expectation value calculation
 *
 * linear parametrization.
 * first element: (x - x2)^2 * Y^2
 * second element: (x - x1)^2 * Y^2
 * third element: (x - x1)*(x - x2) * Y^2
 */
std::array<double, 3> calcSquareSummands(const std::array<double, 2>& supports,
                                         const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);

  return std::array<double, 3> {
    supDiffs[1]*supDiffs[1] * Y*Y,
      supDiffs[0]*supDiffs[0] * Y*Y,
      supDiffs[0] * supDiffs[1] * Y*Y
      };
}

/**
 * calculate the summands needed for the squared expectation value calculation
 *
 * parabolic parametrization.
 * first element:  (x - x2)^2 * (x - x3)^2 * Y^2
 * second element: (x - x1)^2 * (x = x3)^2 * Y^2
 * third element:  (x - x1)^2 * (x - x2)^2 * Y^2
 * fourth element: -(x - x1) * (x - x2) * (x - x3)^2 * Y^2
 * fifth elment:   (x - x1) * (x - x2)^2 * (x - x3) * Y^2
 * sixth element:  -(x - x1)^2 *& (x - x2) * (x  x3) * Y^2
 */
std::array<double, 6> calcSquareSummands(const std::array<double, 3>& supports,
                                         const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);
  return std::array<double, 6> {
    supDiffs[1]*supDiffs[1] * supDiffs[2]*supDiffs[2] * Y*Y,
      supDiffs[0]*supDiffs[0] * supDiffs[2]*supDiffs[2] * Y*Y,
      supDiffs[0]*supDiffs[0] * supDiffs[1]*supDiffs[1] * Y*Y,
      -supDiffs[0] * supDiffs[1] * supDiffs[2]*supDiffs[2] * Y*Y,
      supDiffs[0] * supDiffs[1]*supDiffs[1] * supDiffs[2] * Y*Y,
      -supDiffs[0]*supDiffs[0] * supDiffs[1] *supDiffs[2] * Y*Y
      };
}


template<size_t N>
void addWeighted(std::array<double, N>& sums, const std::array<double, N>& summands,
                 const double weight = 1)
{
  for (size_t i = 0; i < N; ++i) {
    sums[i] += summands[i] * weight;
  }
}

template<size_t N>
void rescale(std::array<double, N>& a, const double n)
{
  for (auto& e : a) e /= n;
}


void setSupportExpValues(const std::array<double, 0>&, const std::array<double,0>&,
                         std::array<double,0>&)
{
  // No-op
}

void setSupportExpValues(const std::array<double, 1>& expVals,
                         const std::array<double, 1>&,
                         std::array<double, 1>& supportCoeffs)
{
  supportCoeffs[0] = expVals[0];
}

void setSupportExpValues(const std::array<double, 2>& expVals,
                         const std::array<double, 2>& support,
                         std::array<double, 2>& supportCoeffs)
{
  supportCoeffs[0] = expVals[0] / (support[0] - support[1]);
  supportCoeffs[1] = expVals[1] / (support[1] - support[0]);
}

void setSupportExpValues(const std::array<double, 3>& expVals,
                         const std::array<double, 3>& support,
                         std::array<double, 3>& supportCoeffs)
{
  supportCoeffs[0] = expVals[0] / ((support[0] - support[1]) * (support[0] - support[2]));
  supportCoeffs[1] = expVals[1] / ((support[1] - support[0]) * (support[1] - support[2]));
  supportCoeffs[2] = expVals[2] / ((support[2] - support[0]) * (support[2] - support[1]));
}


template<size_t N>
void printRescale(std::array<double, N>& a, const double n)
{
  rescale(a, n);
  std::cout << a << '\n';
}

template<size_t N, size_t M, size_t P>
class Support {
public:
  Support(const std::string& varName, const std::array<double, N>& AL,
          const std::array<double,M>& Aphi, const std::array<double, P>& Atp) :
    m_varName(varName), m_AL(AL), m_Aphi(Aphi), m_Atp(Atp) {}

  /** Initialize, set branch addresses for all the necessary variables. */
  void Init(TTree* t, const BranchNames& bNames);

  void calcSupportExpVals(TTree* t);

  double evalExpValueAL(const std::array<double, N>& paramsAL) const;
  double evalExpValueAphi(const std::array<double, M>& paramsAphi) const;
  double evalExpValueAtp(const std::array<double, P>& paramsAtp) const;

  /** create an angular parameters parametrization using the same support points. */
  AngularParametrization<N,M,P> getAngParams() const;

  PartialExpValues<N,M,P> getPartialExpValues() const;

private:
  const std::string m_varName; /**< variable name (as stored in TTree). */
  const std::array<double, N> m_AL; /**< support points for A_L. */
  const std::array<double, M> m_Aphi; /**< support points for A_phi. */
  const std::array<double, P> m_Atp; /**< support points for A_theta,phi. */

  double m_var{};
  double m_weight{};
  double m_cosTh;
  double m_phi;

  std::array<double, N> m_expAL{}; /**< parameters to calculate the expectation value of AL. */
  std::array<double, N*(N+1)/2> m_sqExpAL{}; /**< parameters to calculate the squared expectation value of AL. */
  std::array<double, N> m_expALct2{}; /**< parameters for <A_L*costheta^2>. */

  std::array<double, M> m_expAphi{};
  std::array<double, M*(M+1)/2> m_sqExpAphi{};
  std::array<double, P> m_expAtp{};
  std::array<double, P*(P+1)/2> m_sqExpAtp{};

  double m_expCt2; /**< expecation value of costheta^2*/

  /**
   * calculate cosTh^2, sinTh^2 * cos(2*phi) and sin(2*theta) * cos(phi) for an event
   */
  AngularVariables calcAngularVars() const;
};

template<size_t N, size_t M, size_t P>
AngularParametrization<N,M,P> Support<N,M,P>::getAngParams() const
{
  return AngularParametrization<N,M,P>{m_AL, m_Aphi, m_Atp};
}

template<size_t N, size_t M, size_t P>
PartialExpValues<N,M,P> Support<N,M,P>::getPartialExpValues() const
{
  return PartialExpValues<N,M,P>{m_expCt2, m_expAL, m_expALct2, m_expAphi, m_expAtp};
}

template<size_t N, size_t M, size_t P>
void Support<N,M,P>::Init(TTree* t, const BranchNames& bNames)
{
  // t->SetBranchStatus("*", 0);
  t->SetBranchAddress(m_varName.c_str(), &m_var);
  t->SetBranchAddress(bNames.weight.c_str(), &m_weight);
  t->SetBranchAddress(bNames.cosTh.c_str(), &m_cosTh);
  t->SetBranchAddress(bNames.phi.c_str(), &m_phi);
}

template<size_t N, size_t M, size_t P>
void Support<N,M,P>::calcSupportExpVals(TTree* t)
{
  const int nEntries = t->GetEntries();
  std::array<double, N> sumsAL{};
  // std::array<double, N*(N+1)/2> sqSumsAL{}; // squared sums for squared expect value
  std::array<double, N> sumsALcT2{};
  // std::array<double, N*(N+1)/2> sqSumsALcT2{};
  std::array<double, M> sumsAphiST2cP{};
  // std::array<double, M*(M+1)/2> sqSumsAphiST2cP{};
  std::array<double, P> sumsAtpS2TcP{};
  // std::array<double, P*(P+1)/2> sqSumsAtpS2TcP{};
  double sumCt2{};

  double sumWeight{};
  for (int i = 0; i < nEntries; ++i) {
    t->GetEntry(i);

    AngularVariables angVars = calcAngularVars();
    addWeighted(sumsAL, calcSummands(m_AL, m_var, 1), m_weight);
    addWeighted(sumsALcT2, calcSummands(m_AL, m_var, angVars.cosTh2), m_weight);
    addWeighted(sumsAphiST2cP, calcSummands(m_Aphi, m_var, angVars.sinTh2cos2phi), m_weight);
    addWeighted(sumsAtpS2TcP, calcSummands(m_Atp, m_var, angVars.sin2ThcosPhi), m_weight);

    // addWeighted(sqSumsAL, calcSquareSummands(m_AL, m_var, 1), m_weight);
    // addWeighted(sqSumsALcT2, calcSquareSummands(m_AL, m_var, angVars.cosTh2), m_weight);
    // addWeighted(sqSumsAphiST2cP, calcSquareSummands(m_Aphi, m_var, angVars.sinTh2cos2phi), m_weight);
    // addWeighted(sqSumsAtpS2TcP, calcSquareSummands(m_Atp, m_var, angVars.sin2ThcosPhi), m_weight);

    sumCt2 += angVars.cosTh2 * m_weight;
    sumWeight += m_weight;
  }

  rescale(sumsAL, sumWeight);
  rescale(sumsALcT2, sumWeight);
  rescale(sumsAphiST2cP, sumWeight);
  rescale(sumsAtpS2TcP, sumWeight);

  // rescale(sqSumsAL, sumWeight);
  // rescale(sqSumsALcT2, sumWeight);
  // rescale(sqSumsAphiST2cP, sumWeight);
  // rescale(sqSumsAtpS2TcP, sumWeight);

  setSupportExpValues(sumsAL, m_AL, m_expAL);
  setSupportExpValues(sumsALcT2, m_AL, m_expALct2);
  setSupportExpValues(sumsAphiST2cP, m_Aphi, m_expAphi);
  setSupportExpValues(sumsAtpS2TcP, m_Atp, m_expAtp);

  m_expCt2 = sumCt2 / sumWeight;
}

template<size_t N, size_t M, size_t P>
AngularVariables Support<N,M,P>::calcAngularVars() const
{
  constexpr double pi = std::atan(1) * 4;
  constexpr double toRad = pi / 180.0;

  const double cosTh2 = m_cosTh*m_cosTh;
  const double sinTh2 = 1 - cosTh2;
  const double sth2c2p = sinTh2 * std::cos(2 * m_phi * toRad);
  // 2 * sin(x) * cos(x) = sin(2*x)
  const double s2thcp = 2 * std::sqrt(sinTh2) * m_cosTh * std::cos(m_phi * toRad);

  return AngularVariables{cosTh2, sth2c2p, s2thcp};
}

template<size_t N>
double multParamsCoeffs(const std::array<double, N>& params,
                        const std::array<double, N>& coeffs)
{
  double sum{};
  for (size_t i = 0; i < N; ++i) {
    sum += params[i] * coeffs[i];
  }
  return sum;
}

template<size_t N, size_t M, size_t P>
double Support<N,M,P>::evalExpValueAL(const std::array<double, N>& paramsAL) const
{
  return multParamsCoeffs(paramsAL, m_expAL);
}

template<size_t N, size_t M, size_t P>
double Support<N,M,P>::evalExpValueAphi(const std::array<double, M>& paramsAphi) const
{
  return multParamsCoeffs(paramsAphi, m_expAphi);
}

template<size_t N, size_t M, size_t P>
double Support<N,M,P>::evalExpValueAtp(const std::array<double, P>& paramsAtp) const
{
  return multParamsCoeffs(paramsAtp, m_expAtp);
}

#endif
