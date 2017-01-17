#ifndef PHYSUTILS_CREATEHISTFROMFUNC_H__
#define PHYSUTILS_CREATEHISTFROMFUNC_H__

#include "general/integration.h"
#include "general/progress.h"

#include "TH2D.h"

#include <string>

/**
 * create a (bin-integrated) TH2D from the passed function. Function interface has to take two double arguments
 */
template<typename Func2D>
TH2D* createTH2DfromFunc(const int nBinsX, const double xMin, const double xMax,
                         const int nBinsY, const double yMin, const double yMax, const Func2D& func,
                         const size_t intSteps = 100000, const std::string& name = "")
{
  TH2D* h = new TH2D(name.c_str(), "", nBinsX, xMin, xMax, nBinsY, yMin, yMax);

  const int nIntegrations = nBinsX * nBinsY;
  const size_t nStepsX = intSteps / nBinsX;
  const size_t nStepsY = intSteps / nBinsY;

  const double dx = (xMax - xMin) / nBinsX;
  const double dy = (yMax - yMin) / nBinsY;

  auto startTime = ProgressClock::now();
  // bin indices 0 and nBins + 1 of TH1 (and TH2) are under- resp. overflow bin
  for (int i = 1; i < nBinsX + 1; ++i) {
    const double xmin = xMin + (i-1) * dx;
    const double xmax = xMin + i * dx;

    for (int j = 1; j < nBinsY + 1; ++j) {
      const double ymin = yMin + (j-1) * dy;
      const double ymax = yMin + j * dy;
      h->SetBinContent(i, j, calcIntegral2D(func, xmin, xmax, ymin, ymax, nStepsX, nStepsY));
      h->SetBinError(i, j, 1e-9); // Setting error to zero (hopefully avoiding numerical instabilities)
      printProgress((i-1)*nBinsY + j, nIntegrations, startTime, 4);
    }
  }

  return h;
}

#endif
