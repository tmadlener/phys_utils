#ifndef PHYSUTILS_POLUTILS_MISCUTILS_H__
#define PHYSUTILS_POLUTILS_MISCUTILS_H__

#include "RooRealVar.h"

#include <string>
#include <sstream>
#include <cmath>

/** get the ending \'_rapX_ptY\' from the passed values. */
template<typename T>
std::string getBinString(const T iRap, const T iPt)
{
  std::stringstream str;
  str << "_rap" << iRap << "_pt" << iPt;
  return str.str();
}

/** check if the passed value is in ragne (min, max) */
template<typename T>
inline bool inRange(const T val, const T min, const T max)
{
  return (val > min && val < max);
}

/**
 * check if the passed variable is in the (var->getMin(), var->getMax()).
 */
inline bool inRange(const double val, const RooRealVar* var)
{
  return inRange(val, var->getMin(), var->getMax());
}

/** get the bin of val in binning. return -1 if no bin can be found. */
template<typename T, typename C>
int getBin(const T val, const C& binning)
{
  for (size_t i = 0; i < binning.size() -1; ++i) {
    if (inRange(val, binning[i], binning[i+1])) return i;
  }
  return -1;
}

/**
 * Get the lower bound of the bin in the passed binning.
 * CAUTION: no boundary checks
 */
template<typename C>
inline typename C::value_type getBinMin(const size_t bin, const C& binning)
{
  return (!bin ? binning.front() : binning[bin - 1]);
}

/**
 * Get the upper bound of the bin in the passed binning.
 * CAUTION: no boundary checks
 */
template<typename C>
inline typename C::value_type getBinMax(const size_t bin, const C& binning)
{
  return (!bin ? binning.back() : binning[bin]);
}

/**
 * reduce the range of the passed variable into the range of [-pi, pi].
 */
template<typename T>
T reduceRange(T x)
{
  constexpr T pi = T(M_PI);
  constexpr T o2pi = 1.0 / (2*pi);

  if(std::abs(x) <= T(M_PI)) return x;

  auto n = std::round(x * o2pi);
  return x - n * 2 * pi;
}

/**
 * produce a cut-string that can be used in e.g. TTree::Draw() from the passed binning and bin.
 * If bin == 0, the full range is returned.
 * NOTE: there is no out-of-range check!
 */
template<typename T>
std::string getCutString(const size_t bin, const std::vector<T>& binning, const std::string& varname)
{
  // get the range from the binning, depending if the full range is wanted, or only one bin
  // const T min = !bin ? binning.front() : binning[bin - 1];
  // const T max = !bin ? binning.back() : binning[bin];
  const T min = getBinMin(bin, binning);
  const T max = getBinMax(bin, binning);

  std::stringstream cutstr;
  cutstr << "(" << varname << " >= " << min << " && " << varname << " <= " << max << ")";

  return cutstr.str();
}

#endif
