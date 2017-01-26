#!/usr/bin/env python

import argparse

from utils.recurse import collectHistograms, TH1DCollector
from utils.miscHelpers import filterDict, getRapPt
from utils.Fit_utils import createRapGraph

def fitWcosTheta(h):
    """Fit the phi integrated angular distribution function to the histogram"""
    W = TF1("Wcosth",
            "[0] * (1 + [1]*x[0]*x[0])", -1.0, 1.0)
    W.SetParameters(1.0, 0.0)

    fitRlt = h.Fit(W, "S")
    fitRlt.SetName("_".join([h.GetName(), "Wcosth_rlt"]))
    fitRlt.Write()

    return{"N":   [fitRlt.Parameter(0), fitRlt.Error(0)],
           "lth": [fitRlt.Parameter(1), fitRlt.Error(1)]}


def fitWPhi(h):
    """Fit the phi integrated angular distribution function to the histogram"""
    W = TF1("Wphi",
            "[0]/(3+[1]) * (3+[1] + 2*[2]*cos(2*x[0]*0.0174532925))",
            -180.0, 180.0)
    W.SetParameters(1.0, 0.0, 0.0)

    fitRlt = h.Fit(W, "S")
    fitRlt.SetName("_".join([h.GetName(), "Wphi"]))
    fitRlt.Write()

    return {"N":   [fitRlt.Parameter(0), fitRlt.Error(0)],
            "lth": [fitRlt.Parameter(1), fitRlt.Error(1)],
            "lph": [fitRlt.Parameter(2), fitRlt.Error(2)]}

def collectLambdas(hists):
    """
    Fit all the projections ending with '_costh' and '_phi' with the appropriate
    angular distributed and collect the labmdas into a dictionary with the same keys
    as the histograms
    """
    lambdas = {}
    for (name, h) in filterDict(hists, "_costh(_|$)").iteritems():
        lambdas[name] = fitWcosTheta(h)
    for (name, h) in filterDict(hists, "_phi(_|$)").iteritems():
        lambdas[name] = fitWPhi(h)

    return lambdas


def createAndStoreGraphs(lambdas, baseName):
    """
    Create TGraphAsymmErrors from the passed lambda values
    """
    ptBinning = [10, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 50, 70]

    # filter the dictionary first for "_phi" resp. "_costh"
    phiLambdas = filterDict(lambdas, "_phi(_|$)")
    costhLambdas = filterDict(lambdas, "_costh(_|$)")

    for rapBin in set([getRapPt(k)[0] for (k,v) in phiLambdas.iteritems()]):
        rapStr = "rap" + str(rapBin)
        graph = createRapGraph(phiLambdas, rapBin, "lph", ptBinning)
        graph.SetName("_".join([baseName, "lph", rapStr]))
        graph.Write()

    for rapBin in set([getRapPt(k)[0] for (k,v) in costhLambdas.iteritems()]):
        rapStr = "rap" + str(rapBin)
        graph = createRapGraph(costhLambdas, rapBin, "lth", ptBinning)
        graph.SetName("_".join([baseName, "lth", rapStr]))
        graph.Write()


"""
Argparse
"""
parser = argparse.ArgumentParser(description='Fit 1D Projections')
parser.add_argument('histFile', help="File containing the histograms to be fit")
parser.add_argument('-hr', '--histrgx', help="Regex the histograms have to match to be fitted",
                    dest="histRgx", action="store", default="")
parser.add_argument("--graphbase", "-g", help="Base name for the created graphs.",
                    dest="graphBase", action="store", default="proj_fit")

args = parser.parse_args()

"""
Script
"""
from ROOT import TFile, TH1D, TGraphAsymmErrors, TF1, gROOT
gROOT.SetBatch()

inputF = TFile.Open(args.histFile, "update")
projections = collectHistograms(inputF, args.histRgx, TH1DCollector)

createAndStoreGraphs(collectLambdas(projections), args.graphBase)

inputF.Close()
