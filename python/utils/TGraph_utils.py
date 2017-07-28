def createGraph(x, y, exh, exl, eyh, eyl):
    """
    Create a TGraphAsymmErrors from the passed (lists) of values
    """
    from ROOT import TGraphAsymmErrors
    nPoints = len(x) # simply assume that all lists have the same length
    graph = TGraphAsymmErrors(nPoints)
    for i in range(0, nPoints):
        # "blinding" of signal region
        # if x[i] > 5.2 and x[i] < 5.4: continue
        graph.SetPoint(i, x[i], y[i])
        graph.SetPointError(i, exl[i], exh[i], eyl[i], eyh[i])

    return graph


def createGraphSym(x, y, ex, ey):
    """
    Create a TGraphAsymmErrors from the passed (lists) of values.
    This is a simple convenience wrapper around createGraph.
    """
    return createGraph(x, y, ex, ex, ey, ey)


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
    if not graph:
        return [0, 0, 0]

    from ROOT import Double
    x = Double(0)
    y = Double(0)
    graph.GetPoint(i, x, y)
    exl = graph.GetErrorXlow(i)
    exh = graph.GetErrorXhigh(i)

    return [x, exl, exh]


def getYInfo(graph, i):
    """Get the x information from point i in the passed graph."""
    if not graph:
        return [0, 0, 0]

    from ROOT import Double
    x = Double(0)
    y = Double(0)
    graph.GetPoint(i, x, y)
    eyl = graph.GetErrorYlow(i)
    eyh = graph.GetErrorYhigh(i)

    return [y, eyl, eyh]


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


class TGAPoint(object):
    """
    Class for one point in a TGraph(Asymm)Errors
    """

    def __init__(self, graph = None, i = None):
        """
        Create point and get all values from the graph (using point at index)
        """
        [x, exl, exh] = getXInfo(graph, i)
        [y, eyl, eyh] = getYInfo(graph, i)

        self.x = x
        self.exl = exl
        self.exh = exh
        self.y = y
        self.eyl = eyl
        self.eyh = eyh

    def set(self, x, y, exl, exh, eyl, eyh):
        """Set the values"""
        self.x = x
        self.y = y
        self.exl = exl
        self.exh = exh
        self.eyl = eyl
        self.eyh = eyh


def dividePoint(n, d):
    """
    divide point n by d (and do proper error propagation)
    """
    from math import sqrt
    ratioPoint = TGAPoint()

    if d.y == 0:
        if n.y == 0:
            print("Point ratio set to zero to avoid division by 0. "
                  "Numerator was {}".format(n.y))
        return ratioPoint

    ratio = n.y / d.y
    relErrNHigh = (n.eyh / n.y)**2
    relErrNLow = (n.eyl / n.y)**2
    relErrDHigh = (d.eyh / d.y)**2
    relErrDLow = (d.eyl / d.y)**2

    relErrH = relErrNHigh + relErrDHigh
    relErrL = relErrNLow + relErrDLow

    ratioPoint.set(n.x, ratio, n.exl, n.exh, sqrt(relErrL) * ratio, sqrt(relErrH) * ratio)

    return ratioPoint


def collectPoints(graph):
    """
    Collect all the points from a graph
    """
    points = []
    for i in range(0, graph.GetN() + 1):
        points.append(TGAPoint(graph, i))

    return points



def createGraphFromPoints(points):
    """
    create a new TGraph from the passed points
    """
    def collectVals(points, f):
        a = []
        for p in points: a.append(f(p))
        return a

    x = collectVals(points, lambda p: p.x)
    y = collectVals(points, lambda p: p.y)
    exh = collectVals(points, lambda p: p.exh)
    exl = collectVals(points, lambda p: p.exl)
    eyh = collectVals(points, lambda p: p.eyh)
    eyl = collectVals(points, lambda p: p.eyl)

    return createGraph(x, y, exh, exl, eyh, eyl)


def divide(h, g, name = ""):
    """
    Divide TGraph h by TGraph g, returns a new graph
    NOTE: currently does no sort of "bin-matching" or anything of that kind
    """
    from ROOT import TGraphAsymmErrors

    n = h.Clone()
    d = g.Clone()

    nPoints = collectPoints(n)
    dPoints = collectPoints(d)

    if len(nPoints) != len(dPoints):
        print("Cannot divide Graphs with unequal number of points: "
              "{} vs. {}".format(len(nPoints), len(dPoints)))
        return TGraphAsymmErrors()

    nPoints.sort(key=lambda p: p.x)
    dPoints.sort(key=lambda p: p.x)

    ratioPoints = []
    for i in range(0, len(nPoints)):
        ratioPoints.append(dividePoint(nPoints[i], dPoints[i]))

    graph = createGraphFromPoints(ratioPoints)
    if name:
        graph.SetName(name)

    return graph
