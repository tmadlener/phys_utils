#ifndef PHYSUTILS_STRING_HELPER_H__
#define PHYSUTILS_STRING_HELPER_H__

#include <string>
#include <vector>
#include <sstream>

/** check if string starts with another string. */
inline bool startsWith(const std::string& input, const std::string& prefix)
{
  return input.substr(0, prefix.length()) == prefix;
}

/** split string at delimiter delim and return vector of all substrings. If a token is empty it will be ignored. */
std::vector<std::string> splitString(const std::string& in, const char delim)
{
  std::vector<std::string> tokens;
  std::stringstream sstr(in);
  std::string tok;
  while(std::getline(sstr,tok,delim)) {
    if(!tok.empty()) {
      tokens.push_back(tok);
    }
  }

  return tokens;
}

#endif
