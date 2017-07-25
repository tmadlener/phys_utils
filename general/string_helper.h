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

/** count the number of occurences of substr in str. */
size_t countStrInStr(const std::string& str, const std::string& substr)
{
  size_t cnt{};
  size_t pos = str.find(substr, 0);
  while(pos != std::string::npos) {
    ++cnt;
    pos = str.find(substr, pos + 1);
  }

  return cnt;
}

/** remove all characters that follow AFTER the LAST passed string */
inline std::string removeAfterLast(const std::string& str, const std::string& last)
{
  const size_t pos = str.find_last_of(last);
  if (pos != std::string::npos) {
    return str.substr(0,pos);
  }

  return str;
}

/**
 * replace all instances of tar in str by rep.
 * Returns the number of replacements that took place if succesful or -2 if the tar is empty or -1 if str is empty.
 */
int replace(std::string& str, const std::string& tar, const std::string& rep)
{
  // handle some trivial "error" cases that make no sense in usage
  if (tar.empty()) return -2;
  if (str.empty()) return -1;

  const size_t tarlen = tar.length();
  const size_t replen = rep.length();
  if (tarlen > str.length()) return 0; // we can't replace something that doesn't even fit

  int repCtr = 0;

  size_t pos = 0;
    while ((pos = str.find(tar,pos)) != std::string::npos) {
    str.replace(pos, replen, rep);
    pos += replen;
    repCtr++;
  }

  return repCtr;
}

#endif
