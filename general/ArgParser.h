#ifndef PHYS_UTILS_GENERAL_ARGPARSER_H__
#define PHYS_UTILS_GENERAL_ARGPARSER_H__

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm> // for_each
#include <ostream>
#include <iostream>
#include <typeinfo>
#include <regex>


/**
 * very simple argparser.
 *
 * Simply stores flags in map together with all arguments that follow until next flag.
 * For retrieval the full flag is needed.
 * NOTE:
 *   - Types only checked at retrieval (i.e. when trying to convert)
 *   - Flags beginning with a number (e.g. -2) are not allowed to be able to parse negative numbers!
 */
class ArgParser {
public:
  ArgParser() = delete;
  /** constructor takes arguments directly. */
  ArgParser(int argc, char* argv[]);

  /** ostream operator for debugging purposes (i.e. does not print nicely!) */
  friend std::ostream& operator<<(std::ostream&, const ArgParser&);

  /** Try to get the value associated with the passed key. If the key is not found the defVal will be returned. */
  template<typename T>
  T getOptionVal(const std::string& key, const T& defVal) const;

  /** Try to get the value associated with the passed key. If the key is not found the program is terminated. */
  template<typename T>
  T getOptionVal(const std::string& key) const;

private:
  /** internal typedef for less typing effort. */
  using ParseMap = std::unordered_multimap<std::string, std::string>;

  /**
   * get the argument from the map and indicate the succes with the .first of the returned pair.
   * Specialized for std::string and std::vector<std::string>, all basic types supported as long as they
   * can be converted by convertArg.
   */
  template<typename T>
  std::pair<bool, T> getArg(const std::string& key) const;

  /**
   * convert the argument into T and indicate succes with the .first of the returned pair.
   * Converts the string argument to the passed type (boolalpha for bools!). Should work for all basic types.
   */
  template<typename T>
  std::pair<bool, T> convertArg(const std::string& arg) const;

  ParseMap m_argumentMap; /**< The internally used map for storing all flag value pairs. */
};

ArgParser::ArgParser(int argc, char* argv[])
{
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  const std::regex flagRgx("-{1,2}[a-zA-z]+[a-zA-z0-9]*");
  std::string key = "";
  for (const auto& arg : args) {
    if (std::regex_match(arg, flagRgx)) {
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
T ArgParser::getOptionVal(const std::string& key) const
{
  auto args = getArg<T>(key);

  if (!args.first) {
    std::cerr << "Could not get argument for key \'" << key << "\'" << std::endl;
    std::cerr << "If no conversion error message is printed the flag has not been found in the input arguments" << std::endl;
    exit(1);
  }

  return args.second;
}


template<typename T>
T ArgParser::getOptionVal(const std::string& key, const T& defVal) const
{
  auto args = getArg<T>(key);

  if(!args.first) {
    // std::cerr << "\'" << key << "\' was not found in the input arguments."
    //           << "Using [ " << defVal << " ] as default value" << std::endl;
    return defVal;
  }

  return args.second;
}


template<typename T>
std::pair<bool, T> ArgParser::getArg(const std::string& key) const
{
  auto keyIt = m_argumentMap.find(key);
  if (keyIt != m_argumentMap.end()) {
    auto convArg = convertArg<T>(keyIt->second);
    if (convArg.first) return convArg;
    else {
      std::cerr << "\'" << key << "\' was found in the input arguments but could not be converted to \'"
                << typeid(T).name() << "\'" << std::endl;
    }
  }
  return {false, T{}};
}

template<>
std::pair<bool, std::string> ArgParser::getArg(const std::string& key) const
{
  auto keyIt = m_argumentMap.find(key);
  if (keyIt != m_argumentMap.end()) {
    return {true, keyIt->second};
  }

  return {false, ""};
}

template<>
std::pair<bool, std::vector<std::string> > ArgParser::getArg(const std::string& key) const
{
  std::vector<std::string> optionVals;
  auto keyItRange = m_argumentMap.equal_range(key);

  for_each(keyItRange.first, keyItRange.second,
           [&optionVals](const ParseMap::value_type& x) { optionVals.push_back(x.second); });

  return {!optionVals.empty(), optionVals}; // simply set the boolean to false if the vector is empty
}


template<typename T>
std::pair<bool, T> ArgParser::convertArg(const std::string& arg) const
{
  std::stringstream valstr(arg);
  T val;
  if (valstr >> std::boolalpha >> val) {
    return {true, val};
  }

  else return {false, T{}};
}

#endif
