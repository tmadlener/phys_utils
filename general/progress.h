#ifndef PHYSUTILS_PROGRESS_H__
#define PHYSUTILS_PROGRESS_H__

#include <chrono>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <iostream>

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

/**
 * Print the progress of a process assuming that there are N total steps and i is the current step.
 * Also prints the elapsed time assuming that the processing startet at the startTime and an estimate of the remaining time.
 *
 * The nPrints variable controls how many printouts there will be and the os can be used to redirect the printout to any ostream.
 */
template<typename ClockType = std::chrono::high_resolution_clock>
void printProgress(const size_t& i, const size_t& N, const typename ClockType::time_point startTime, const size_t& nPrints = 100,
                   std::ostream& os = std::cout)
{
  if (i % (N / nPrints)) return; // only print if desired

  using namespace std::chrono; // avoid typing effort
  const SimpleTime elapsedTime{duration_cast<seconds>(ClockType::now() - startTime).count()};

  // calculate the approximate remaining time
  const double compRatio = static_cast<double>(i) / N;
  const SimpleTime remainTime{static_cast<unsigned>(elapsedTime.count() / compRatio) - elapsedTime.count()};

  os << "Processed " << i << " / " << N << " (" << std::setw(3) << compRatio * 100 << " %)."
     << " Elapsed time: " << elapsedTime << ". Approx. " << remainTime << " remaining." << std::endl;
}

#endif
