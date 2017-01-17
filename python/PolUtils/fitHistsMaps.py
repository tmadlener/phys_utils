#!/usr/bin/env python

import argparse

from utils.recurse import collectHistograms
from utils.miscHelpers import filterDict
from utils.TGraph_utils import createGraph

def getPtRap(fullName):
    """
    Get the numerical values of the pt and rapidity bin
    NOTE: assumes that this follows the convention that rapX_ptY are always at the end of the full name
    """
    strList = fullName.lower().split("_")
    return[int(strList[-1][2:]), int(strList[-2][3:])]


def fitAngularDistribution(h):
    """Fit the angular distribution function to the histogram"""
    W = TF2("Wcosthphi",
            "[0] * ("
            "1.0 + [1]*x[0]*x[0] + [2]*(1.0-x[0]*x[0])*cos(2*x[1]*0.0174532925)"
            "+ [3]*2*x[0]*sqrt(1-x[0]*x[0])*cos(x[1]*0.0174532925)"
            ")",
            -1.0, 1.0, -180.0, 180.0)
    W.SetParameters(1.0, 0.0, 0.0, 0.0)

    h.Fit(W)

    return {"lth": [W.GetParameter(1), W.GetParError(1)],
            "lph": [W.GetParameter(2), W.GetParError(2)],
            "ltp": [W.GetParameter(3), W.GetParError(3)]}


def collectLambdas(hists):
    """
    Fit all hists in the passed dict and collect the lambdas into a dictionary with
    the same keys as the histograms
    """
    lambdas = {}
    for (name, h) in hists.iteritems():
        lambdas[name] = fitAngularDistribution(h)

    return lambdas


def getValuesFromDict(valDict, rapBin, lam, idx):
    """
    TODO: documentation
    GOAL: given a dict of dict (i.e. what's returned by collectLambdas) return a list of double values
    idx - 0 for params, 1 for param errors
    """
    rapDict = filterDict(valDict, "rap" + str(rapBin)) # get only values for the current rap bin
    def getKey(it): # sort dict according to the pt bin
        return getPtRap(it)[0]

    return [rapDict.get(k)[lam][idx] for k in sorted(rapDict, key=getKey)]


def createRapGraph(lambdas, rapBin, lam, ptBinning):
    """
    Create the TGraphAsymmErrors of one lambda parameter in one rapidity bin
    """
    binCenters = [0.5*(ptBinning[i+1] + ptBinning[i]) for i in range(0, len(ptBinning) - 1)]
    binErrors = [0.5*(ptBinning[i+1] - ptBinning[i]) for i in range(0, len(ptBinning) - 1)]

    lamVals = getValuesFromDict(lambdas, rapBin, lam, 0)
    lamErrs = getValuesFromDict(lambdas, rapBin, lam, 1)

    return createGraph(binCenters, lamVals, binErrors, binErrors, lamErrs, lamErrs)


def createAndStoreGraphs(lambdas, baseName):
    """
    Create TGraphAsymmErrors from the passed lambda values
    """
    # TODO: make this more flexible
    ptBinning = [10, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 50, 70]

    for rapBin in set([getPtRap(k)[1] for (k, v) in lambdas.iteritems()]):
        rapStr = "rap" + str(rapBin)
        for lam in ["lth", "lph", "ltp"]:
            graph = createRapGraph(lambdas, rapBin, lam, ptBinning)
            graph.SetName("_".join([baseName, lam, rapStr]))
            graph.Write()



"""
Argparse
"""
parser = argparse.ArgumentParser(description="Fit cosTheta-Phi histograms with angular distribution"
                                 "and store results as TGraphAsymmErrors in the input file")
parser.add_argument("histFile", help="File containing the histograms to be fit")
parser.add_argument("--histrgx", help="Regex the histogram names have to match to be fitted.",
                    dest="histRgx", action="store")
parser.add_argument("--graphbase", help="Base name for the created graphs.",
                    dest="graphBase", action="store")

parser.set_defaults(histRgx="", graphBase="fitted")
args = parser.parse_args()


"""
Script
"""
from ROOT import TFile, TH2D, TGraphAsymmErrors, TF2, gROOT
gROOT.SetBatch()

inputF = TFile.Open(args.histFile, "update")
histsToFit = collectHistograms(inputF, args.histRgx)
lambdas = collectLambdas(histsToFit)

createAndStoreGraphs(lambdas, args.graphBase)

inputF.Close()
