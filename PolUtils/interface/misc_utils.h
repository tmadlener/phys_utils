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

#endif
