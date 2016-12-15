#ifndef PHYSUTILS_INTEGRATION_H__
#define PHYSUTILS_INTEGRATION_H__

#include <vector>
#include <algorithm>
#include <iostream>

/**
 * Integrate function f from min to max via the trapezoidal rule using nSteps steps.
 *
 * returns:
 * \f[
 * I = \frac{h}{2} (f(x_{\text{min}}) + f(x_{\text{max}})) +
 *     h \cdot \sum_{i=1}^{N-1} f(x_i)
 * \f]
 * where \$ x_i = x_{\text{min}} + i * h \$ and \$ h = (x_{\text{max}} - x_{\text{min}}) / N\$.
 */
template<typename F>
double calcIntegral(const F& f, const double min, const double max, const size_t nSteps = 100000)
{
  std::vector<double> funcVals;
  funcVals.reserve(nSteps);

  funcVals.push_back(f(min) * 0.5);
  const double step = (max - min) / nSteps;
  for (size_t i = 1; i < nSteps - 1; ++i) {
    double x = min + i * step;
    funcVals.push_back(f(x));
  }
  funcVals.push_back(f(max) * 0.5);

  return std::accumulate(funcVals.begin(), funcVals.end(), 0.0) * step;
}

/**
 * Integrate function f in the two dimensional rectangle spanned by (xmin, ymin), (xmax, ymax) via the
 * trapezoidal rule using nStepsX in x-direction and nStepsY in y-direction.
 *
 * Implemented as seen here: http://mathfaculty.fullerton.edu/mathews/n2003/SimpsonsRule2DMod.html (14.12.16)
 */
template<typename F>
double calcIntegral2D(const F& f, const double xmin, const double xmax, const double ymin, const double ymax,
                      const size_t nStepsX = 10000, const size_t nStepsY = 10000)
{
  const double h = (xmax - xmin) / nStepsX;
  const double k = (ymax - ymin) / nStepsY;

  // not using vector and std::accumulate here since that can lead to serious memory consumption with
  // large numbers of steps
  double sum = 0;
  sum += 0.25 * (f(xmin, ymin) + f(xmin, ymax) + f(xmax, ymin) + f(xmax, ymax)); // "corner" values

  for (size_t i = 1; i < nStepsX - 1; ++i) { // border-values along y
    const double x = xmin + i * h;
    sum += 0.5 * (f(x, ymin) + f(x, ymax));
  }
  for (size_t j = 1; j < nStepsY - 1; ++j) { // border-values along x
    const double y = ymin + j * k;
    sum += 0.5 * (f(xmin, y) + f(xmax, y));
  }

  for (size_t i = 1; i < nStepsX - 1; ++i) { // "core" values
    for (size_t j = 1; j < nStepsY - 1; ++j) {
      const double x = xmin + i * h;
      const double y = ymin + j * k;
      sum += f(x,y);
    }
  }

  return sum * h * k;
}

/**
 * Integrate function f in the two dimensional rectangle spanned by (xmim, ymin), (xmax, ymax) via the
 * 2D Simpson rule.
 *
 * Implented as seen here: http://mathfaculty.fullerton.edu/mathews/n2003/SimpsonsRule2DMod.html
 *
 * NOTE: doesn't yield too much more precision compared to the trapezoidal rule for large enough numbers of steps
 * but takes bout 4 times longer.
 */
template<typename F>
double calcIntegral2DSimps(const F& f, const double xmin, const double xmax, const double ymin, const double ymax,
                           const size_t  nStepsX = 10000, const size_t nStepsY = 10000)
{
  if ((nStepsX % 2) || (nStepsY % 2)) {
    std::cerr << "Please provide even numbers of steps in both dimensions!" << std::endl;
    return -1;
  }

  const double h = (xmax - xmin) / (nStepsX * 2);
  const double k = (ymax - ymin) / (nStepsY * 2);

  static constexpr double over9 = 1.0 / 9.0;
  std::cout << "beginning integration" << std::endl;
  double sum = 0;
  sum += over9 * (f(xmin, ymin) + f(xmin, ymax) + f(xmax, ymin) + f(xmax, ymax)); // "corner" values

  // computation here is ordered so that different sums are grouped together by their prefactors
  double subsum = 0;
  for (size_t i = 1; i < nStepsX; ++i) { // border values along y (odd)
    const double x = xmin + (2 * i - 1) * h;
    subsum += f(x, ymin) + f(x, ymax);
  }
  for (size_t j = 1; j < nStepsY; ++j) { // border values along x (odd)
    const double y = ymin + (2 * j - 1) * k;
    subsum += f(xmin, y) + f(xmax, y);
  }
  for (size_t i = 1; i < nStepsX - 1; ++i) { // central values (even, even)
    for (size_t j = 1; j < nStepsY - 1; ++ j){
      subsum += f(xmin + 2*i*h, ymin + 2*j*k);
    }
  }
  sum += 4*over9 * subsum;
  // std::cout << "prefactor 4 done" << std::endl;
  subsum = 0;
  for (size_t i = 1; i < nStepsX - 1; ++i) { // border values along y (even)
    const double x = xmin + 2 * i * h;
    subsum += f(x, ymin) + f(x, ymax);
  }
  for (size_t j = 1; j < nStepsY - 1; ++j) { // border values along x (even)
    const double y = ymin + 2 * j * k;
    subsum += f(xmin, y) + f(xmax, y);
  }
  sum += 2*over9 * subsum;
  // std::cout << "prefactor 2 done" << std::endl;
  subsum = 0;
  for (size_t i = 1; i < nStepsX; ++i) { // central values (odd, odd)
    for (size_t j = 1; j < nStepsY; ++j) {
      subsum += f(xmin + (2*i - 1) * h, ymin + (2*j - 1) * k);
    }
  }
  sum += 16*over9 * subsum;
  // std::cout << "prefactor 16 done" << std::endl;
  subsum = 0;
  for (size_t i = 1; i < nStepsX - 1; ++i) { // central values (even, odd)
    for (size_t j = 1; j < nStepsY; ++j) {
      subsum += f(xmin + 2*i*h, ymin + (2*j - 1)*k);
    }
  }
  for (size_t i = 1; i < nStepsX; ++i) { // central values (odd, even)
    for (size_t j = 1; j < nStepsY - 1; ++j) {
      subsum += f(xmin + 2*i*h, ymin + (2*j - 1)*k);
    }
  }
  sum += 8 * over9 * subsum;

  return sum * h * k;
}

#endif
