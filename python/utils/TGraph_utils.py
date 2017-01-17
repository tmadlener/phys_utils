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
