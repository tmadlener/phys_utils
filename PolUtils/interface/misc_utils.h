#ifndef PHYSUTILS_POLUTILS_MISCUTILS_H__
#define PHYSUTILS_POLUTILS_MISCUTILS_H__

#include <string>
#include <sstream>

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

#endif
