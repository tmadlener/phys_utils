#ifndef PHYSUTILS_UNBINNEDPOLFW_NORMCALCULATION_H__
#define PHYSUTILS_UNBINNEDPOLFW_NORMCALCULATION_H__

#include "AngularParameters.h"
#include "Data.h"

// stl
#include <array>
#include <iostream> // debugging

/**
 * typedef used for the storage of the coefficients needed to calculate the squared
 * expecation value.
 */
template<size_t N>
using SqSumMatrix = std::array<std::array<double, N>, N>;

/**
 * helper struct holding the expectation values of the different parts needed.
 * Also holds the coefficients needed to calculate the squared expectation value
 */
template<size_t N, size_t M, size_t P>
struct PartialExpValues {
  const double ct2{};
  const std::array<double, N> AL{};
  const std::array<double, N> AL_ct2{};
  const std::array<double, M> Aphi_st2c2p{};
  const std::array<double, P> Atp_s2tcp{};

  // Not yet filled anywhere!
  const SqSumMatrix<N> sqAL{};
  const SqSumMatrix<N> sqAL_ct2{};
  const SqSumMatrix<M> sqAphi_st2c2p{};
  const SqSumMatrix<P> sqAtp_s2tcp{};
};

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

/**
 * calculate the summands to be added for the expectation value calculation.
 * Indices in returned array correspond to indices of the Q
 *
 * Returns an array of size 0.
 */
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

/** add the summands to the sums multiplied by the pased weight. */
template<size_t N>
void addWeighted(std::array<double, N>& sums, const std::array<double, N>& summands,
                 const double weight = 1)
{
  for (size_t i = 0; i < N; ++i) {
    sums[i] += summands[i] * weight;
  }
}

/** add the summands to the sums multiplied by the passed weight. */
template<size_t N>
void addWeighted(SqSumMatrix<N>& sums, const SqSumMatrix<N>& summands,
                 const double weight = 1)
{
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
      sums[i][j] += summands[i][j] * weight;
    }
  }
}

/** rescale all entries by factor n. */
template<size_t N>
void rescale(std::array<double, N>& a, const double n)
{
  for (auto& e : a) e /= n;
}

/** rescale all entries by factor n. */
template<size_t N>
void rescale(SqSumMatrix<N>& m, const double n)
{
  for (auto& a : m) {
    rescale(a, n);
  }
}

/** calculate the needed variables from the angular variables stored in the tree. */
AngularVariables calcAngularVars(const double cosTh, const double phi)
{
  constexpr double pi = std::atan(1) * 4;
  constexpr double toRad = pi / 180.0;

  const double cosTh2 = cosTh*cosTh;
  const double sinTh2 = 1 - cosTh2;
  const double sth2c2p = sinTh2 * std::cos(2 * phi * toRad);
  // 2 * sin(x) * cos(x) = sin(2*x)
  const double s2thcp = 2 * std::sqrt(sinTh2) * cosTh * std::cos(phi * toRad);

  return AngularVariables{cosTh2, sth2c2p, s2thcp};
}

/**
 * calculate the expectation values for each coefficient in the parametrization
 * factoring in the support.
 */
std::array<double, 0> getSupportExpValues(const std::array<double, 0>&,
                                          const std::array<double, 0>&)
{
  return std::array<double, 0>{};
}

/**
 * calculate the expectation values for each coefficient in the parametrization
 * factoring in the support.
 */
std::array<double, 1> getSupportExpValues(const std::array<double, 1>& expVals,
                                          const std::array<double, 1>&)
{
  return expVals;
}

/**
 * calculate the expectation values for each coefficient in the parametrization
 * factoring in the support.
 */
std::array<double, 2> getSupportExpValues(const std::array<double, 2>& expVals,
                                          const std::array<double, 2>& support)
{
  std::array<double, 2> coeffs{};
  coeffs[0] = expVals[0] / (support[0] - support[1]);
  coeffs[1] = expVals[1] / (support[1] - support[0]);

  return coeffs;
}

