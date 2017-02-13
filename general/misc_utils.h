#ifndef PHYSUTILS_GENERAL_MISCUTILS_H__
#define PHYSUTILS_GENERAL_MISCUTILS_H__

/** check if the passed value is in ragne (min, max) */
template<typename T>
inline bool inRange(const T val, const T min, const T max)
{
  return (val >= min && val < max);
}

/** get the bin of val in binning. return -1 if no bin can be found. */
template<typename T, typename C>
int getBin(const T val, const C& binning)
{
  for (size_t i = 0; i < binning.size() - 1; ++i) {
    if (inRange(val, binning[i], binning[i+1])) return i;
  }
  return -1;
}

#endif
