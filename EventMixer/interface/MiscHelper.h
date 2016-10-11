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

#endif
