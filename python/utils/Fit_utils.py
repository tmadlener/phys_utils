from miscHelpers import filterDict, getRapPt
from TGraph_utils import createGraph

def createRapGraph(lambdas, rapBin, lam, ptBinning):
    """
    Create the TGraphAsymmErrors of one lambda parameter in one rapidity bin
    """
    binCenters = [0.5*(ptBinning[i+1] + ptBinning[i]) for i in range(0, len(ptBinning) - 1)]
    binErrors = [0.5*(ptBinning[i+1] - ptBinning[i]) for i in range(0, len(ptBinning) - 1)]

    lamVals = getValuesFromDict(lambdas, rapBin, lam, 0)
    lamErrs = getValuesFromDict(lambdas, rapBin, lam, 1)

    return createGraph(binCenters, lamVals, binErrors, binErrors, lamErrs, lamErrs)


def getValuesFromDict(valDict, rapBin, lam, idx):
    """
    TODO: documentation
    GOAL: given a dict of dict (i.e. what's returned by collectLambdas) return a list of double values
    idx - 0 for params, 1 for param errors
    """
    rapDict = filterDict(valDict, "rap" + str(rapBin)) # get only values for the current rap bin
    def getKey(it): # sort dict according to the pt bin
        return getRapPt(it)[1]

    return [rapDict.get(k)[lam][idx] for k in sorted(rapDict, key=getKey)]


def createAndStoreGraphs(lambdas, baseName, ptBinning,
                         parameters=["lth", "lph", "ltp"], f=None):
    """
    Create TGraphAsymmErrors from the passed lambda values
    """
    if f:
        f.cd()

    for rapBin in set([getRapPt(k)[0] for (k, v) in lambdas.iteritems()]):
        rapStr = "rap" + str(rapBin)
        for lam in parameters:
            graph = createRapGraph(lambdas, rapBin, lam, ptBinning)
            graph.SetName("_".join([baseName, lam, rapStr]))
            graph.Write()