/**
 * calculate the expectation values for each coefficient in the parametrization
 * factoring in the support.
 */
std::array<double, 3> getSupportExpValues(const std::array<double, 3>& expVals,
                                          const std::array<double, 3>& support)
{
  std::array<double, 3> coeffs{};
  coeffs[0] = expVals[0] / ((support[0] - support[1]) * (support[0] - support[2]));
  coeffs[1] = expVals[1] / ((support[1] - support[0]) * (support[1] - support[2]));
  coeffs[2] = expVals[2] / ((support[2] - support[0]) * (support[2] - support[1]));

  return coeffs;
}

SqSumMatrix<0> calcSquareSummands(const std::array<double, 0>&, const double, const double)
{
  return SqSumMatrix<0>{};
}

SqSumMatrix<1> calcSquareSummands(const std::array<double, 1>&,
                                  const double, const double Y)
{
  return SqSumMatrix<1>{{ Y* Y }};
}

/**
 * [0][0] = (x - x2)^2 * Y^2
 * [1][1] = (x - x1)^2 * Y^2
 * [0][1] = [1][0] = (x - x1) * (x - x2) * Y^2
 */
SqSumMatrix<2> calcSquareSummands(const std::array<double, 2>& supports,
                                  const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);
  const double Ysq = Y*Y;
  SqSumMatrix<2> mat{};
  mat[0][0] = supDiffs[1]*supDiffs[1] * Ysq;
  mat[1][1] = supDiffs[0]*supDiffs[0] * Ysq;
  mat[0][1] = supDiffs[0]*supDiffs[1] * Ysq;
  mat[1][0] = supDiffs[0]*supDiffs[1] * Ysq;

  return mat;
}

/**
 * [0][0] = (x - x2)^2 * (x - x3)^2 * Y^2
 * [1][1] = (x - x1)^2 * (x - x3)^2 * Y^2
 * [2][2] = (x - x1)^2 * (x - x2)^2 * Y^2
 * [0][1] = [1][0] = (x - x2) * (x - x3) * (x - x1) * (x - x3) * Y^2
 * [0][2] = [2][0] = (x - x2) * (x - x3) * (x - x2) * (x - x3) * Y^2
 * [1][2] = [2][1] = (x - x1) * (x - x3) * (x - x1) * (x - x2) * Y^2
 */
SqSumMatrix<3> calcSquareSummands(const std::array<double, 3>& supports,
                                  const double x, const double Y)
{
  const auto supDiffs = calcSupportDiffs(supports, x);
  const double Ysq = Y*Y;
  SqSumMatrix<3> mat{};

  // diagonal
  mat[0][0] = supDiffs[1]*supDiffs[1] * supDiffs[2]*supDiffs[2] * Ysq;
  mat[1][1] = supDiffs[0]*supDiffs[0] * supDiffs[2]*supDiffs[2] * Ysq;
  mat[2][2] = supDiffs[0]*supDiffs[0] * supDiffs[1]*supDiffs[1] * Ysq;

  // off-diagonal
  mat[0][1] = supDiffs[1] * supDiffs[2] * supDiffs[0] * supDiffs[2] * Ysq;
  mat[1][0] = supDiffs[1] * supDiffs[2] * supDiffs[0] * supDiffs[2] * Ysq;

  mat[0][2] = supDiffs[1] * supDiffs[2] * supDiffs[0] * supDiffs[1] * Ysq;
  mat[2][0] = supDiffs[1] * supDiffs[2] * supDiffs[0] * supDiffs[1] * Ysq;

  mat[1][2] = supDiffs[0] * supDiffs[2] * supDiffs[0] * supDiffs[1] * Ysq;
  mat[2][1] = supDiffs[0] * supDiffs[2] * supDiffs[0] * supDiffs[1] * Ysq;

  return mat;
}

