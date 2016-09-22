#ifndef EVENTMIXER_MISCHELPER_H__
#define EVENTMIXER_MISCHELPER_H__

#include <string>
#include <ostream>
#include <sstream>
#include <iomanip>

template<typename T>
inline T* clone(const T& t)
{
  return static_cast<T*>(t.Clone());
}

template<typename T>
inline T* clone(T* t)
{
  return static_cast<T*>(t->Clone());
}

/** very simple class for easy printing of basic time (hours:minutes:seconds). */
class SimpleTime {
public:
  SimpleTime(long int secs = 0) : m_seconds(secs) {;}

  const long int& count() const { return m_seconds; }
  const std::string print() const;
private:
  long int m_seconds{};
};


const std::string SimpleTime::print() const
{
  const long int hours = m_seconds / 3600;
  const long int mins = (m_seconds - hours * 3600) / 60; // get the minutes above full hours
  const long int secs = m_seconds % 60; // no need to subtract here, since 3600 and 60 are both divisable by 60

  std::stringstream time;
  std::string unit = hours ? "h:m:s" : "m:s";
  if (hours) time << hours << ":";
  time << std::setfill('0') << std::setw(2) << mins << ":" << std::setw(2) << std::setfill('0') << secs << " " << unit;

  return time.str();
}

/** operator overload for SimpleTime class for easy printing to streams. */
std::ostream& operator<<(std::ostream& os, const SimpleTime& time)
{
  os << time.print();
  return os;
}

#endif
