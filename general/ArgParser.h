#ifndef POLFW_CLARG_PARSING_H__
#define POLFW_CLARG_PARSING_H__

#include "string_stuff.h" // other repo!

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm> // for_each
#include <ostream>
#include <iostream>
#include <typeinfo>


/** very simple argparser. */
class ArgParser {
public:
  ArgParser() = delete;
  /** constructor takes arguments directly. */
  ArgParser(int argc, char* argv[]);

  /** ostream operator for debugging purposes (i.e. does not print nicely!) */
  friend std::ostream& operator<<(std::ostream&, const ArgParser&);

  /**
   * Try to get the value associated with the passed key. If the key is not found the defVal will be returned
   * Specialization for string and vector of string present, otherwise probably only works with basic types.
   */
  template<typename T>
  T getOptionVal(const std::string& key, const T& defVal = T{}) const;

private:
  /** internal typedef for less typing effort. */
  using ParseMap = std::unordered_multimap<std::string, std::string>;

  ParseMap m_argumentMap;
};

ArgParser::ArgParser(int argc, char* argv[])
{
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  std::string key = "";
  for (const auto& arg : args) {
    if (startsWith(arg, "-")) {
      key = arg;
    } else {
      m_argumentMap.insert({key, arg});
    }
  }
}


std::ostream& operator<<(std::ostream& os, const ArgParser& args)
{
  for (const auto& kvPair : args.m_argumentMap) {
    os << kvPair.first << ": " << kvPair.second << std::endl;
  }
  return os;
}


template<typename T>
T ArgParser::getOptionVal(const std::string& key, const T& defVal) const
{
  auto keyIt = m_argumentMap.find(key);
  if (keyIt != m_argumentMap.end()) {
    std::stringstream valstr(keyIt->second);
    T val;
    if (valstr >> std::boolalpha >> val) {
      return val;
    }
    std::cerr << "\'" << key << "\' was found in the input arguments but could not be converted to \' "
              << typeid(val).name() << "\'" << std::endl;
  } else {
    std::cerr << "\'" << key << "\' was not found in the input arguments" << std::endl;
  }

  return defVal;
}

// Specialization for std::string
template<>
std::string ArgParser::getOptionVal(const std::string& key, const std::string& defVal) const
{
  auto keyIt = m_argumentMap.find(key);
  if (keyIt != m_argumentMap.end()) {
    return keyIt->second;
  }

  std::cerr << "\'" << key << "\' was not found in the input arguments" << std::endl;
  return defVal;
}


// Specialization for std::vector<std::string>
template<>
std::vector<std::string> ArgParser::getOptionVal(const std::string& key, const std::vector<std::string>& defVal) const
{
  std::vector<std::string> optionVals;
  auto keyItRange = m_argumentMap.equal_range(key);

  for_each(keyItRange.first, keyItRange.second,
           [&optionVals](const ParseMap::value_type& x) { optionVals.push_back(x.second); });

  if (optionVals.empty()) {
    std::cerr << "\'" << key << "\' was not found in the input arguments" << std::endl;
    return defVal;
  }
  return optionVals;
}

#endif
