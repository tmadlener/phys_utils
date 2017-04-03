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
