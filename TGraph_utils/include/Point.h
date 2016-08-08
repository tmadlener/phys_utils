#ifndef POINT_H__ // probably not the best include guard but should do its thing
#define POINT_H__

#include "TGraphAsymmErrors.h"

#include <iostream>

/**
 * class that holds all the data needed to define one point in a TGraphAsymmErrors.
 * constructible from a TGraphAsymmErrors (plus index)
 */
struct Point {
  /** constructor from TGraphAsymmErrors needs the index of the point of the TGraphAsymmErrors to use. */
  Point(const TGraphAsymmErrors* tga, int index);

  /**
   * add this point to the passed TGraphAsymmErrors.
   * NOTE: This merely appends the point after the last point in the TGraphAsymmErrors!
   */
  void addTo(TGraphAsymmErrors* tga) const;

  /**
   * Define a comparison operator for easy use with std::sort.
   * Since ordering by x-coordinate is the default use case, this is what is done.
   */
  bool operator<(const Point& other) const { return x < other.x; }

  double x{}; /**< x coordinate.*/
  double y{}; /**< y coordinate. */
  double exh{}; /**< error high of x coordinate. */
  double exl{}; /**< error low of x coordinate. */
  double eyh{}; /**< error high of y coordinate. */
  double eyl{}; /**< error low of y coordinate. */
};

Point::Point(const TGraphAsymmErrors* tga, int index)
{
  const int nPoints = tga->GetN();
  if (nPoints > index) {
    if (tga->GetPoint(index, x, y) != index) { // test this here and only fill the errors, when the point can be retrieved
      std::cerr << "Error while getting point " << index << " from TGraphAsymmErrors \'" << tga->GetName() << "\'" << std::endl;
    } else {
      exh = tga->GetErrorXhigh(index);
      exl = tga->GetErrorXlow(index);
      eyh = tga->GetErrorYhigh(index);
      eyl = tga->GetErrorYlow(index);
    }
  } else {
    std::cerr << "TGraphAsymmErrors \'" << tga->GetName() << "\' only has " << nPoints << ". Cannot use index = " << index << std::endl;
  }
}

void Point::addTo(TGraphAsymmErrors* tga) const
{
  const int nPoints = tga->GetN(); // number of points in the TGraphAsymmErrors prior to the call to addTo
  tga->Set(nPoints + 1); // add a new point at the end
  tga->SetPoint(nPoints, x, y); // indexing still starts at zero
  tga->SetPointEXhigh(nPoints, exh);
  tga->SetPointEXlow(nPoints, exl);
  tga->SetPointEYhigh(nPoints, eyh);
  tga->SetPointEYlow(nPoints, eyl);
}

#endif
