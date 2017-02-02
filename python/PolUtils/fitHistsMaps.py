#!/usr/bin/env python

import argparse

from utils.recurse import collectHistograms, TH2DCollector
from utils.miscHelpers import getRapPt
from utils.Fit_utils import createAndStoreGraphs

def fitAngularDistribution(h):
    """Fit the angular distribution function to the histogram"""
    W = TF2("Wcosthphi",
            "[0] * ("
            "1.0 + [1]*x[0]*x[0] + [2]*(1.0-x[0]*x[0])*cos(2*x[1]*0.0174532925)"
            "+ [3]*2*x[0]*sqrt(1-x[0]*x[0])*cos(x[1]*0.0174532925)"
            ")",
            -1.0, 1.0, -180.0, 180.0)
            # -0.8, 0.8, -180.0, 180.0) # TESTING
    W.SetParameters(1.0, 0.0, 0.0, 0.0)

    fitRlt = h.Fit(W, "S")
    # fitRlt = h.Fit(W, "SR") # TESTING
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
parser.add_argument("--ptBinning", help="pt binning that should be used",
                    dest="ptBinning", nargs="+", required=True)

parser.set_defaults(histRgx="", graphBase="fitted")
args = parser.parse_args()

ptBinning = [float(v) for v in args.ptBinning]

"""
Script
"""
from ROOT import TFile, TH2D, TGraphAsymmErrors, TF2, gROOT, TFitResult
gROOT.SetBatch()

inputF = TFile.Open(args.histFile, "update")
histsToFit = collectHistograms(inputF, args.histRgx, TH2DCollector)
lambdas = collectLambdas(histsToFit)

createAndStoreGraphs(lambdas, args.graphBase, ptBinning, ["lth", "lph", "ltp"], inputF)

inputF.Close()
