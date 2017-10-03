#ifndef PHYSUTILS_GENERAL_MISCUTILS_H__
#define PHYSUTILS_GENERAL_MISCUTILS_H__

#include <string>
#include <regex>
#include <iostream>

/**
 * extract the 'rapX_ptY' part from the passed string (must not necessarily be a filename)
 */
std::string getBinFromFile(const std::string& filename)
{
  // declaring a separate (char *) string here to be able to print it later in case
  constexpr auto rgxstr = ".*_rap([0-9]+)_pt([0-9]+).*";
  std::regex rgx(rgxstr);
  std::smatch cm;
  if (std::regex_match(filename, cm, rgx)) {
    return "rap" + cm[1].str() + "_pt" + cm[2].str();
  }

  std::cerr << "Could't match " << rgxstr << " to " << filename << "\n";
  return "";
}


#endif
