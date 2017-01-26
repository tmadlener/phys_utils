#!/usr/bin/env python

import argparse

from utils.recurse import collectHistograms, TH2DCollector
from utils.miscHelpers import getRapPt
from utils.Fit_utils import createRapGraph

def fitAngularDistribution(h):
    """Fit the angular distribution function to the histogram"""
    W = TF2("Wcosthphi",
            "[0] * ("
            "1.0 + [1]*x[0]*x[0] + [2]*(1.0-x[0]*x[0])*cos(2*x[1]*0.0174532925)"
            "+ [3]*2*x[0]*sqrt(1-x[0]*x[0])*cos(x[1]*0.0174532925)"
            ")",
            -1.0, 1.0, -180.0, 180.0)
    W.SetParameters(1.0, 0.0, 0.0, 0.0)

    fitRlt = h.Fit(W, "S")
    fitRlt.SetName("_".join([h.GetName(), "Wcosthphi_rlt"]))
    fitRlt.Write()

    return {"lth": [fitRlt.Parameter(1), fitRlt.Error(1)],
            "lph": [fitRlt.Parameter(2), fitRlt.Error(2)],
            "ltp": [fitRlt.Parameter(3), fitRlt.Error(3)]}


def collectLambdas(hists):
    """
    Fit all hists in the passed dict and collect the lambdas into a dictionary with
    the same keys as the histograms
    """
    lambdas = {}
    for (name, h) in hists.iteritems():
        lambdas[name] = fitAngularDistribution(h)

    return lambdas


def createAndStoreGraphs(lambdas, baseName):
    """
    Create TGraphAsymmErrors from the passed lambda values
    """
    # TODO: make this more flexible
    ptBinning = [10, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 50, 70]

    for rapBin in set([getRapPt(k)[0] for (k, v) in lambdas.iteritems()]):
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
from ROOT import TFile, TH2D, TGraphAsymmErrors, TF2, gROOT, TFitResult
gROOT.SetBatch()

inputF = TFile.Open(args.histFile, "update")
histsToFit = collectHistograms(inputF, args.histRgx, TH2DCollector)
lambdas = collectLambdas(histsToFit)

createAndStoreGraphs(lambdas, args.graphBase)

inputF.Close()
