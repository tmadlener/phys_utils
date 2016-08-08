#ifndef TGA_DATA_H__
#define TGA_DATA_H__

#include "Point.h"

#include "TGraphAsymmErrors.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

/**
 * Class that holds all data needed to construct a TGraphAsymmErrors.
 */
class TGAData {
public:
  /** constructor from a TGraphAsymmErrors. Collects all points stored in the passed graph. */
  TGAData(const TGraphAsymmErrors* tga);

  /** default empty constructor. */
  TGAData() = default;

  /** add points of a TGraphAsymmErrors. Returns the number of added points. */
  int addGraph(const TGraphAsymmErrors* tga);

  /**
   * get the TGraphAsymmErrors constructed from all the data that is currently stored in the the instance.
   * NOTE: you take ownership of this one!
   */
  TGraphAsymmErrors* getTGA();

  /**
   * Second implementation of getTGA. More "experimental apporach, but possibly faster. Have to test it."
   */
  TGraphAsymmErrors* getTGAVectors();

protected:
  std::vector<Point> m_points{}; /**< internal storage of graph points. */

  /** get a certain member value from all points stored in the internal storage. */
  std::vector<double> getValues(std::function<double (const Point&)> valFunc) const;
};

TGAData::TGAData(const TGraphAsymmErrors* tga)
{
  addGraph(tga);
}

int TGAData::addGraph(const TGraphAsymmErrors* tga)
{
  for (int i = 0; i < tga->GetN(); ++i) { // TODO: check if indexing starts at zero
    m_points.push_back(Point(tga, i));
  }

  return tga->GetN();
}

TGraphAsymmErrors* TGAData::getTGA()
{
  // first sort the points according to the values of their x-coordinates
  std::sort(m_points.begin(), m_points.end());

  TGraphAsymmErrors* graph = new TGraphAsymmErrors();
  // NOTE: this is a "clean" but probably really slow implementation of doing this (It is possible that this is cleverly handled in TGraphAsymmErrors however)
  // COULDDO: Find out how to bind members to a lambda and construct vector<double> from the points and use the appropriate constructor of TGraphAsymmErrors
  for (const Point& point : m_points) {
    point.addTo(graph);
  }

  return graph;
}

TGraphAsymmErrors* TGAData::getTGAVectors()
{
  std::sort(m_points.begin(), m_points.end());

  // first assemble all the coordinates needed in a vector each
  std::vector<double> xCoords = getValues([] (const Point& p) { return p.x; });
  std::vector<double> yCoords = getValues([] (const Point& p) { return p.y; });
  std::vector<double> errXH = getValues([] (const Point& p) { return p.exh; });
  std::vector<double> errXL = getValues([] (const Point& p) { return p.exl; });
  std::vector<double> errYH = getValues([] (const Point& p) { return p.eyh; });
  std::vector<double> errYL = getValues([] (const Point& p) { return p.eyl; });

  // now construct the TGraphAsymmErrors as a whole from all values
  return new TGraphAsymmErrors(m_points.size(), xCoords.data(), yCoords.data(),
                               errXL.data(), errXH.data(), errYL.data(), errYH.data());
}

std::vector<double> TGAData::getValues(std::function<double (const Point&)> valFunc) const
{
  std::vector<double> values;
  values.reserve(m_points.size());
  for (const Point& point : m_points) {
    values.push_back(valFunc(point));
  }
  return values;
}

#endif
