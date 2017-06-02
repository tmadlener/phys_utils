from miscHelpers import filterDict, getBinIdx
from TGraph_utils import createGraph

def createBinGraph(lambdas, fixVarRgx, binVarRgx, lam, binning):
    """
    Create the TGraphAsymmErrors of one lambda parameter for one bin of the fixed variable
    """
    binCenters = [0.5*(binning[i+1] + binning[i]) for i in range(0, len(binning) - 1)]
    binErrors = [0.5*(binning[i+1] - binning[i]) for i in range(0, len(binning) - 1)]

    lamVals = getValuesFromDict(lambdas, fixVarRgx, binVarRgx, lam, 0)
    lamErrs = getValuesFromDict(lambdas, fixVarRgx, binVarRgx, lam, 1)

    return createGraph(binCenters, lamVals, binErrors, binErrors, lamErrs, lamErrs)


def getValuesFromDict(valDict, fixVarRgx, binVarRgx, lam, idx):
    """
    TODO: documentation
    GOAL: given a dict of dict (i.e. what's returned by collectLambdas) return a list of double values
    idx - 0 for params, 1 for param errors
    """
    binValDict = filterDict(valDict, fixVarRgx) # get only the parameters of interest

    def getKey(it): # sort dict according to the pt bin
        return getBinIdx(it, binVarRgx)

    return [binValDict.get(k)[lam][idx] for k in sorted(binValDict, key=getKey)]


def createAndStoreGraphs(lambdas, baseName, binning, binVarRgx, fixVarRgx,
                         parameters=["lth", "lph", "ltp"], f=None):
    """
    Create TGraphAsymmErrors for every bin of the fixVarRgx and the passed lambda values
    """
    if f:
        f.cd()

    if fixVarRgx:
        for fixVarIdx in set([getBinIdx(k, fixVarRgx) for (k, v) in lambdas.iteritems()]):
            fixVarStr = fixVarRgx + str(fixVarIdx)

            for lam in parameters:
                graph = createBinGraph(lambdas, fixVarStr, binVarRgx, lam, binning)
                graph.SetName("_".join([baseName, lam, fixVarStr]))
                graph.Write()
    else:
        for lam in parameters:
            graph = createBinGraph(lambdas, "", binVarRgx, lam, binning)
            graph.SetName("_".join([baseName, lam]))
            graph.Write()


def fitAngularDistribution(h, store=True):
    """
    Fit the angular distribution function to the histogram.
    If store is set to True, the fitresult pointer is stored in the current TFile.
    """
    from ROOT import TF2, TFitResult
    W = TF2("Wcosthphi",
            "[0] * ("
            "1.0 + [1]*x[0]*x[0] + [2]*(1.0-x[0]*x[0])*cos(2*x[1]*0.0174532925)"
            "+ [3]*2*x[0]*sqrt(1-x[0]*x[0])*cos(x[1]*0.0174532925)"
            ")",
            -1.0, 1.0, -180.0, 180.0)
    W.SetParameters(1.0, 0.0, 0.0, 0.0)

    fitRlt = h.Fit(W, "S")
    if int(fitRlt) == 0: # 0 indicates succesful fit
        fitRlt.SetName("_".join([h.GetName(), "Wcosthphi_rlt"]))
        if store:
            fitRlt.Write()

        return {"lth": [fitRlt.Parameter(1), fitRlt.Error(1)],
                "lph": [fitRlt.Parameter(2), fitRlt.Error(2)],
                "ltp": [fitRlt.Parameter(3), fitRlt.Error(3)]}
    else:
        print("Fit returned status {} for histogram {}".format(int(fitRlt), h.GetName()))
        return {"lth": [-999, 999],
                "lph": [-999, 999],
                "ltp": [-999, 999]}
