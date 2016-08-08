#include "TGAData.h"

#include "TGraphAsymmErrors.h"

#include <vector>

/** merge all passed TGraphAsymmErrors into one (that is newly allocated and you have to take ownership of). */
TGraphAsymmErrors* mergeGraphs(std::vector<TGraphAsymmErrors*>& graphs)
{
  TGAData graphData;
  for (const auto* graph : graphs) graphData.addGraph(graph);
  return graphData.getTGA();
}
