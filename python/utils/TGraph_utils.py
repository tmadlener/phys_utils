def createGraph(x, y, exh, exl, eyh, eyl):
    """
    Create a TGraphAsymmErrors from the passed (lists) of values
    """
    from ROOT import TGraphAsymmErrors
    nPoints = len(x) # simply assume that all lists have the same length
    graph = TGraphAsymmErrors(nPoints)
    for i in range(0, nPoints):
        graph.SetPoint(i, x[i], y[i])
        graph.SetPointError(i, exl[i], exh[i], eyl[i], eyh[i])

    return graph


def getAvgVal(graphs, i):
    """Get the average value of all y-values at index i in all the passed graphs."""
    from numpy import mean, std
    from ROOT import Double
    from math import sqrt
    x = Double(0)
    y = Double(0)
    vals = []
    for g in graphs:
        g.GetPoint(i, x, y)
        # convert the ROOT.Double to a float to get the value into the list not just a
        # reference (which would result in a list of equal values when evaluated later)
        vals.append(float(y))

    # numpy.std returns 1/N * sum(squared diff to mean) but sample std deviation is defined
    # as 1/(N-1) * sum(squared diff to mean) (reps. the square root of it)
    corrFactor = sqrt(float(len(vals))/(len(vals) - 1.0))
    return [mean(vals), std(vals)*corrFactor]


def getAvgErr(graphs, i):
    """Get the average error of all the y-values at index i in all the passed graphs."""
    from numpy import mean
    return mean([g.GetErrorY(i) for g in graphs])


def getXInfo(graph, i):
    """Get the x information from point i in the passed graph."""
    from ROOT import Double
    x = Double(0)
    y = Double(0)
    graph.GetPoint(i, x, y)
    exl = graph.GetErrorXlow(i)
    exh = graph.GetErrorXhigh(i)

    return [x, exl, exh]


def averageGraphs(graphs):
    """
    Create a new TGraphAsymmErrors from the passed list of TGraphAsymmErrors
    where the central values are the average values of all graphs.
    The uncertainties are computed from adding in quadrature the average of
    the uncertainties and the std-deviation of the central values
    """
    from ROOT import TGraphAsymmErrors
    from math import sqrt
    # simply assume that all graphs have the same number of points
    nPoints = graphs[0].GetN()
    graph = TGraphAsymmErrors(nPoints)
    for i in range(0, nPoints):
        [centralVal, cValSig] = getAvgVal(graphs, i)
        meanErr = getAvgErr(graphs, i)
        uncer = sqrt(cValSig**2 + meanErr**2)

        [x, exl, exh] = getXInfo(graphs[0], i) # simply assume it is the same for all
        graph.SetPoint(i, x, centralVal)
        graph.SetPointError(i, exl, exh, uncer, uncer)

    return graph