template<size_t N, size_t M, size_t P>
PartialExpValues<N,M,P> calcPartialExpVals(const AngularParametrization<N,M,P>& paramA,
                                           const PolData& data)
{

  std::array<double, N> sumsAL{};
  std::array<double, N> sumsALcT2{};
  std::array<double, M> sumsAphiST2c2P{};
  std::array<double, P> sumsAtpS2TcP{};
  double sumCt2{};
  double sumWeight{};

  SqSumMatrix<N> sqSumsAL{};
  SqSumMatrix<N> sqSumsALcT2{};
  SqSumMatrix<M> sqSumsAphiST2c2P{};
  SqSumMatrix<P> sqSumsAtpS2TcP{};

  for (size_t i = 0; i < data.size(); ++i) {
    const AngularVariables angVars{data.cth2[i], data.sth2c2ph[i], data.s2thcph[i]};
    const double weight = data.w[i];
    const double var = data.x[i];

    addWeighted(sumsAL, calcSummands(paramA.getSupAL(), var, 1), weight);
    addWeighted(sumsALcT2, calcSummands(paramA.getSupAL(), var, angVars.cosTh2), weight);
    addWeighted(sumsAphiST2c2P, calcSummands(paramA.getSupAphi(), var, angVars.sinTh2cos2phi), weight);
    addWeighted(sumsAtpS2TcP, calcSummands(paramA.getSupAtp(), var, angVars.sin2ThcosPhi), weight);

    addWeighted(sqSumsAL, calcSquareSummands(paramA.getSupAL(), var, 1), weight);
    addWeighted(sqSumsALcT2, calcSquareSummands(paramA.getSupAL(), var, angVars.cosTh2), weight);
    addWeighted(sqSumsAphiST2c2P, calcSquareSummands(paramA.getSupAphi(), var, angVars.sinTh2cos2phi), weight);
    addWeighted(sqSumsAtpS2TcP, calcSquareSummands(paramA.getSupAtp(), var, angVars.sin2ThcosPhi), weight);

    sumCt2 += angVars.cosTh2 * weight;
    sumWeight += weight;
  }

  // calculate the expecation values
  rescale(sumsAL, sumWeight);
  rescale(sumsALcT2, sumWeight);
  rescale(sumsAphiST2c2P, sumWeight);
  rescale(sumsAtpS2TcP, sumWeight);

  rescale(sqSumsAL, sumWeight);
  rescale(sqSumsALcT2, sumWeight);
  rescale(sqSumsAphiST2c2P, sumWeight);
  rescale(sqSumsAtpS2TcP, sumWeight);

  const double expCt2 = sumCt2 / sumWeight;

  return PartialExpValues<N, M, P>{expCt2,
      getSupportExpValues(sumsAL, paramA.getSupAL()),
      getSupportExpValues(sumsALcT2, paramA.getSupAL()),
      getSupportExpValues(sumsAphiST2c2P, paramA.getSupAphi()),
      getSupportExpValues(sumsAtpS2TcP, paramA.getSupAtp())};
}

/** basically a vector multiplication. TOOD: change name. */
template<size_t N>
double evalExpAtCoeffs(const std::array<double, N>& partialExp,
                       const std::array<double, N>& coeffs)
{
  double sum{};
  for (size_t i = 0; i < N; ++i) {
    sum += partialExp[i] * coeffs[i];
  }
  return sum;
}

/** evaluate the normalization using a given set of values for the parametrization.
 * TODO: proper documentation
 */
template<size_t N, size_t M, size_t P>
double calcExpFcosthphi(const AngularParametrization<N,M,P>& A,
                        const PartialExpValues<N,M,P>& pExpVals)
{
  const double expAL = evalExpAtCoeffs(pExpVals.AL, A.getValAL());
  const double expALct2 = evalExpAtCoeffs(pExpVals.AL_ct2, A.getValAL());
  const double expAphi = evalExpAtCoeffs(pExpVals.Aphi_st2c2p, A.getValAphi());
  const double expAtp = evalExpAtCoeffs(pExpVals.Atp_s2tcp, A.getValAtp());

  return 1.0 + expAL + pExpVals.ct2 - 3.0 * expALct2 + expAphi + expAtp;
}

#endif
